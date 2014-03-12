#ifndef WaterToAirHeatPump_hh_INCLUDED
#define WaterToAirHeatPump_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/FArray1D.hh>
#include <ObjexxFCL/FArray1S.hh>
#include <ObjexxFCL/Fstring.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus.hh>
#include <DataGlobals.hh>

namespace EnergyPlus {

namespace WaterToAirHeatPump {

	// Using/Aliasing
	using DataGlobals::MaxNameLength;

	// Data
	//MODULE PARAMETER DEFINITIONS
	extern int const CompressorType_Reciprocating;
	extern int const CompressorType_Rotary;
	extern int const CompressorType_Scroll;

	// DERIVED TYPE DEFINITIONS

	// Output Variables Type definition

	//MODULE VARIABLE DECLARATIONS:
	extern int NumWatertoAirHPs; // The Number of Water to Air Heat Pumps found in the Input
	extern FArray1D_bool CheckEquipName;

	extern int RefrigIndex; // Refrigerant index
	extern int WaterIndex; // Water index
	extern bool GetCoilsInputFlag; // Flag set to make sure you get input once
	// Subroutine Specifications for the Module
	// Driver/Manager Routines

	// Get Input routines for module

	// Initialization routines for module

	// Computational routines

	// Update routine to check convergence and update nodes

	// Utility routines

	// Types

