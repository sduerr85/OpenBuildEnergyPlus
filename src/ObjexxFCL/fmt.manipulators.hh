#ifndef ObjexxFCL_fmt_manipulators_hh_INCLUDED
#define ObjexxFCL_fmt_manipulators_hh_INCLUDED

// Fortran-Compatible Formatted Input/Output Support Manipulators
//
// Project: Objexx Fortran Compatibility Library (ObjexxFCL)
//
// Version: 4.0.0
//
// Language: C++
//
// Copyright (c) 2000-2014 Objexx Engineering, Inc. All Rights Reserved.
// Use of this source code or any derivative of it is restricted by license.
// Licensing is available from Objexx Engineering, Inc.:  http://objexx.com

// C++ Headers
#include <climits>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>

namespace ObjexxFCL {
namespace fmt {

// Signed Discriminator Template
template< bool s >
struct Signed
{

	// Is Value Negative?
	template< typename T >
	inline
	static
	bool
	is_negative( T )
	{
		static_assert( ! std::is_signed< T >::value, "ObjexxFCL::fmt::Signed< false >::is_negative< T > called with signed T" );
		return false;
	}

}; // Signed

// Signed Discriminator Template: True Specialization
template<>
struct Signed< true >
{

	// Is Value Negative?
	template< typename T >
	inline
	static
	bool
	is_negative( T v )
	{
		static_assert( std::is_signed< T >::value, "ObjexxFCL::fmt::Signed< true >::is_negative< T > called with unsigned T" );
		return v < T( 0 );
	}

}; // Signed

// Binary Formatted Output Facet
struct Binary_num_put : std::num_put< char >
{

	// Constructor
	inline
	Binary_num_put()
	{}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, bool v ) const
	{
		return binary_do_put( out, str, fill, static_cast< unsigned int >( v ) );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, long int v ) const
	{
		return binary_do_put( out, str, fill, static_cast< unsigned long int >( v ) ); // Fortran treat's value as unsigned
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, unsigned long int v ) const
	{
		return binary_do_put( out, str, fill, v );
	}

private: // Static Methods

	inline
	static
	int
	geti()
	{
		static int const i( std::ios_base::xalloc() );
		return i;
	}

	template< typename T >
	inline
	static
	iter_type
	binary_do_put( iter_type out, std::ios_base & str, char_type fill, T v )
	{
		bool const negative( Signed< std::is_signed< T >::value >::is_negative( v ) );
		char digits[ CHAR_BIT * sizeof( T ) ];
		int i( 0 );
		while ( v != T( 0 ) ) {
			digits[ i++ ] = ( v % 2 == 0 ? '0' : '1' );
			v /= 2;
		}
		std::ios_base::fmtflags const flags( str.flags() );
		bool const showplus( flags & std::ios::showpos );
		std::streamsize const v_wid( i + ( negative || showplus ? 1ul : 0ul ) );
		std::streamsize const str_wid( str.width() );
		if ( ( str_wid > 0 ) && ( v_wid > str_wid ) ) { // Fortran *-fills when output is too wide
			for ( std::streamsize j = 0, e = str_wid; j < e; ++j ) *out++ = '*';
			str.width( 0 ); // Reset the width
			return out;
		}
		unsigned long int const fill_width( str_wid >= v_wid ? str_wid - v_wid : 0 );
		if ( ( flags & std::ios::right ) || ( ! ( flags & ( std::ios::internal | std::ios::left ) ) ) ) std::fill_n( out, fill_width, fill ); // Front fill
		if ( negative ) {
			*out++ = '-';
		} else if ( showplus ) {
			*out++ = '+';
		}
		if ( flags & std::ios::internal ) std::fill_n( out, fill_width, fill ); // Internal fill
		while ( i > 0 ) *out++ = digits[ --i ];
		if ( flags & std::ios::left ) std::fill_n( out, fill_width, fill ); // Back fill
		str.width( 0 ); // Reset the width
		return out;
	}

}; // Binary_num_put

// Exponent Formatted Output Facet
struct Exponent_num_put : std::num_put< char >
{

