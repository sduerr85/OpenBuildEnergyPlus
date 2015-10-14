%module energyplus


%include "std_string.i"
	
%{
	void EnergyPlusPgm( std::string const & filepath = std::string() );
%}

void EnergyPlusPgm( std::string const & filepath = std::string() );