	struct WatertoAirHPEquipConditions
	{
		// Members
		Fstring Name; // Name of the Water to Air Heat pump
		Fstring WatertoAirHPType; // Type of WatertoAirHP ie. Heating or Cooling
		int WAHPPlantTypeOfNum; // type of component in plant
		Fstring Refrigerant; // Refrigerant name
		bool SimFlag;
		Real64 InletAirMassFlowRate; // Inlet Air Mass Flow through the Water to Air Heat Pump being Simulated [kg/s]
		Real64 OutletAirMassFlowRate; // Outlet Air Mass Flow through the Water to Air Heat Pump being Simulated [kg/s]
		Real64 InletAirDBTemp; // Inlet Air Dry Bulb Temperature [C]
		Real64 InletAirHumRat; // Inlet Air Humidity Ratio [kg/kg]
		Real64 OutletAirDBTemp; // Outlet Air Dry Bulb Temperature [C]
		Real64 OutletAirHumRat; // Outlet Air Humidity Ratio [kg/kg]
		Real64 InletAirEnthalpy; // Inlet Air Enthalpy [J/kg]
		Real64 OutletAirEnthalpy; // Outlet Air Enthalpy [J/kg]
		Real64 InletWaterTemp; // Inlet Water Temperature [C]
		Real64 OutletWaterTemp; // Outlet Water Temperature [C]
		Real64 InletWaterMassFlowRate; // Inlet Water Mass Flow Rate [kg/s]
		Real64 OutletWaterMassFlowRate; // Outlet Water Mass Flow Rate [kg/s]
		Real64 DesignWaterMassFlowRate; // Design Water Mass Flow Rate [kg/s]
		Real64 DesignWaterVolFlowRate; // Design Water Volumetric Flow Rate [m3/s]
		Real64 InletWaterEnthalpy; // Inlet Water Enthalpy [J/kg]
		Real64 OutletWaterEnthalpy; // Outlet Water Enthalpy [J/kg]
		Real64 Power; // Power Consumption [W]
		Real64 Energy; // Energy Consumption [J]
		Real64 QSensible; // Sensible Load Side Heat Transfer Rate [W]
		Real64 QLatent; // Latent Load Side Heat Transfer Rate [W]
		Real64 QSource; // Source Side Heat Transfer Rate [W]
		Real64 EnergySensible; // Sensible Load Side Heat Transferred [J]
		Real64 EnergyLatent; // Latent Load Side Heat Transferred [J]
		Real64 EnergySource; // Source Side Heat Transferred [J]
		Real64 RunFrac; // Duty Factor
		Real64 PartLoadRatio; // Part Load Ratio
		Real64 HeatingCapacity; // Nominal Heating Capacity
		Real64 CoolingCapacity; // Nominal Cooling Capacity
		Real64 QLoadTotal; // Load Side Total Heat Transfer Rate [W]
		Real64 EnergyLoadTotal; // Load Side Total Heat Transferred [J]
		Real64 Twet_Rated; // Nominal Time for Condensate Removal to Begin [s]
		Real64 Gamma_Rated; // Ratio of Initial Moisture Evaporation Rate and Steady-state Latent Capacity
		Real64 MaxONOFFCyclesperHour; // Maximum cycling rate of heat pump [cycles/hr]
		Real64 HPTimeConstant; // Heat pump time constant [s]
		Real64 FanDelayTime; // Fan delay time, time delay for the HP's fan to
		// shut off after compressor cycle off [s]
		Real64 SourceSideUACoeff; // Source Side Heat Transfer coefficient [W/C]
		Real64 LoadSideTotalUACoeff; // Load Side Total Heat Transfer coefficient [W/C]
		Real64 LoadSideOutsideUACoeff; // Load Side Outside Heat Transfer coefficient [W/C]
		Real64 CompPistonDisp; // Compressor Piston Displacement [m3/s]
		Real64 CompClearanceFactor; // Compressor Clearance Factor
		Real64 CompSucPressDrop; // Suction Pressure Drop [Pa]
		Real64 SuperheatTemp; // Superheat Temperature [C]
		Real64 PowerLosses; // Constant Part of the Compressor Power Losses [W]
		Real64 LossFactor; // Compressor Power Loss Factor
		Real64 RefVolFlowRate; // Refrigerant Volume Flow rate at the beginning
		// of the Compression [m3/s]
		Real64 VolumeRatio; // Built-in-volume ratio [~]
		Real64 LeakRateCoeff; // Coefficient for the relationship between
		// Pressure Ratio and Leakage Rate [~]
		Real64 SourceSideHTR1; // Source Side Heat Transfer Resistance coefficient 1 [~]
		Real64 SourceSideHTR2; // Source Side Heat Transfer Resistance coefficient 2 [k/kW]
		Real64 HighPressCutoff; // High Pressure Cut-off [Pa]
		Real64 LowPressCutoff; // Low Pressure Cut-off [Pa]
		int CompressorType; // Type of Compressor ie. Reciprocating,Rotary or Scroll
		int AirInletNodeNum; // air side coil inlet node number
		int AirOutletNodeNum; // air side coil outlet node number
		int WaterInletNodeNum; // water side coil inlet node number
		int WaterOutletNodeNum; // water side coil outlet node number
		int LowPressClgError; // count for low pressure errors (cooling)
		int HighPressClgError; // count for high pressure errors (cooling)
		int LowPressHtgError; // count for low pressure errors (heating)
		int HighPressHtgError; // count for high pressure errors (heating)
		int LoopNum; // plant loop index for water side
		int LoopSide; // plant loop side index
		int BranchNum; // plant branch index
		int CompNum; // plant component index

