#ifndef ObjexxFCL_Optional_hh_INCLUDED
#define ObjexxFCL_Optional_hh_INCLUDED

// Optional Argument Wrapper
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

// ObjexxFCL Headers
#include <ObjexxFCL/Optional.fwd.hh>
#include <ObjexxFCL/Omit.hh>

// C++ Headers
#include <cassert>
#include <type_traits>

namespace ObjexxFCL {

// Optional Argument Wrapper
template< typename T, typename Enable >
class Optional
{

private: // Friend

	template< typename, typename > friend class Optional;

public: // Types

	typedef  T  Value;

public: // Creation

	// Default Constructor
	inline
	Optional() :
		ptr_( nullptr ),
		own_( false )
	{}

	// Copy Constructor
	inline
	Optional( Optional const & o ) :
		ptr_( o.own_ ? new Value( o() ) : o.ptr_ ),
		own_( o.own_ )
	{}

	// Optional Constructor Template
	template< typename U >
	inline
	Optional( Optional< U, Enable > const & o, typename std::enable_if< std::is_const< T >::value && std::is_same< U, typename std::remove_const< T >::type >::value >::type * = 0 ) :
		ptr_( o.own_ ? new Value( o() ) : o.ptr_ ),
		own_( o.own_ )
	{}

	// Value Constructor
	inline
	Optional( Value const & val ) :
		ptr_( const_cast< Value * >( &val ) ),
		own_( false )
	{}

	// Value Constructor Template
	template< typename U >
	inline
	Optional( U const & val ) :
		ptr_( new Value( val ) ), // Requires Value( U ) constructor
		own_( true )
	{}

	// rvalue Constructor
	inline
	Optional( Value && val ) :
		ptr_( new Value( val ) ), // Requires Value copy constructor
		own_( true )
	{}

	// Omit Constructor
	inline
	Optional( Omit ) :
		ptr_( nullptr ),
		own_( false )
	{}

	// Destructor
	inline
	~Optional()
	{
		if ( own_ ) delete ptr_;
	}

public: // Assignment

	// Copy Assignment
	inline
	Optional &
	operator =( Optional const & o )
	{
		if ( own_ ) delete ptr_;
		ptr_ = o.own_ ? new Value( o() ) : o.ptr_;
		own_ = o.own_;
		return *this;
	}

	// Value Assignment
	inline
	Optional &
	operator =( Value const & val )
	{
		assert( ptr_ );
		*ptr_ = val;
		return *this;
	}

	// Value Assignment Template
	template< typename U >
	inline
	Optional &
	operator =( U const & val )
	{
		assert( ptr_ );
		*ptr_ = val;
		return *this;
	}

	// rvalue Assignment
	inline
	Optional &
	operator =( Value && val )
	{
		assert( ptr_ );
		*ptr_ = val;
		return *this;
	}

	// Omit Assignment
	inline
	Optional &
	operator =( Omit )
	{
		if ( own_ ) delete ptr_;
		ptr_ = nullptr;
		own_ = false;
		return *this;
	}

public: // Conversion

	// Value Conversion
	inline
	operator Value const &() const
	{
		assert( ptr_ );
		return *ptr_;
	}

	// Value Conversion
	inline
	operator Value &()
	{
		assert( ptr_ );
		return *ptr_;
	}

public: // Operators

	// Value
	inline
	Value const &
	operator ()() const
	{
		assert( ptr_ );
		return *ptr_;
	}

	// Value
	inline
	Value &
	operator ()()
	{
		assert( ptr_ );
		return *ptr_;
	}

public: // Properties

	// Present?
	inline
	bool
	present() const
	{
		return ptr_;
	}

	// Own?
	inline
	bool
	own() const
	{
		return own_;
	}

public: // Modifiers

	// Clear
	inline
	void
	clear()
	{
		if ( own_ ) delete ptr_;
		ptr_ = nullptr;
		own_ = false;
	}

public: // Comparison

	// Optional == Optional
	inline
	friend
	bool
	operator ==( Optional const & a, Optional const & b )
	{
		return ( a.ptr_ && b.ptr_ && ( *a.ptr_ == *b.ptr_ ) ) || ( ( ! a.ptr_ ) && ( ! b.ptr_ ) );
	}

	// Optional != Optional
	inline
	friend
	bool
	operator !=( Optional const & a, Optional const & b )
	{
		return !( a == b );
	}

	// Optional == Value
	inline
	friend
	bool
	operator ==( Optional const & a, Value const & b )
	{
		return ( ( a.ptr_ ) && ( *a.ptr_ == b ) );
	}

	// Optional != Value
	inline
	friend
	bool
	operator !=( Optional const & a, Value const & b )
	{
		return !( a == b );
	}