	// Constructor
	inline
	explicit
	Exponent_num_put( unsigned int const d, unsigned int const e = 2u, int const k = 0, char const E = 'E' ) :
		d_( d ),
		e_( e ),
		k_( k ),
		E_( E )
	{}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, bool v ) const
	{
		return exponent_do_put( out, str, fill, static_cast< unsigned int >( v ) );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, long int v ) const
	{
		return exponent_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, unsigned long int v ) const
	{
		return exponent_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, float v ) const
	{
		return exponent_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, double v ) const
	{
		return exponent_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, long double v ) const
	{
		return exponent_do_put( out, str, fill, v );
	}

private: // Static Methods

	inline
	static
	int
	geti()
	{
		static int const i( std::ios_base::xalloc() );
		return i;
	}

	template< typename T >
	inline
	iter_type
	exponent_do_put( iter_type out, std::ios_base & str, char_type fill, T v ) const
	{
		bool const negative( Signed< std::is_signed< T >::value >::is_negative( v ) );
		if ( negative ) v = -v;
		int vexp( 0 );
		if ( v != T( 0 ) ) {
			vexp = int( std::floor( std::log10( v ) ) ) + 1 - k_;
			if ( -vexp < 309 ) {
				v *= std::pow( 10, -vexp );
			} else {
				v *= std::pow< long double, long double >( 10, -vexp );
			}
			if ( k_ >= 0 ) {
				std::string const k_str( "1" + std::string( k_, '0' ) + "." );
				std::ostringstream x;
				x << std::fixed << v;
				if ( x.str().substr( 0, k_str.length() ) == k_str ) { // Rounding adjustment
					v /= T( 10.0 );
					vexp += 1;
				}
			} else {
				std::string const k_str( "0." + std::string( 1 - k_, '0' ) );
				std::ostringstream x;
				x << std::fixed << std::setprecision( std::max( int( d_ ), -k_ + 1 ) ) << v;
				if ( x.str().substr( 0, k_str.length() ) == k_str ) { // Rounding adjustment
					v *= T( 10.0 );
					vexp -= 1;
				} else { // Check for too few 0's
					std::string const k_str( "0." + std::string( -k_, '0' ) );
					std::ostringstream x;
					x << std::fixed << std::setprecision( std::max( int( d_ ), -k_ + 1 ) ) << v;
					if ( x.str().substr( 0, k_str.length() ) != k_str ) { // Rounding adjustment
						v /= T( 10.0 );
						vexp += 1;
					}
				}
			}
		}
		std::ostringstream s;
		s << std::fixed << std::setprecision( d_ ) << v << std::internal << std::showpos << std::setfill( '0' );
#ifdef OBJEXXFCL_FMT_STRICT_EXPONENTS
		if ( ( e_ == 2 ) && ( std::abs( vexp ) > 99 ) ) { // Use compact +eee or -eee exponent: Fortran Ew.d and Dw.d behavior
#else
		if ( ( vexp != 0 ) && ( static_cast< unsigned int >( std::log10( std::abs( vexp ) ) ) + 1u > e_ ) ) { // Use compact exponent whenever it helps
#endif
			s << std::setw( e_ + 2 ) << vexp;
		} else { // Use normal exponent with specified width
			s << E_ << std::setw( e_ + 1 ) << vexp;
		}
		std::string v_str( s.str() );
		std::ios_base::fmtflags const flags( str.flags() );
		bool const showplus( flags & std::ios::showpos );
		std::streamsize v_wid( v_str.length() + ( negative || showplus ? 1ul : 0ul ) );
		std::streamsize const str_wid( str.width() );
		if ( ( str_wid > 0 ) && ( v_wid > str_wid ) && ( v_str.substr( 0, 2 ) == "0." ) ) { // Drop leading 0 to narrow output
			v_str.erase( 0, 1 );
			--v_wid;
		}
		if ( ( str_wid > 0 ) && ( v_wid > str_wid ) ) { // Fortran *-fills when output is too wide
			for ( std::streamsize j = 0, e = str_wid; j < e; ++j ) *out++ = '*';
			str.width( 0 ); // Reset the width
			return out;
		}
		unsigned long int const fill_width( str_wid >= v_wid ? str_wid - v_wid : 0 );
		if ( ( flags & std::ios::right ) || ( ! ( flags & ( std::ios::internal | std::ios::left ) ) ) ) std::fill_n( out, fill_width, fill ); // Front fill
		if ( negative ) {
			*out++ = '-';
		} else if ( showplus ) {
			*out++ = '+';
		}
		if ( flags & std::ios::internal ) std::fill_n( out, fill_width, fill ); // Internal fill
		for ( int i = 0, e = v_str.length(); i < e; ++i ) *out++ = v_str[ i ];
		if ( flags & std::ios::left ) std::fill_n( out, fill_width, fill ); // Back fill
		str.width( 0 ); // Reset the width
		return out;
	}

private: // Data

	unsigned int d_; // Digits after decimal point
	unsigned int e_; // Exponent width
	int k_; // Scale factor
	char E_; // Exponent prefix character

}; // Exponent_num_put

// Engineering Formatted Output Facet
struct Engineering_num_put : std::num_put< char >
{