		// Default Constructor
		WatertoAirHPEquipConditions() :
			Name( MaxNameLength ),
			WatertoAirHPType( MaxNameLength ),
			WAHPPlantTypeOfNum( 0 ),
			Refrigerant( MaxNameLength ),
			SimFlag( false ),
			InletAirMassFlowRate( 0.0 ),
			OutletAirMassFlowRate( 0.0 ),
			InletAirDBTemp( 0.0 ),
			InletAirHumRat( 0.0 ),
			OutletAirDBTemp( 0.0 ),
			OutletAirHumRat( 0.0 ),
			InletAirEnthalpy( 0.0 ),
			OutletAirEnthalpy( 0.0 ),
			InletWaterTemp( 0.0 ),
			OutletWaterTemp( 0.0 ),
			InletWaterMassFlowRate( 0.0 ),
			OutletWaterMassFlowRate( 0.0 ),
			DesignWaterMassFlowRate( 0.0 ),
			DesignWaterVolFlowRate( 0.0 ),
			InletWaterEnthalpy( 0.0 ),
			OutletWaterEnthalpy( 0.0 ),
			Power( 0.0 ),
			Energy( 0.0 ),
			QSensible( 0.0 ),
			QLatent( 0.0 ),
			QSource( 0.0 ),
			EnergySensible( 0.0 ),
			EnergyLatent( 0.0 ),
			EnergySource( 0.0 ),
			RunFrac( 0.0 ),
			PartLoadRatio( 0.0 ),
			HeatingCapacity( 0.0 ),
			CoolingCapacity( 0.0 ),
			QLoadTotal( 0.0 ),
			EnergyLoadTotal( 0.0 ),
			Twet_Rated( 0.0 ),
			Gamma_Rated( 0.0 ),
			MaxONOFFCyclesperHour( 0.0 ),
			HPTimeConstant( 0.0 ),
			FanDelayTime( 0.0 ),
			SourceSideUACoeff( 0.0 ),
			LoadSideTotalUACoeff( 0.0 ),
			LoadSideOutsideUACoeff( 0.0 ),
			CompPistonDisp( 0.0 ),
			CompClearanceFactor( 0.0 ),
			CompSucPressDrop( 0.0 ),
			SuperheatTemp( 0.0 ),
			PowerLosses( 0.0 ),
			LossFactor( 0.0 ),
			RefVolFlowRate( 0.0 ),
			VolumeRatio( 0.0 ),
			LeakRateCoeff( 0.0 ),
			SourceSideHTR1( 0.0 ),
			SourceSideHTR2( 0.0 ),
			HighPressCutoff( 0.0 ),
			LowPressCutoff( 0.0 ),
			CompressorType( 0 ),
			AirInletNodeNum( 0 ),
			AirOutletNodeNum( 0 ),
			WaterInletNodeNum( 0 ),
			WaterOutletNodeNum( 0 ),
			LowPressClgError( 0 ),
			HighPressClgError( 0 ),
			LowPressHtgError( 0 ),
			HighPressHtgError( 0 ),
			LoopNum( 0 ),
			LoopSide( 0 ),
			BranchNum( 0 ),
			CompNum( 0 )
		{}

