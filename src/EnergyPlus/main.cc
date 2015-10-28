#include <EnergyPlusPgm.hh>
#include <CommandLineInterface.hh>
using EnergyPlus::CommandLineInterface::ProcessArgs;

int
main( int argc, const char * argv[] )
{
    printf("+---------------------------------------------------+\n");
    printf("|  Modified version of EnergyPlus for openBuildNet  |\n");
    printf("+---------------------------------------------------+\n\n");
    
	ProcessArgs( argc, argv );
	EnergyPlusPgm();
}