	// Constructor
	inline
	explicit
	Engineering_num_put( unsigned int const d, unsigned int const e = 2u ) :
		d_( d ),
		e_( e )
	{}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, bool v ) const
	{
		return engineering_do_put( out, str, fill, static_cast< unsigned int >( v ) );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, long int v ) const
	{
		return engineering_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, unsigned long int v ) const
	{
		return engineering_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, float v ) const
	{
		return engineering_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, double v ) const
	{
		return engineering_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, long double v ) const
	{
		return engineering_do_put( out, str, fill, v );
	}

private: // Static Methods

	inline
	static
	int
	geti()
	{
		static int const i( std::ios_base::xalloc() );
		return i;
	}

	template< typename T >
	inline
	iter_type
	engineering_do_put( iter_type out, std::ios_base & str, char_type fill, T v ) const
	{
		bool const negative( Signed< std::is_signed< T >::value >::is_negative( v ) );
		if ( negative ) v = -v;
		int vexp( 0 );
		if ( v != T( 0 ) ) {
			int const vl10( std::floor( std::log10( v ) ) );
			vexp = ( vl10 >= 0 ? ( vl10 / 3 ) * 3 : ( ( -vl10 + 3 ) / 3 ) * ( -3 ) );
			v *= std::pow( 10, -vexp );
			std::ostringstream x;
			x << std::fixed << v;
			if ( x.str().substr( 0, 5 ) == std::string( "1000." ) ) { // Rounding adjustment
				v /= T( 1000.0 );
				vexp += 3;
			}
		}
		std::ostringstream s;
		s << std::fixed << std::setprecision( d_ ) << v << std::internal << std::showpos << std::setfill( '0' );
#ifdef OBJEXXFCL_FMT_STRICT_EXPONENTS
		if ( ( e_ == 2 ) && ( std::abs( vexp ) > 99 ) ) { // Use compact +eee or -eee exponent: Fortran Ew.d and Dw.d behavior
#else
		if ( ( vexp != 0 ) && ( static_cast< unsigned int >( std::log10( std::abs( vexp ) ) ) + 1u > e_ ) ) { // Use compact exponent whenever it helps
#endif
			s << std::setw( e_ + 2 ) << vexp;
		} else { // Use normal exponent with specified width
			s << 'E' << std::setw( e_ + 1 ) << vexp;
		}
		std::string const v_str( s.str() );
		std::ios_base::fmtflags const flags( str.flags() );
		bool const showplus( flags & std::ios::showpos );
		std::streamsize const v_wid( v_str.length() + ( negative || showplus ? 1ul : 0ul ) );
		std::streamsize const str_wid( str.width() );
		if ( ( str_wid > 0 ) && ( v_wid > str_wid ) ) { // Fortran *-fills when output is too wide
			for ( std::streamsize j = 0, e = str_wid; j < e; ++j ) *out++ = '*';
			str.width( 0 ); // Reset the width
			return out;
		}
		unsigned long int const fill_width( str_wid >= v_wid ? str_wid - v_wid : 0 );
		if ( ( flags & std::ios::right ) || ( ! ( flags & ( std::ios::internal | std::ios::left ) ) ) ) std::fill_n( out, fill_width, fill ); // Front fill
		if ( negative ) {
			*out++ = '-';
		} else if ( showplus ) {
			*out++ = '+';
		}
		if ( flags & std::ios::internal ) std::fill_n( out, fill_width, fill ); // Internal fill
		for ( int i = 0, e = v_str.length(); i < e; ++i ) *out++ = v_str[ i ];
		if ( flags & std::ios::left ) std::fill_n( out, fill_width, fill ); // Back fill
		str.width( 0 ); // Reset the width
		return out;
	}

private: // Data

	unsigned int d_; // Digits after decimal point
	unsigned int e_; // Exponent width

}; // Engineering_num_put

// Scientific Formatted Output Facet
struct Scientific_num_put : std::num_put< char >
{