	// Value == Optional
	inline
	friend
	bool
	operator ==( Value const & a, Optional const & b )
	{
		return ( ( b.ptr_ ) && ( a == *b.ptr_ ) );
	}

	// Value != Optional
	inline
	friend
	bool
	operator !=( Value const & a, Optional const & b )
	{
		return !( a == b );
	}

private: // Data

	Value * ptr_; // Pointer to object
	bool own_; // Own the object?

}; // Optional

// Optional Argument Wrapper: Abstract Type Specialization
template< typename T >
class Optional< T, typename std::enable_if< std::is_abstract< T >::value >::type >
{

private: // Friend

	template< typename, typename > friend class Optional;

public: // Types

	typedef  T  Value;
	typedef  typename std::enable_if< std::is_abstract< T >::value >::type  EnableType;

public: // Creation

	// Default Constructor
	inline
	Optional() :
		ptr_( nullptr )
	{}

	// Copy Constructor
	inline
	Optional( Optional const & o ) :
		ptr_( o.ptr_ )
	{}

	// Optional Constructor Template
	template< typename U >
	inline
	Optional( Optional< U, EnableType > const & o, typename std::enable_if< std::is_const< T >::value && std::is_same< U, typename std::remove_const< T >::type >::value >::type * = 0 ) :
		ptr_( o.ptr_ )
	{}

	// Value Constructor
	inline
	Optional( Value const & val ) :
		ptr_( const_cast< Value * >( &val ) )
	{}

	// Omit Constructor
	inline
	Optional( Omit ) :
		ptr_( nullptr )
	{}

	// Destructor
	inline
	~Optional()
	{}

public: // Assignment

	// Copy Assignment
	inline
	Optional &
	operator =( Optional const & o )
	{
		ptr_ = o.ptr_;
		return *this;
	}

	// Value Assignment
	inline
	Optional &
	operator =( Value const & val )
	{
		assert( ptr_ );
		*ptr_ = val;
		return *this;
	}

	// Value Assignment Template
	template< typename U >
	inline
	Optional &
	operator =( U const & val )
	{
		assert( ptr_ );
		*ptr_ = val;
		return *this;
	}

	// rvalue Assignment
	inline
	Optional &
	operator =( Value && val )
	{
		assert( ptr_ );
		*ptr_ = val;
		return *this;
	}

	// Omit Assignment
	inline
	Optional &
	operator =( Omit )
	{
		ptr_ = nullptr;
		return *this;
	}

public: // Conversion

	// Value Conversion
	inline
	operator Value const &() const
	{
		assert( ptr_ );
		return *ptr_;
	}

	// Value Conversion
	inline
	operator Value &()
	{
		assert( ptr_ );
		return *ptr_;
	}

public: // Operators

	// Value
	inline
	Value const &
	operator ()() const
	{
		assert( ptr_ );
		return *ptr_;
	}

	// Value
	inline
	Value &
	operator ()()
	{
		assert( ptr_ );
		return *ptr_;
	}

public: // Properties

	// Present?
	inline
	bool
	present() const
	{
		return ptr_;
	}

public: // Modifiers

	// Clear
	inline
	void
	clear()
	{
		ptr_ = nullptr;
	}

public: // Comparison

	// Optional == Optional
	inline
	friend
	bool
	operator ==( Optional const & a, Optional const & b )
	{
		return ( a.ptr_ && b.ptr_ && ( *a.ptr_ == *b.ptr_ ) ) || ( ( ! a.ptr_ ) && ( ! b.ptr_ ) );
	}

	// Optional != Optional
	inline
	friend
	bool
	operator !=( Optional const & a, Optional const & b )
	{
		return !( a == b );
	}

	// Optional == Value
	inline
	friend
	bool
	operator ==( Optional const & a, Value const & b )
	{
		return ( ( a.ptr_ ) && ( *a.ptr_ == b ) );
	}

	// Optional != Value
	inline
	friend
	bool
	operator !=( Optional const & a, Value const & b )
	{
		return !( a == b );
	}

	// Value == Optional
	inline
	friend
	bool
	operator ==( Value const & a, Optional const & b )
	{
		return ( ( b.ptr_ ) && ( a == *b.ptr_ ) );
	}

	// Value != Optional
	inline
	friend
	bool
	operator !=( Value const & a, Optional const & b )
	{
		return !( a == b );
	}

private: // Data

	Value * ptr_; // Pointer

}; // Optional

// Argument Present?
template< typename T >
inline
bool
present( Optional< T > const & o )
{
	return o.present();
}

// Argument Present?
template< typename T >
inline
bool
PRESENT( Optional< T > const & o )
{
	return o.present();
}

} // ObjexxFCL

#endif // ObjexxFCL_Optional_hh_INCLUDED