		// Member Constructor
		WatertoAirHPEquipConditions(
			Fstring const & Name, // Name of the Water to Air Heat pump
			Fstring const & WatertoAirHPType, // Type of WatertoAirHP ie. Heating or Cooling
			int const WAHPPlantTypeOfNum, // type of component in plant
			Fstring const & Refrigerant, // Refrigerant name
			bool const SimFlag,
			Real64 const InletAirMassFlowRate, // Inlet Air Mass Flow through the Water to Air Heat Pump being Simulated [kg/s]
			Real64 const OutletAirMassFlowRate, // Outlet Air Mass Flow through the Water to Air Heat Pump being Simulated [kg/s]
			Real64 const InletAirDBTemp, // Inlet Air Dry Bulb Temperature [C]
			Real64 const InletAirHumRat, // Inlet Air Humidity Ratio [kg/kg]
			Real64 const OutletAirDBTemp, // Outlet Air Dry Bulb Temperature [C]
			Real64 const OutletAirHumRat, // Outlet Air Humidity Ratio [kg/kg]
			Real64 const InletAirEnthalpy, // Inlet Air Enthalpy [J/kg]
			Real64 const OutletAirEnthalpy, // Outlet Air Enthalpy [J/kg]
			Real64 const InletWaterTemp, // Inlet Water Temperature [C]
			Real64 const OutletWaterTemp, // Outlet Water Temperature [C]
			Real64 const InletWaterMassFlowRate, // Inlet Water Mass Flow Rate [kg/s]
			Real64 const OutletWaterMassFlowRate, // Outlet Water Mass Flow Rate [kg/s]
			Real64 const DesignWaterMassFlowRate, // Design Water Mass Flow Rate [kg/s]
			Real64 const DesignWaterVolFlowRate, // Design Water Volumetric Flow Rate [m3/s]
			Real64 const InletWaterEnthalpy, // Inlet Water Enthalpy [J/kg]
			Real64 const OutletWaterEnthalpy, // Outlet Water Enthalpy [J/kg]
			Real64 const Power, // Power Consumption [W]
			Real64 const Energy, // Energy Consumption [J]
			Real64 const QSensible, // Sensible Load Side Heat Transfer Rate [W]
			Real64 const QLatent, // Latent Load Side Heat Transfer Rate [W]
			Real64 const QSource, // Source Side Heat Transfer Rate [W]
			Real64 const EnergySensible, // Sensible Load Side Heat Transferred [J]
			Real64 const EnergyLatent, // Latent Load Side Heat Transferred [J]
			Real64 const EnergySource, // Source Side Heat Transferred [J]
			Real64 const RunFrac, // Duty Factor
			Real64 const PartLoadRatio, // Part Load Ratio
			Real64 const HeatingCapacity, // Nominal Heating Capacity
			Real64 const CoolingCapacity, // Nominal Cooling Capacity
			Real64 const QLoadTotal, // Load Side Total Heat Transfer Rate [W]
			Real64 const EnergyLoadTotal, // Load Side Total Heat Transferred [J]
			Real64 const Twet_Rated, // Nominal Time for Condensate Removal to Begin [s]
			Real64 const Gamma_Rated, // Ratio of Initial Moisture Evaporation Rate and Steady-state Latent Capacity
			Real64 const MaxONOFFCyclesperHour, // Maximum cycling rate of heat pump [cycles/hr]
			Real64 const HPTimeConstant, // Heat pump time constant [s]
			Real64 const FanDelayTime, // Fan delay time, time delay for the HP's fan to
			Real64 const SourceSideUACoeff, // Source Side Heat Transfer coefficient [W/C]
			Real64 const LoadSideTotalUACoeff, // Load Side Total Heat Transfer coefficient [W/C]
			Real64 const LoadSideOutsideUACoeff, // Load Side Outside Heat Transfer coefficient [W/C]
			Real64 const CompPistonDisp, // Compressor Piston Displacement [m3/s]
			Real64 const CompClearanceFactor, // Compressor Clearance Factor
			Real64 const CompSucPressDrop, // Suction Pressure Drop [Pa]
			Real64 const SuperheatTemp, // Superheat Temperature [C]
			Real64 const PowerLosses, // Constant Part of the Compressor Power Losses [W]
			Real64 const LossFactor, // Compressor Power Loss Factor
			Real64 const RefVolFlowRate, // Refrigerant Volume Flow rate at the beginning
			Real64 const VolumeRatio, // Built-in-volume ratio [~]
			Real64 const LeakRateCoeff, // Coefficient for the relationship between
			Real64 const SourceSideHTR1, // Source Side Heat Transfer Resistance coefficient 1 [~]
			Real64 const SourceSideHTR2, // Source Side Heat Transfer Resistance coefficient 2 [k/kW]
			Real64 const HighPressCutoff, // High Pressure Cut-off [Pa]
			Real64 const LowPressCutoff, // Low Pressure Cut-off [Pa]
			int const CompressorType, // Type of Compressor ie. Reciprocating,Rotary or Scroll
			int const AirInletNodeNum, // air side coil inlet node number
			int const AirOutletNodeNum, // air side coil outlet node number
			int const WaterInletNodeNum, // water side coil inlet node number
			int const WaterOutletNodeNum, // water side coil outlet node number
			int const LowPressClgError, // count for low pressure errors (cooling)
			int const HighPressClgError, // count for high pressure errors (cooling)
			int const LowPressHtgError, // count for low pressure errors (heating)
			int const HighPressHtgError, // count for high pressure errors (heating)
			int const LoopNum, // plant loop index for water side
			int const LoopSide, // plant loop side index
			int const BranchNum, // plant branch index
			int const CompNum // plant component index
		) :
			Name( MaxNameLength, Name ),
			WatertoAirHPType( MaxNameLength, WatertoAirHPType ),
			WAHPPlantTypeOfNum( WAHPPlantTypeOfNum ),
			Refrigerant( MaxNameLength, Refrigerant ),
			SimFlag( SimFlag ),
			InletAirMassFlowRate( InletAirMassFlowRate ),
			OutletAirMassFlowRate( OutletAirMassFlowRate ),
			InletAirDBTemp( InletAirDBTemp ),
			InletAirHumRat( InletAirHumRat ),
			OutletAirDBTemp( OutletAirDBTemp ),
			OutletAirHumRat( OutletAirHumRat ),
			InletAirEnthalpy( InletAirEnthalpy ),
			OutletAirEnthalpy( OutletAirEnthalpy ),
			InletWaterTemp( InletWaterTemp ),
			OutletWaterTemp( OutletWaterTemp ),
			InletWaterMassFlowRate( InletWaterMassFlowRate ),
			OutletWaterMassFlowRate( OutletWaterMassFlowRate ),
			DesignWaterMassFlowRate( DesignWaterMassFlowRate ),
			DesignWaterVolFlowRate( DesignWaterVolFlowRate ),
			InletWaterEnthalpy( InletWaterEnthalpy ),
			OutletWaterEnthalpy( OutletWaterEnthalpy ),
			Power( Power ),
			Energy( Energy ),
			QSensible( QSensible ),
			QLatent( QLatent ),
			QSource( QSource ),
			EnergySensible( EnergySensible ),
			EnergyLatent( EnergyLatent ),
			EnergySource( EnergySource ),
			RunFrac( RunFrac ),
			PartLoadRatio( PartLoadRatio ),
			HeatingCapacity( HeatingCapacity ),
			CoolingCapacity( CoolingCapacity ),
			QLoadTotal( QLoadTotal ),
			EnergyLoadTotal( EnergyLoadTotal ),
			Twet_Rated( Twet_Rated ),
			Gamma_Rated( Gamma_Rated ),
			MaxONOFFCyclesperHour( MaxONOFFCyclesperHour ),
			HPTimeConstant( HPTimeConstant ),
			FanDelayTime( FanDelayTime ),
			SourceSideUACoeff( SourceSideUACoeff ),
			LoadSideTotalUACoeff( LoadSideTotalUACoeff ),
			LoadSideOutsideUACoeff( LoadSideOutsideUACoeff ),
			CompPistonDisp( CompPistonDisp ),
			CompClearanceFactor( CompClearanceFactor ),
			CompSucPressDrop( CompSucPressDrop ),
			SuperheatTemp( SuperheatTemp ),
			PowerLosses( PowerLosses ),
			LossFactor( LossFactor ),
			RefVolFlowRate( RefVolFlowRate ),
			VolumeRatio( VolumeRatio ),
			LeakRateCoeff( LeakRateCoeff ),
			SourceSideHTR1( SourceSideHTR1 ),
			SourceSideHTR2( SourceSideHTR2 ),
			HighPressCutoff( HighPressCutoff ),
			LowPressCutoff( LowPressCutoff ),
			CompressorType( CompressorType ),
			AirInletNodeNum( AirInletNodeNum ),
			AirOutletNodeNum( AirOutletNodeNum ),
			WaterInletNodeNum( WaterInletNodeNum ),
			WaterOutletNodeNum( WaterOutletNodeNum ),
			LowPressClgError( LowPressClgError ),
			HighPressClgError( HighPressClgError ),
			LowPressHtgError( LowPressHtgError ),
			HighPressHtgError( HighPressHtgError ),
			LoopNum( LoopNum ),
			LoopSide( LoopSide ),
			BranchNum( BranchNum ),
			CompNum( CompNum )
		{}

	};