	// Constructor
	inline
	explicit
	Scientific_num_put( unsigned int const d, unsigned int const e = 2u ) :
		d_( d ),
		e_( e )
	{}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, bool v ) const
	{
		return scientific_do_put( out, str, fill, static_cast< unsigned int >( v ) );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, long int v ) const
	{
		return scientific_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, unsigned long int v ) const
	{
		return scientific_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, float v ) const
	{
		return scientific_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, double v ) const
	{
		return scientific_do_put( out, str, fill, v );
	}

	inline
	iter_type
	do_put( iter_type out, std::ios_base & str, char_type fill, long double v ) const
	{
		return scientific_do_put( out, str, fill, v );
	}

private: // Static Methods

	inline
	static
	int
	geti()
	{
		static int const i( std::ios_base::xalloc() );
		return i;
	}

	template< typename T >
	inline
	iter_type
	scientific_do_put( iter_type out, std::ios_base & str, char_type fill, T v ) const
	{
		bool const negative( Signed< std::is_signed< T >::value >::is_negative( v ) );
		if ( negative ) v = -v;
		int vexp( 0 );
		if ( v != T( 0 ) ) {
			vexp = std::floor( std::log10( v ) );
			v *= std::pow( 10, -vexp );
			std::ostringstream x;
			x << std::fixed << v;
			if ( x.str().substr( 0, 3 ) == std::string( "10." ) ) { // Rounding adjustment
				v /= T( 10.0 );
				vexp += 1;
			}
		}
		std::ostringstream s;
		s << std::fixed << std::setprecision( d_ ) << v << std::internal << std::showpos << std::setfill( '0' );
#ifdef OBJEXXFCL_FMT_STRICT_EXPONENTS
		if ( ( e_ == 2 ) && ( std::abs( vexp ) > 99 ) ) { // Use compact +eee or -eee exponent: Fortran Ew.d and Dw.d behavior
#else
		if ( ( vexp != 0 ) && ( static_cast< unsigned int >( std::log10( std::abs( vexp ) ) ) + 1u > e_ ) ) { // Use compact exponent whenever it helps
#endif
			s << std::setw( e_ + 2 ) << vexp;
		} else { // Use normal exponent with specified width
			s << 'E' << std::setw( e_ + 1 ) << vexp;
		}
		std::string const v_str( s.str() );
		std::ios_base::fmtflags const flags( str.flags() );
		bool const showplus( flags & std::ios::showpos );
		std::streamsize const v_wid( v_str.length() + ( negative || showplus ? 1ul : 0ul ) );
		std::streamsize const str_wid( str.width() );
		if ( ( str_wid > 0 ) && ( v_wid > str_wid ) ) { // Fortran *-fills when output is too wide
			for ( std::streamsize j = 0, e = str_wid; j < e; ++j ) *out++ = '*';
			str.width( 0 ); // Reset the width
			return out;
		}
		unsigned long int const fill_width( str_wid >= v_wid ? str_wid - v_wid : 0 );
		if ( ( flags & std::ios::right ) || ( ! ( flags & ( std::ios::internal | std::ios::left ) ) ) ) std::fill_n( out, fill_width, fill ); // Front fill
		if ( negative ) {
			*out++ = '-';
		} else if ( showplus ) {
			*out++ = '+';
		}
		if ( flags & std::ios::internal ) std::fill_n( out, fill_width, fill ); // Internal fill
		for ( int i = 0, e = v_str.length(); i < e; ++i ) *out++ = v_str[ i ];
		if ( flags & std::ios::left ) std::fill_n( out, fill_width, fill ); // Back fill
		str.width( 0 ); // Reset the width
		return out;
	}

private: // Data

	unsigned int d_; // Digits after decimal point
	unsigned int e_; // Exponent width

}; // Scientific_num_put

} // fmt
} // ObjexxFCL

#endif // ObjexxFCL_fmt_manipulators_hh_INCLUDED