	// Object Data
	extern FArray1D< WatertoAirHPEquipConditions > WatertoAirHP;

	// Functions

	void
	SimWatertoAirHP(
		Fstring const & CompName, // component name
		int & CompIndex, // Index for Component name
		Real64 const DesignAirflow, // design air flow rate
		int const CyclingScheme, // cycling scheme--either continuous fan/cycling compressor or
		bool const FirstHVACIteration, // first iteration flag
		Real64 const RuntimeFrac, // compressor run time fraction
		Real64 & MaxONOFFCyclesperHour, // Maximum cycling rate of heat pump [cycles/hr]
		Real64 & HPTimeConstant, // Heat pump time constant [s]
		Real64 & FanDelayTime, // Fan delay time, time delay for the HP's fan to
		bool const InitFlag, // initialization flag used to suppress property routine errors
		Real64 const SensLoad, // sensible load
		Real64 const LatentLoad, // latent load
		int const CompOp,
		Real64 const PartLoadRatio
	);

	// Get Input Section of the Module
	//******************************************************************************

	void
	GetWatertoAirHPInput();

	// End of Get Input subroutines for the HB Module
	//******************************************************************************

	// Beginning Initialization Section of the Module
	//******************************************************************************

	void
	InitWatertoAirHP(
		int const HPNum, // index to main heat pump data structure
		bool const InitFlag,
		Real64 const MaxONOFFCyclesperHour, // Maximum cycling rate of heat pump [cycles/hr]
		Real64 const HPTimeConstant, // Heat pump time constant [s]
		Real64 const FanDelayTime, // Fan delay time, time delay for the HP's fan to
		Real64 const SensLoad,
		Real64 const LatentLoad,
		Real64 const DesignAirFlow,
		Real64 const PartLoadRatio
	);

	// End Initialization Section of the Module
	//******************************************************************************

	// Begin Algorithm Section of the Module
	//******************************************************************************

	void
	CalcWatertoAirHPCooling(
		int const HPNum, // heat pump number
		int const CyclingScheme, // fan/compressor cycling scheme indicator
		bool const FirstHVACIteration, // first iteration flag
		Real64 const RuntimeFrac,
		bool const InitFlag, // suppress property errors if true
		Real64 const SensDemand,
		int const CompOp,
		Real64 const PartLoadRatio
	);

	Real64
	CalcCompSuctionTempResidual(
		Real64 const CompSuctionTemp, // HP compressor suction temperature (C)
		Optional< FArray1S< Real64 > const > Par = _ // Function parameters
	);

	void
	CalcWatertoAirHPHeating(
		int const HPNum, // heat pump number
		int const CyclingScheme, // fan/compressor cycling scheme indicator
		bool const FirstHVACIteration, // first iteration flag
		Real64 const RuntimeFrac,
		bool const InitFlag, // first iteration flag
		Real64 const SensDemand,
		int const CompOp,
		Real64 const PartLoadRatio
	);

	// End Algorithm Section of the Module
	// *****************************************************************************

	// End Algorithm Section of the Module
	// *****************************************************************************

	// Beginning of Update subroutines for the WatertoAirHP Module
	// *****************************************************************************

	void
	UpdateWatertoAirHP( int const HPNum );

	//        End of Update subroutines for the WatertoAirHP Module
	// *****************************************************************************

	Real64
	CalcEffectiveSHR(
		int const HPNum, // Index number for cooling coil
		Real64 const SHRss, // Steady-state sensible heat ratio
		int const CyclingScheme, // fan/compressor cycling scheme indicator
		Real64 const RTF, // Compressor run-time fraction
		Real64 const QLatRated, // Rated latent capacity
		Real64 const QLatActual, // Actual latent capacity
		Real64 const EnteringDB, // Entering air dry-bulb temperature
		Real64 const EnteringWB // Entering air wet-bulb temperature
	);

	Real64
	DegradF(
		Fstring & FluidName, // Name of glycol used in source side
		Real64 & Temp, // Temperature of the fluid
		int & FluidIndex // Index number for the fluid
	);

	int
	GetCoilIndex(
		Fstring const & CoilType, // must match coil types in this module
		Fstring const & CoilName, // must match coil names for the coil type
		bool & ErrorsFound // set to true if problem
	);

	Real64
	GetCoilCapacity(
		Fstring const & CoilType, // must match coil types in this module
		Fstring const & CoilName, // must match coil names for the coil type
		bool & ErrorsFound // set to true if problem
	);

	int
	GetCoilInletNode(
		Fstring const & CoilType, // must match coil types in this module
		Fstring const & CoilName, // must match coil names for the coil type
		bool & ErrorsFound // set to true if problem
	);

	int
	GetCoilOutletNode(
		Fstring const & CoilType, // must match coil types in this module
		Fstring const & CoilName, // must match coil names for the coil type
		bool & ErrorsFound // set to true if problem
	);

} // WaterToAirHeatPump

} // EnergyPlus

#endif
