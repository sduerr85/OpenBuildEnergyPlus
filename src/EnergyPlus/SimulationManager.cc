// C++ Headers
#include <cmath>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/environment.hh>
#include <ObjexxFCL/FArray.functions.hh>
#include <ObjexxFCL/FArray1D.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/Fstring.hh>
#include <ObjexxFCL/gio.hh>

// EnergyPlus Headers
#include <SimulationManager.hh>
#include <BranchInputManager.hh>
#include <BranchNodeConnections.hh>
#include <CostEstimateManager.hh>
#include <CurveManager.hh>
#include <DataAirLoop.hh>
#include <DataBranchNodeConnections.hh>
#include <DataContaminantBalance.hh>
#include <DataConvergParams.hh>
#include <DataEnvironment.hh>
#include <DataErrorTracking.hh>
#include <DataGlobalConstants.hh>
#include <DataGlobals.hh>
#include <DataHeatBalance.hh>
#include <DataHeatBalFanSys.hh>
#include <DataHVACGlobals.hh>
#include <DataIPShortCuts.hh>
#include <DataLoopNode.hh>
#include <DataOutputs.hh>
#include <DataPlant.hh>
#include <DataPrecisionGlobals.hh>
#include <DataReportingFlags.hh>
#include <DataRuntimeLanguage.hh>
#include <DataSizing.hh>
#include <DataStringGlobals.hh>
#include <DataSurfaces.hh>
#include <DataSystemVariables.hh>
#include <DataTimings.hh>
#include <DataZoneEquipment.hh>
#include <DemandManager.hh>
#include <DisplayRoutines.hh>
#include <DualDuct.hh>
#include <EconomicLifeCycleCost.hh>
#include <EconomicTariff.hh>
#include <EMSManager.hh>
#include <ExteriorEnergyUse.hh>
#include <ExternalInterface.hh>
#include <FaultsManager.hh>
#include <FluidProperties.hh>
#include <General.hh>
#include <GeneralRoutines.hh>
#include <HeatBalanceAirManager.hh>
#include <HeatBalanceManager.hh>
#include <HeatBalanceSurfaceManager.hh>
#include <HVACControllers.hh>
#include <HVACManager.hh>
#include <InputProcessor.hh>
#include <ManageElectricPower.hh>
#include <MixedAir.hh>
#include <NodeInputManager.hh>
#include <OutAirNodeManager.hh>
#include <OutputProcessor.hh>
#include <OutputReportPredefined.hh>
#include <OutputReportTabular.hh>
#include <OutputReports.hh>
#include <PlantManager.hh>
#include <PollutionModule.hh>
#include <Psychrometrics.hh>
#include <RefrigeratedCase.hh>
#include <SetPointManager.hh>
#include <SizingManager.hh>
#include <SolarShading.hh>
#include <SQLiteProcedures.hh>
#include <SystemReports.hh>
#include <UtilityRoutines.hh>
#include <WeatherManager.hh>
#include <ZoneContaminantPredictorCorrector.hh>
#include <ZoneTempPredictorCorrector.hh>
#include <Timer.h>

namespace EnergyPlus {

// HBIRE_USE_OMP defined, then openMP instructions are used.  Compiler may have to have switch for openmp
// HBIRE_NO_OMP defined, then old code is used without any openmp instructions
// HBIRE - loop in HeatBalanceIntRadExchange.f90

#ifdef HBIRE_USE_OMP
#undef HBIRE_NO_OMP
#else
#define HBIRE_NO_OMP
#endif

namespace SimulationManager {

	// MODULE INFORMATION:
	//       AUTHOR         Rick Strand
	//       DATE WRITTEN   January 1997
	//       MODIFIED       na
	//       RE-ENGINEERED  na

	// PURPOSE OF THIS MODULE:
	// This module contains the main driver routine which manages the major
	// control loops of the EnergyPlus simulation.  This module is also
	// responsible for setting the global environment flags for these
	// loops.

	// METHODOLOGY EMPLOYED:
	// This module was constructed from the remnants of (I)BLAST routines
	// SIMBLD (Simulate Building), SIMZG (Simulate Zone Group), and SIMZGD
	// (Simulate Zone Group for a Day).

	// REFERENCES:
	// (I)BLAST legacy code, internal Reverse Engineering documentation,
	// and internal Evolutionary Engineering documentation.

	// Using/Aliasing
	using namespace DataPrecisionGlobals;
	using namespace DataGlobals;
	using namespace DataSizing;
	using namespace DataReportingFlags;
	using namespace DataSystemVariables;
	using namespace HeatBalanceManager;
	using namespace WeatherManager;
	using namespace ExternalInterface;

	// Data
	// MODULE PARAMETER DEFINITIONS:
	// na

	// DERIVED TYPE DEFINITIONS:
	// na

	// INTERFACE BLOCK SPECIFICATIONS:
	// na

	// MODULE VARIABLE DECLARATIONS:
	bool RunPeriodsInInput( false );
	bool RunControlInInput( false );

	// SUBROUTINE SPECIFICATIONS FOR MODULE SimulationManager

	// MODULE SUBROUTINES:

	// Functions

	void
	ManageSimulation()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Rick Strand
		//       DATE WRITTEN   January 1997
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// This subroutine is the main driver of the simulation manager module.
		// It contains the main environment-time loops for the building
		// simulation.  This includes the environment loop, a day loop, an
		// hour loop, and a time step loop.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using DataHVACGlobals::TimeStepSys;
		using DataEnvironment::EnvironmentName;
		using DataEnvironment::CurMnDy;
		using DataEnvironment::CurrentOverallSimDay;
		using DataEnvironment::TotalOverallSimDays;
		using DataEnvironment::TotDesDays;
		using DataEnvironment::TotRunDesPersDays;
		using DataEnvironment::EndMonthFlag;
		using InputProcessor::GetNumRangeCheckErrorsFound;
		using InputProcessor::GetNumObjectsFound;
		using SizingManager::ManageSizing;
		using ExteriorEnergyUse::ManageExteriorEnergyUse;
		using OutputReportTabular::WriteTabularReports;
		using OutputReportTabular::OpenOutputTabularFile;
		using OutputReportTabular::CloseOutputTabularFile;
		using DataErrorTracking::AskForConnectionsReport;
		using DataErrorTracking::ExitDuringSimulations;
		using OutputProcessor::SetupTimePointers;
		using OutputProcessor::ReportForTabularReports;
		using CostEstimateManager::SimCostEstimate;
		using EconomicTariff::ComputeTariff; // added for computing annual utility costs
		using EconomicTariff::WriteTabularTariffReports;
		using General::TrimSigDigits;
		using OutputReportPredefined::SetPredefinedTables;
		using HVACControllers::DumpAirLoopStatistics;
		using NodeInputManager::SetupNodeVarsForReporting;
		using NodeInputManager::CheckMarkedNodes;
		using BranchNodeConnections::CheckNodeConnections;
		using BranchNodeConnections::TestCompSetInletOutletNodes;
		using PollutionModule::SetupPollutionMeterReporting;
		using PollutionModule::SetupPollutionCalculations;
		using PollutionModule::CheckPollutionMeterReporting;
		using SystemReports::ReportAirLoopConnections;
		using SystemReports::CreateEnergyReportStructure;
		using BranchInputManager::ManageBranchInput;
		using BranchInputManager::TestBranchIntegrity;
		using BranchInputManager::InvalidBranchDefinitions;
		using ManageElectricPower::VerifyCustomMetersElecPowerMgr;
		using MixedAir::CheckControllerLists;
		using EMSManager::CheckIfAnyEMS;
		using EMSManager::ManageEMS;
		using EconomicLifeCycleCost::GetInputForLifeCycleCost;
		using EconomicLifeCycleCost::ComputeLifeCycleCostAndReport;
		using SQLiteProcedures::WriteOutputToSQLite;
		using SQLiteProcedures::CreateSQLiteSimulationsRecord;
		using SQLiteProcedures::InitializeIndexes;
		using SQLiteProcedures::CreateSQLiteEnvironmentPeriodRecord;
		using SQLiteProcedures::CreateZoneExtendedOutput;
		using SQLiteProcedures::SQLiteBegin;
		using SQLiteProcedures::SQLiteCommit;
		using DemandManager::InitDemandManagers;
		using PlantManager::CheckIfAnyPlant;
		using CurveManager::InitCurveReporting;
		using namespace DataTimings;
		using DataSystemVariables::DeveloperFlag;
		using DataSystemVariables::TimingFlag;
		using DataSystemVariables::FullAnnualRun;
		using SetPointManager::CheckIfAnyIdealCondEntSetPoint;
		using Psychrometrics::InitializePsychRoutines;
		using namespace FaultsManager;

		// Locals
		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		static bool Available; // an environment is available to process
		static bool ErrorsFound( false );
		static bool TerminalError( false );
		bool SimsDone;
		bool ErrFound;
		//  REAL(r64) :: t0,t1,st0,st1

		//  CHARACTER(len=70) :: tdstring
		//  CHARACTER(len=138) :: tdstringlong

		int EnvCount;

		// Formats
		std::string const Format_700( "('Environment:WarmupDays,',I3)" );

		// FLOW:
		PostIPProcessing();

		InitializePsychRoutines();

		BeginSimFlag = true;
		BeginFullSimFlag = false;
		DoOutputReporting = false;
		DisplayPerfSimulationFlag = false;
		DoWeatherInitReporting = false;
		RunPeriodsInInput = ( GetNumObjectsFound( "RunPeriod" ) > 0 || GetNumObjectsFound( "RunPeriod:CustomRange" ) > 0 || FullAnnualRun );
		AskForConnectionsReport = false; // set to false until sizing is finished

		OpenOutputFiles();
		CheckThreading();
		GetProjectData();
		CheckForMisMatchedEnvironmentSpecifications();
		CheckForRequestedReporting();
		SetPredefinedTables();

		SetupTimePointers( "Zone", TimeStepZone ); // Set up Time pointer for HB/Zone Simulation
		SetupTimePointers( "HVAC", TimeStepSys );

		CheckIfAnyEMS();
		CheckIfAnyPlant();

		CheckIfAnyIdealCondEntSetPoint();

		CheckAndReadFaults();

		ManageBranchInput(); // just gets input and returns.

		DoingSizing = true;
		ManageSizing();

		BeginFullSimFlag = true;
		SimsDone = false;
		if ( DoDesDaySim || DoWeathSim ) {
			DoOutputReporting = true;
		}
		DoingSizing = false;

		if ( ( DoZoneSizing || DoSystemSizing || DoPlantSizing ) && ! ( DoDesDaySim || ( DoWeathSim && RunPeriodsInInput ) ) ) {
			ShowWarningError( "ManageSimulation: Input file has requested Sizing Calculations but no Simulations are requested " "(in SimulationControl object). Succeeding warnings/errors may be confusing." );
		}
		Available = true;

		if ( InvalidBranchDefinitions ) {
			ShowFatalError( "Preceding error(s) in Branch Input cause termination." );
		}

		DisplayString( "Initializing Simulation" );
		KickOffSimulation = true;

		ResetEnvironmentCounter();
		SetupSimulation( ErrorsFound );
		InitCurveReporting();

		AskForConnectionsReport = true; // set to true now that input processing and sizing is done.
		KickOffSimulation = false;
		WarmupFlag = false;
		DoWeatherInitReporting = true;

		//  Note:  All the inputs have been 'gotten' by the time we get here.
		ErrFound = false;
		if ( DoOutputReporting ) {
			DisplayString( "Reporting Surfaces" );

			ReportSurfaces();

			SetupNodeVarsForReporting();
			MetersHaveBeenInitialized = true;
			SetupPollutionMeterReporting();
			UpdateMeterReporting();
			CheckPollutionMeterReporting();
			VerifyCustomMetersElecPowerMgr();
			SetupPollutionCalculations();
			InitDemandManagers();

			TestBranchIntegrity( ErrFound );
			if ( ErrFound ) TerminalError = true;
			TestAirPathIntegrity( ErrFound );
			if ( ErrFound ) TerminalError = true;
			CheckMarkedNodes( ErrFound );
			if ( ErrFound ) TerminalError = true;
			CheckNodeConnections( ErrFound );
			if ( ErrFound ) TerminalError = true;
			TestCompSetInletOutletNodes( ErrFound );
			if ( ErrFound ) TerminalError = true;
			CheckControllerLists( ErrFound );
			if ( ErrFound ) TerminalError = true;

			if ( DoDesDaySim || DoWeathSim ) {
				ReportLoopConnections();
				ReportAirLoopConnections();
				ReportNodeConnections();
				// Debug reports
				//      CALL ReportCompSetMeterVariables
				//      CALL ReportParentChildren
			}

			CreateEnergyReportStructure();

			ManageEMS( emsCallFromSetupSimulation ); // point to finish setup processing EMS, sensor ready now

			ProduceRDDMDD();

			if ( TerminalError ) {
				ShowFatalError( "Previous Conditions cause program termination." );
			}
		}

		if ( WriteOutputToSQLite ) {
			SQLiteBegin();
			CreateSQLiteSimulationsRecord( 1 );
			SQLiteCommit();
		}

		GetInputForLifeCycleCost(); //must be prior to WriteTabularReports -- do here before big simulation stuff.

		ShowMessage( "Beginning Simulation" );
		ResetEnvironmentCounter();

		EnvCount = 0;
		WarmupFlag = true;

		while ( Available ) {

			GetNextEnvironment( Available, ErrorsFound );

			if ( ! Available ) break;
			if ( ErrorsFound ) break;
			if ( ( ! DoDesDaySim ) && ( KindOfSim != ksRunPeriodWeather ) ) continue;
			if ( ( ! DoWeathSim ) && ( KindOfSim == ksRunPeriodWeather ) ) continue;

			++EnvCount;

			if ( WriteOutputToSQLite ) {
				SQLiteBegin();
				CreateSQLiteEnvironmentPeriodRecord();
				SQLiteCommit();
			}

			ExitDuringSimulations = true;
			SimsDone = true;
			DisplayString( "Initializing New Environment Parameters" );

			BeginEnvrnFlag = true;
			EndEnvrnFlag = false;
			EndMonthFlag = false;
			WarmupFlag = true;
			DayOfSim = 0;
			DayOfSimChr = "0";
			NumOfWarmupDays = 0;

			ManageEMS( emsCallFromBeginNewEvironment ); // calling point

			while ( ( DayOfSim < NumOfDayInEnvrn ) || ( WarmupFlag ) ) { // Begin day loop ...

				if ( WriteOutputToSQLite ) SQLiteBegin(); // setup for one transaction per day

				++DayOfSim;
				gio::write( DayOfSimChr, "*" ) << DayOfSim;
				DayOfSimChr = adjustl( DayOfSimChr );
				if ( ! WarmupFlag ) {
					++CurrentOverallSimDay;
					DisplaySimDaysProgress( CurrentOverallSimDay, TotalOverallSimDays );
				} else {
					DayOfSimChr = "0";
				}
				BeginDayFlag = true;
				EndDayFlag = false;

				if ( WarmupFlag ) {
					++NumOfWarmupDays;
					cWarmupDay = TrimSigDigits( NumOfWarmupDays );
					DisplayString( "Warming up {" + trim( cWarmupDay ) + "}" );
				} else if ( DayOfSim == 1 ) {
					DisplayString( "Starting Simulation at " + trim( CurMnDy ) + " for " + trim( EnvironmentName ) );
					gio::write( OutputFileInits, Format_700 ) << NumOfWarmupDays;
				} else if ( DisplayPerfSimulationFlag ) {
					DisplayString( "Continuing Simulation at " + trim( CurMnDy ) + " for " + trim( EnvironmentName ) );
					DisplayPerfSimulationFlag = false;
				}

				for ( HourOfDay = 1; HourOfDay <= 24; ++HourOfDay ) { // Begin hour loop ...

					BeginHourFlag = true;
					EndHourFlag = false;

					for ( TimeStep = 1; TimeStep <= NumOfTimeStepInHour; ++TimeStep ) {

						BeginTimeStepFlag = true;
						ExternalInterfaceExchangeVariables();

						// Set the End__Flag variables to true if necessary.  Note that
						// each flag builds on the previous level.  EndDayFlag cannot be
						// .TRUE. unless EndHourFlag is also .TRUE., etc.  Note that the
						// EndEnvrnFlag and the EndSimFlag cannot be set during warmup.
						// Note also that BeginTimeStepFlag, EndTimeStepFlag, and the
						// SubTimeStepFlags can/will be set/reset in the HVAC Manager.

						if ( ( TimeStep == NumOfTimeStepInHour ) ) {
							EndHourFlag = true;
							if ( HourOfDay == 24 ) {
								EndDayFlag = true;
								if ( ( ! WarmupFlag ) && ( DayOfSim == NumOfDayInEnvrn ) ) {
									EndEnvrnFlag = true;
								}
							}
						}

						ManageWeather();

						ManageExteriorEnergyUse();

						ManageHeatBalance();

						//  After the first iteration of HeatBalance, all the 'input' has been gotten
						if ( BeginFullSimFlag ) {
							if ( GetNumRangeCheckErrorsFound() > 0 ) {
								ShowFatalError( "Out of \"range\" values found in input" );
							}
						}

						BeginHourFlag = false;
						BeginDayFlag = false;
						BeginEnvrnFlag = false;
						BeginSimFlag = false;
						BeginFullSimFlag = false;

					} // TimeStep loop

					PreviousHour = HourOfDay;

				} // ... End hour loop.

				if ( WriteOutputToSQLite ) SQLiteCommit(); // one transaction per day

			} // ... End day loop.

			// Need one last call to send latest states to middleware
			ExternalInterfaceExchangeVariables();

		} // ... End environment loop.

		WarmupFlag = false;
		if ( ! SimsDone && DoDesDaySim ) {
			if ( ( TotDesDays + TotRunDesPersDays ) == 0 ) { // if sum is 0, then there was no sizing done.
				ShowWarningError( "ManageSimulation: SizingPeriod:* were requested in SimulationControl  " "but no SizingPeriod:* objects in input." );
			}
		}

		if ( ! SimsDone && DoWeathSim ) {
			if ( ! RunPeriodsInInput ) { // if no run period requested, and sims not done
				ShowWarningError( "ManageSimulation: Weather Simulation was requested in SimulationControl " "but no RunPeriods in input." );
			}
		}

		if ( WriteOutputToSQLite ) SQLiteBegin(); // for final data to write

#ifdef EP_Detailed_Timings
		epStartTime( "Closeout Reporting=" );
#endif
		SimCostEstimate();

		ComputeTariff(); //     Compute the utility bills

		ReportForTabularReports(); // For Energy Meters (could have other things that need to be pushed to after simulation)

		OpenOutputTabularFile();

		WriteTabularReports(); //     Create the tabular reports at completion of each

		WriteTabularTariffReports();

		ComputeLifeCycleCostAndReport(); //must be after WriteTabularReports and WriteTabularTariffReports

		CloseOutputTabularFile();

		DumpAirLoopStatistics(); // Dump runtime statistics for air loop controller simulation to csv file

#ifdef EP_Detailed_Timings
		epStopTime( "Closeout Reporting=" );
#endif
		CloseOutputFiles();

		CreateZoneExtendedOutput();

		if ( WriteOutputToSQLite ) {
			DisplayString( "Writing final SQL reports" );
			SQLiteCommit(); // final transactions
			InitializeIndexes(); // do not create indexes (SQL) until all is done.
		}

		if ( ErrorsFound ) {
			ShowFatalError( "Error condition occurred.  Previous Severe Errors cause termination." );
		}

	}

	void
	GetProjectData()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda K. Lawrie
		//       DATE WRITTEN   November 1997
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// This subroutine gets global project data from the input file.

		// METHODOLOGY EMPLOYED:
		// Use GetObjectItem from the Input Processor

		// REFERENCES:
		// na

		// Using/Aliasing
		using namespace InputProcessor;
		using DataStringGlobals::MatchVersion;
		using namespace DataConvergParams;
		using namespace DataSystemVariables;
		using DataHVACGlobals::LimitNumSysSteps;
		using DataHVACGlobals::deviationFromSetPtThresholdHtg;
		using DataHVACGlobals::deviationFromSetPtThresholdClg;
		using General::RoundSigDigits;
		using DataEnvironment::DisplayWeatherMissingDataWarnings;
		using DataEnvironment::IgnoreSolarRadiation;
		using DataEnvironment::IgnoreBeamRadiation;
		using DataEnvironment::IgnoreDiffuseRadiation;
		using namespace DataIPShortCuts;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		static FArray1D_int const Div60( 12, { 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60 } );
		static Fstring const Blank;
		static Fstring const fmtA( "(A)" );

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		FArray1D_Fstring Alphas( 5, sFstring( MaxNameLength ) );
		FArray1D< Real64 > Number( 4 );
		int NumAlpha;
		int NumNumber;
		int IOStat;
		int NumDebugOut;
		int MinInt;
		int Num;
		int Which;
		bool ErrorsFound;
		int Num1;
		int NumA;
		int NumRunControl;
		static Fstring VersionID( 20 );
		Fstring CurrentModuleObject( MaxNameLength );
		bool CondFDAlgo;
		int Item;

		// Formats
		std::string const Format_721( "(' Version, ',A)" );
		std::string const Format_731( "(' Timesteps per Hour, ',I2,', ',I2)" );
		std::string const Format_733( "(' System Convergence Limits',4(', ',A))" );
		std::string const Format_741( "(' Simulation Control',5(', ',A))" );
		std::string const Format_751( "(' Output Reporting Tolerances',5(', ',A))" );

		ErrorsFound = false;

		CurrentModuleObject = "Version";
		Num = GetNumObjectsFound( CurrentModuleObject );
		if ( Num == 1 ) {
			GetObjectItem( CurrentModuleObject, 1, Alphas, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
			Num1 = len_trim( MatchVersion );
			if ( MatchVersion( {Num1,Num1} ) == "0" ) {
				Which = index( Alphas( 1 )( 1, Num1 - 2 ), MatchVersion( {1,Num1 - 2} ) );
			} else {
				Which = index( Alphas( 1 ), MatchVersion );
			}
			if ( Which != 1 ) {
				ShowWarningError( trim( CurrentModuleObject ) + ": in IDF=\"" + trim( Alphas( 1 ) ) + "\" not the same as expected=\"" + trim( MatchVersion ) + "\"" );
			}
			VersionID = Alphas( 1 );
		} else if ( Num == 0 ) {
			ShowWarningError( trim( CurrentModuleObject ) + ": missing in IDF, processing for EnergyPlus version=\"" + trim( MatchVersion ) + "\"" );
		} else {
			ShowSevereError( "Too many " + trim( CurrentModuleObject ) + " Objects found." );
			ErrorsFound = true;
		}

		// Do Mini Gets on HB Algorithm and by-surface overrides
		CurrentModuleObject = "HeatBalanceAlgorithm";
		Num = GetNumObjectsFound( CurrentModuleObject );
		CondFDAlgo = false;
		if ( Num > 0 ) {
			GetObjectItem( CurrentModuleObject, 1, Alphas, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
			{ auto const SELECT_CASE_var( Alphas( 1 ) );
			if ( ( SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE" ) || ( SELECT_CASE_var == "CONDFD" ) || ( SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCEDETAILED" ) || ( SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCESIMPLIFIED" ) ) {
				CondFDAlgo = true;
			} else {
			}}
		}
		CurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm";
		Num = GetNumObjectsFound( CurrentModuleObject );
		if ( Num > 0 ) {
			for ( Item = 1; Item <= Num; ++Item ) {
				GetObjectItem( CurrentModuleObject, Item, Alphas, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
				{ auto const SELECT_CASE_var( Alphas( 2 ) );
				if ( SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE" ) {
					CondFDAlgo = true;

				} else {
				}}
			}
		}
		CurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm:MultipleSurface";
		Num = GetNumObjectsFound( CurrentModuleObject );
		if ( Num > 0 ) {
			for ( Item = 1; Item <= Num; ++Item ) {
				GetObjectItem( CurrentModuleObject, 1, Alphas, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
				{ auto const SELECT_CASE_var( Alphas( 3 ) );
				if ( SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE" ) {
					CondFDAlgo = true;
				} else {
				}}
			}
		}
		CurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm:SurfaceList";
		Num = GetNumObjectsFound( CurrentModuleObject );
		if ( Num > 0 ) {
			for ( Item = 1; Item <= Num; ++Item ) {
				GetObjectItem( CurrentModuleObject, 1, cAlphaArgs, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
				{ auto const SELECT_CASE_var( cAlphaArgs( 2 ) );
				if ( SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE" ) {
					CondFDAlgo = true;
				} else {
				}}
			}
		}
		CurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm:Construction";
		Num = GetNumObjectsFound( CurrentModuleObject );
		if ( Num > 0 ) {
			for ( Item = 1; Item <= Num; ++Item ) {
				GetObjectItem( CurrentModuleObject, 1, cAlphaArgs, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
				{ auto const SELECT_CASE_var( cAlphaArgs( 2 ) );
				if ( SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE" ) {
					CondFDAlgo = true;
				} else {
				}}
			}
		}

		CurrentModuleObject = "Timestep";
		Num = GetNumObjectsFound( CurrentModuleObject );
		if ( Num == 1 ) {
			GetObjectItem( CurrentModuleObject, 1, Alphas, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
			NumOfTimeStepInHour = Number( 1 );
			if ( NumOfTimeStepInHour <= 0 || NumOfTimeStepInHour > 60 ) {
				Alphas( 1 ) = RoundSigDigits( NumOfTimeStepInHour );
				ShowWarningError( trim( CurrentModuleObject ) + ": Requested number (" + trim( Alphas( 1 ) ) + ") invalid, Defaulted to 4" );
				NumOfTimeStepInHour = 4;
			} else if ( mod( 60, NumOfTimeStepInHour ) != 0 ) {
				MinInt = 9999;
				for ( Num = 1; Num <= 12; ++Num ) {
					if ( std::abs( NumOfTimeStepInHour - Div60( Num ) ) > MinInt ) continue;
					MinInt = NumOfTimeStepInHour - Div60( Num );
					Which = Num;
				}
				ShowWarningError( trim( CurrentModuleObject ) + ": Requested number (" + trim( RoundSigDigits( NumOfTimeStepInHour ) ) + ") not evenly divisible into 60, " "defaulted to nearest (" + trim( RoundSigDigits( Div60( Which ) ) ) + ")." );
				NumOfTimeStepInHour = Div60( Which );
			}
			if ( CondFDAlgo && NumOfTimeStepInHour < 20 ) {
				ShowWarningError( trim( CurrentModuleObject ) + ": Requested number (" + trim( RoundSigDigits( NumOfTimeStepInHour ) ) + ") cannot be used when Conduction Finite Difference algorithm is selected." );
				ShowContinueError( "..." + trim( CurrentModuleObject ) + " is set to 20." );
				NumOfTimeStepInHour = 20;
			}
			if ( NumOfTimeStepInHour < 4 && GetNumObjectsFound( "Zone" ) > 0 ) {
				ShowWarningError( trim( CurrentModuleObject ) + ": Requested number (" + trim( RoundSigDigits( NumOfTimeStepInHour ) ) + ") is less than the suggested minimum of 4." );
				ShowContinueError( "Please see entry for " + trim( CurrentModuleObject ) + " in Input/Output Reference for discussion of considerations." );
			}
		} else if ( Num == 0 && GetNumObjectsFound( "Zone" ) > 0 && ! CondFDAlgo ) {
			ShowWarningError( "No " + trim( CurrentModuleObject ) + " object found.  Number of TimeSteps in Hour defaulted to 4." );
			NumOfTimeStepInHour = 4;
		} else if ( Num == 0 && ! CondFDAlgo ) {
			NumOfTimeStepInHour = 4;
		} else if ( Num == 0 && GetNumObjectsFound( "Zone" ) > 0 && CondFDAlgo ) {
			ShowWarningError( "No " + trim( CurrentModuleObject ) + " object found.  Number of TimeSteps in Hour defaulted to 20." );
			ShowContinueError( "...Due to presence of Conduction Finite Difference Algorithm selection." );
			NumOfTimeStepInHour = 20;
		} else if ( Num == 0 && CondFDAlgo ) {
			NumOfTimeStepInHour = 20;
		} else {
			ShowSevereError( "Too many " + trim( CurrentModuleObject ) + " Objects found." );
			ErrorsFound = true;
		}

		TimeStepZone = 1.0 / double( NumOfTimeStepInHour );
		MinutesPerTimeStep = TimeStepZone * 60;

		CurrentModuleObject = "ConvergenceLimits";
		Num = GetNumObjectsFound( CurrentModuleObject );
		if ( Num == 1 ) {
			GetObjectItem( CurrentModuleObject, 1, Alphas, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
			MinInt = int( Number( 1 ) );
			if ( MinInt > MinutesPerTimeStep ) {
				MinInt = MinutesPerTimeStep;
			}
			if ( MinInt < 0 || MinInt > 60 ) {
				ShowWarningError( trim( CurrentModuleObject ) + ": Requested " + trim( cNumericFieldNames( 1 ) ) + " (" + trim( RoundSigDigits( MinInt ) ) + ") invalid. Set to 1 minute." );
				MinTimeStepSys = 1.0 / 60.0;
			} else if ( MinInt == 0 ) { // Set to TimeStepZone
				MinTimeStepSys = TimeStepZone;
			} else {
				MinTimeStepSys = double( MinInt ) / 60.0;
			}
			MaxIter = int( Number( 2 ) );
			if ( MaxIter <= 0 ) {
				MaxIter = 20;
			}
			if ( ! lNumericFieldBlanks( 3 ) ) MinPlantSubIterations = int( Number( 3 ) );
			if ( ! lNumericFieldBlanks( 4 ) ) MaxPlantSubIterations = int( Number( 4 ) );
			// trap bad values
			if ( MinPlantSubIterations < 1 ) MinPlantSubIterations = 1;
			if ( MaxPlantSubIterations < 3 ) MaxPlantSubIterations = 3;
			if ( MinPlantSubIterations > MaxPlantSubIterations ) MaxPlantSubIterations = MinPlantSubIterations + 1;

		} else if ( Num == 0 ) {
			MinTimeStepSys = 1.0 / 60.0;
			MaxIter = 20;
			MinPlantSubIterations = 2;
			MaxPlantSubIterations = 8;
		} else {
			ShowSevereError( "Too many " + trim( CurrentModuleObject ) + " Objects found." );
			ErrorsFound = true;
		}

		LimitNumSysSteps = int( TimeStepZone / MinTimeStepSys );

		DebugOutput = false;
		EvenDuringWarmup = false;
		CurrentModuleObject = "Output:DebuggingData";
		NumDebugOut = GetNumObjectsFound( CurrentModuleObject );
		if ( NumDebugOut > 0 ) {
			GetObjectItem( CurrentModuleObject, 1, Alphas, NumAlpha, Number, NumNumber, IOStat );
			if ( int( Number( 1 ) ) == 1 ) {
				DebugOutput = true;
			}
			if ( int( Number( 2 ) ) == 1 ) {
				EvenDuringWarmup = true;
			}
		}

		CurrentModuleObject = "Output:Diagnostics";
		Num = GetNumObjectsFound( CurrentModuleObject );
		for ( Num1 = 1; Num1 <= Num; ++Num1 ) {
			GetObjectItem( CurrentModuleObject, Num1, Alphas, NumAlpha, Number, NumNumber, IOStat );
			for ( NumA = 1; NumA <= NumAlpha; ++NumA ) {
				if ( SameString( Alphas( NumA ), "DisplayExtraWarnings" ) ) {
					DisplayExtraWarnings = true;
				} else if ( SameString( Alphas( NumA ), "DisplayAdvancedReportVariables" ) ) {
					DisplayAdvancedReportVariables = true;
				} else if ( SameString( Alphas( NumA ), "DisplayAllWarnings" ) ) {
					DisplayAllWarnings = true;
					DisplayExtraWarnings = true;
					DisplayUnusedObjects = true;
					DisplayUnusedSchedules = true;
				} else if ( SameString( Alphas( NumA ), "DisplayUnusedObjects" ) ) {
					DisplayUnusedObjects = true;
				} else if ( SameString( Alphas( NumA ), "DisplayUnusedSchedules" ) ) {
					DisplayUnusedSchedules = true;
				} else if ( SameString( Alphas( NumA ), "DisplayZoneAirHeatBalanceOffBalance" ) ) {
					DisplayZoneAirHeatBalanceOffBalance = true;
				} else if ( SameString( Alphas( NumA ), "DoNotMirrorDetachedShading" ) ) {
					MakeMirroredDetachedShading = false;
				} else if ( SameString( Alphas( NumA ), "DoNotMirrorAttachedShading" ) ) {
					MakeMirroredAttachedShading = false;
				} else if ( SameString( Alphas( NumA ), "IgnoreInteriorWindowTransmission" ) ) {
					IgnoreInteriorWindowTransmission = true;
				} else if ( SameString( Alphas( NumA ), "ReportDuringWarmup" ) ) {
					ReportDuringWarmup = true;
				} else if ( SameString( Alphas( NumA ), "DisplayWeatherMissingDataWarnings" ) ) {
					DisplayWeatherMissingDataWarnings = true;
				} else if ( SameString( Alphas( NumA ), "IgnoreSolarRadiation" ) ) {
					IgnoreSolarRadiation = true;
				} else if ( SameString( Alphas( NumA ), "IgnoreBeamRadiation" ) ) {
					IgnoreBeamRadiation = true;
				} else if ( SameString( Alphas( NumA ), "IgnoreDiffuseRadiation" ) ) {
					IgnoreDiffuseRadiation = true;
				} else if ( SameString( Alphas( NumA ), "DeveloperFlag" ) ) {
					DeveloperFlag = true;
				} else if ( SameString( Alphas( NumA ), "TimingFlag" ) ) {
					TimingFlag = true;
				} else if ( SameString( Alphas( NumA ), "ReportDetailedWarmupConvergence" ) ) {
					ReportDetailedWarmupConvergence = true;
				} else if ( SameString( Alphas( NumA ), "CreateMinimalSurfaceVariables" ) ) {
					continue;
					//        CreateMinimalSurfaceVariables=.TRUE.
				} else if ( SameString( Alphas( NumA ), "CreateNormalSurfaceVariables" ) ) {
					continue;
					//        IF (CreateMinimalSurfaceVariables) THEN
					//          CALL ShowWarningError('GetProjectData: '//TRIM(CurrentModuleObject)//'=''//  &
					//             TRIM(Alphas(NumA))//'', prior set=true for this condition reverts to false.')
					//        ENDIF
					//        CreateMinimalSurfaceVariables=.FALSE.
				} else if ( Alphas( NumA ) != Blank ) {
					ShowWarningError( "GetProjectData: " + trim( CurrentModuleObject ) + "=\"" + trim( Alphas( NumA ) ) + "\", Invalid value for field, entered value ignored." );
				}
			}
		}

		CurrentModuleObject = "OutputControl:ReportingTolerances";
		Num = GetNumObjectsFound( CurrentModuleObject );
		if ( Num > 0 ) {
			GetObjectItem( CurrentModuleObject, 1, Alphas, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
			if ( ! lNumericFieldBlanks( 1 ) ) {
				deviationFromSetPtThresholdHtg = -Number( 1 );
			} else {
				deviationFromSetPtThresholdHtg = -.2;
			}
			if ( ! lNumericFieldBlanks( 2 ) ) {
				deviationFromSetPtThresholdClg = Number( 2 );
			} else {
				deviationFromSetPtThresholdClg = .2;
			}
		}

		DoZoneSizing = false;
		DoSystemSizing = false;
		DoPlantSizing = false;
		DoDesDaySim = true;
		DoWeathSim = true;
		CurrentModuleObject = "SimulationControl";
		NumRunControl = GetNumObjectsFound( CurrentModuleObject );
		if ( NumRunControl > 0 ) {
			RunControlInInput = true;
			GetObjectItem( CurrentModuleObject, 1, Alphas, NumAlpha, Number, NumNumber, IOStat, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
			if ( Alphas( 1 ) == "YES" ) DoZoneSizing = true;
			if ( Alphas( 2 ) == "YES" ) DoSystemSizing = true;
			if ( Alphas( 3 ) == "YES" ) DoPlantSizing = true;
			if ( Alphas( 4 ) == "NO" ) DoDesDaySim = false;
			if ( Alphas( 5 ) == "NO" ) DoWeathSim = false;
		}
		if ( DDOnly ) {
			DoDesDaySim = true;
			DoWeathSim = false;
		}
		if ( FullAnnualRun ) {
			DoDesDaySim = false;
			DoWeathSim = true;
		}

		if ( ErrorsFound ) {
			ShowFatalError( "Errors found getting Project Input" );
		}

		gio::write( OutputFileInits, fmtA ) << "! <Version>, Version ID";
		gio::write( OutputFileInits, Format_721 ) << trim( VersionID );

		gio::write( OutputFileInits, fmtA ) << "! <Timesteps per Hour>, #TimeSteps, Minutes per TimeStep {minutes}";
		gio::write( OutputFileInits, Format_731 ) << NumOfTimeStepInHour << int( MinutesPerTimeStep );

		gio::write( OutputFileInits, fmtA ) << "! <System Convergence Limits>, Minimum System TimeStep {minutes}, Max HVAC Iterations, " " Minimum Plant Iterations, Maximum Plant Iterations";
		MinInt = MinTimeStepSys * 60.;
		gio::write( OutputFileInits, Format_733 ) << trim( RoundSigDigits( MinInt ) ) << trim( RoundSigDigits( MaxIter ) ) << trim( RoundSigDigits( MinPlantSubIterations ) ) << trim( RoundSigDigits( MaxPlantSubIterations ) );

		if ( DoZoneSizing ) {
			Alphas( 1 ) = "Yes";
		} else {
			Alphas( 1 ) = "No";
		}
		if ( DoSystemSizing ) {
			Alphas( 2 ) = "Yes";
		} else {
			Alphas( 2 ) = "No";
		}
		if ( DoPlantSizing ) {
			Alphas( 3 ) = "Yes";
		} else {
			Alphas( 3 ) = "No";
		}
		if ( DoDesDaySim ) {
			Alphas( 4 ) = "Yes";
		} else {
			Alphas( 4 ) = "No";
		}
		if ( DoWeathSim ) {
			Alphas( 5 ) = "Yes";
		} else {
			Alphas( 5 ) = "No";
		}

		gio::write( OutputFileInits, fmtA ) << "! <Simulation Control>, Do Zone Sizing, Do System Sizing, " "Do Plant Sizing, Do Design Days, Do Weather Simulation";
		gio::write( OutputFileInits, "(' Simulation Control',$)" );
		for ( Num = 1; Num <= 5; ++Num ) {
			gio::write( OutputFileInits, "(', ',A,$)" ) << trim( Alphas( Num ) );
		}
		gio::write( OutputFileInits );

		gio::write( OutputFileInits, fmtA ) << "! <Output Reporting Tolerances>, Tolerance for Time Heating Setpoint Not Met, " "Tolerance for Zone Cooling Setpoint Not Met Time";
		gio::write( OutputFileInits, Format_751 ) << trim( RoundSigDigits( std::abs( deviationFromSetPtThresholdHtg ), 3 ) ) << trim( RoundSigDigits( deviationFromSetPtThresholdClg, 3 ) );

		//  IF (DisplayExtraWarnings) THEN
		//    Write(OutputFileInits,740)
		//    Write(OutputFileInits,741) (TRIM(Alphas(Num)),Num=1,5)
		//742 Format('! <Display Extra Warnings>, Display Advanced Report Variables, Do Not Mirror Detached Shading')
		//    IF (DisplayAdvancedReportVariables) THEN
		//      NumOut1='Yes'
		//    ELSE
		//      NumOut2='No'
		//    ENDIF
		//    IF (.not. MakeMirroredDetachedShading) THEN
		//      NumOut1='Yes'
		//    ELSE
		//      NumOut2='No'
		//    ENDIF
		//unused0909743 Format(' Display Extra Warnings',2(', ',A))
		//  ENDIF

	}

	void
	CheckForMisMatchedEnvironmentSpecifications()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   August 2008
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// In response to CR 7518, this routine will check to see if a proper combination of SimulationControl, RunPeriod,
		// SizingPeriod:*, etc are entered to proceed with a simulation.

		// METHODOLOGY EMPLOYED:
		// For now (8/2008), the routine will query several objects in the input.  And try to produce warnings or
		// fatals as a result.

		// REFERENCES:
		// na

		// Using/Aliasing
		using InputProcessor::GetNumObjectsFound;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		int NumZoneSizing;
		int NumSystemSizing;
		int NumPlantSizing;
		int NumDesignDays;
		int NumRunPeriodDesign;
		int NumSizingDays;
		bool WeatherFileAttached;
		bool ErrorsFound;

		ErrorsFound = false;
		NumZoneSizing = GetNumObjectsFound( "Sizing:Zone" );
		NumSystemSizing = GetNumObjectsFound( "Sizing:System" );
		NumPlantSizing = GetNumObjectsFound( "Sizing:Plant" );
		NumDesignDays = GetNumObjectsFound( "SizingPeriod:DesignDay" );
		NumRunPeriodDesign = GetNumObjectsFound( "SizingPeriod:WeatherFileDays" ) + GetNumObjectsFound( "SizingPeriod:WeatherFileConditionType" );
		NumSizingDays = NumDesignDays + NumRunPeriodDesign;
		{ IOFlags flags; gio::inquire( "in.epw", flags ); WeatherFileAttached = flags.exists(); }

		if ( RunControlInInput ) {
			if ( DoZoneSizing ) {
				if ( NumZoneSizing > 0 && NumSizingDays == 0 ) {
					ErrorsFound = true;
					ShowSevereError( "CheckEnvironmentSpecifications: Sizing for Zones has been requested but there are no " "design environments specified." );
					ShowContinueError( "...Add appropriate SizingPeriod:* objects for your simulation." );
				}
				if ( NumZoneSizing > 0 && NumRunPeriodDesign > 0 && ! WeatherFileAttached ) {
					ErrorsFound = true;
					ShowSevereError( "CheckEnvironmentSpecifications: Sizing for Zones has been requested; Design period from " "the weather file requested; but no weather file specified." );
				}
			}
			if ( DoSystemSizing ) {
				if ( NumSystemSizing > 0 && NumSizingDays == 0 ) {
					ErrorsFound = true;
					ShowSevereError( "CheckEnvironmentSpecifications: Sizing for Systems has been requested but there are no " "design environments specified." );
					ShowContinueError( "...Add appropriate SizingPeriod:* objects for your simulation." );
				}
				if ( NumSystemSizing > 0 && NumRunPeriodDesign > 0 && ! WeatherFileAttached ) {
					ErrorsFound = true;
					ShowSevereError( "CheckEnvironmentSpecifications: Sizing for Systems has been requested; Design period from " "the weather file requested; but no weather file specified." );
				}
			}
			if ( DoPlantSizing ) {
				if ( NumPlantSizing > 0 && NumSizingDays == 0 ) {
					ErrorsFound = true;
					ShowSevereError( "CheckEnvironmentSpecifications: Sizing for Equipment/Plants has been requested but there are no " "design environments specified." );
					ShowContinueError( "...Add appropriate SizingPeriod:* objects for your simulation." );
				}
				if ( NumPlantSizing > 0 && NumRunPeriodDesign > 0 && ! WeatherFileAttached ) {
					ErrorsFound = true;
					ShowSevereError( "CheckEnvironmentSpecifications: Sizing for Equipment/Plants has been requested; " "Design period from the weather file requested; but no weather file specified." );
				}
			}
			if ( DoDesDaySim && NumSizingDays == 0 ) {
				ShowWarningError( "CheckEnvironmentSpecifications: SimulationControl specified doing design day simulations, but " "no design environments specified." );
				ShowContinueError( "...No design environment results produced. For these results, " "add appropriate SizingPeriod:* objects for your simulation." );
			}
			if ( DoDesDaySim && NumRunPeriodDesign > 0 && ! WeatherFileAttached ) {
				ErrorsFound = true;
				ShowSevereError( "CheckEnvironmentSpecifications: SimulationControl specified doing design day simulations; weather " "file design environments specified; but no weather file specified." );
			}
			if ( DoWeathSim && ! RunPeriodsInInput ) {
				ShowWarningError( "CheckEnvironmentSpecifications: SimulationControl specified doing weather simulations, but " "no run periods for weather file specified.  No annual results produced." );
			}
			if ( DoWeathSim && RunPeriodsInInput && ! WeatherFileAttached ) {
				ShowWarningError( "CheckEnvironmentSpecifications: SimulationControl specified doing weather simulations; " "run periods for weather file specified; but no weather file specified." );
			}
		}
		if ( ! DoDesDaySim && ! DoWeathSim ) {
			ShowWarningError( "\"Do the design day simulations\" and \"Do the weather file simulation\"" " are both set to \"No\".  No simulations will be performed, and most input will not be read." );
		}
		if ( ! DoZoneSizing && ! DoSystemSizing && ! DoPlantSizing && ! DoDesDaySim && ! DoWeathSim ) {
			ShowSevereError( "All elements of SimulationControl are set to \"No\". No simulations can be done.  Program terminates." );
			ErrorsFound = true;
		}

		if ( ErrorsFound ) {
			ShowFatalError( "Program terminates due to preceding conditions." );
		}

	}

	void
	CheckForRequestedReporting()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   January 2009
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// EnergyPlus does not automatically produce any results files.  Because of this, users may not request
		// reports and may get confused when nothing is produced.  This routine will provide a warning when
		// results should be produced (either sizing periods or weather files are run) but no reports are
		// requested.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using InputProcessor::GetNumObjectsFound;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		bool SimPeriods;
		bool ReportingRequested;

		ReportingRequested = false;
		SimPeriods = ( GetNumObjectsFound( "SizingPeriod:DesignDay" ) > 0 || GetNumObjectsFound( "SizingPeriod:WeatherFileDays" ) > 0 || GetNumObjectsFound( "SizingPeriod:WeatherFileConditionType" ) > 0 || GetNumObjectsFound( "RunPeriod" ) > 0 );

		if ( ( DoDesDaySim || DoWeathSim ) && SimPeriods ) {
			ReportingRequested = ( GetNumObjectsFound( "Output:Table:SummaryReports" ) > 0 || GetNumObjectsFound( "Output:Table:TimeBins" ) > 0 || GetNumObjectsFound( "Output:Table:Monthly" ) > 0 || GetNumObjectsFound( "Output:Variable" ) > 0 || GetNumObjectsFound( "Output:Meter" ) > 0 || GetNumObjectsFound( "Output:Meter:MeterFileOnly" ) > 0 || GetNumObjectsFound( "Output:Meter:Cumulative" ) > 0 || GetNumObjectsFound( "Output:Meter:Cumulative:MeterFileOnly" ) > 0 );
			// Not testing for : Output:SQLite or Output:EnvironmentalImpactFactors
			if ( ! ReportingRequested ) {
				ShowWarningError( "No reporting elements have been requested. No simulation results produced." );
				ShowContinueError( "...Review requirements such as \"Output:Table:SummaryReports\", \"Output:Table:Monthly\", " "\"Output:Variable\", \"Output:Meter\" and others." );
			}
		}

	}

	void
	OpenOutputFiles()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Rick Strand
		//       DATE WRITTEN   June 1997
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// This subroutine opens all of the input and output files needed for
		// an EnergyPlus run.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using DataStringGlobals::VerString;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		int write_stat;

		// FLOW:
		OutputFileStandard = GetNewUnitNumber();
		StdOutputRecordCount = 0;
		{ IOFlags flags; flags.ACTION( "write" ); flags.STATUS( "UNKNOWN" ); gio::open( OutputFileStandard, "eplusout.eso", flags ); write_stat = flags.ios(); }
		if ( write_stat != 0 ) {
			ShowFatalError( "OpenOutputFiles: Could not open file \"eplusout.eso\" for output (write)." );
		}
		gio::write( OutputFileStandard, "(A)" ) << "Program Version," + trim( VerString );

		// Open the Initialization Output File
		OutputFileInits = GetNewUnitNumber();
		{ IOFlags flags; flags.ACTION( "write" ); flags.STATUS( "UNKNOWN" ); gio::open( OutputFileInits, "eplusout.eio", flags ); write_stat = flags.ios(); }
		if ( write_stat != 0 ) {
			ShowFatalError( "OpenOutputFiles: Could not open file \"eplusout.eio\" for output (write)." );
		}
		gio::write( OutputFileInits, "(A)" ) << "Program Version," + trim( VerString );

		// Open the Meters Output File
		OutputFileMeters = GetNewUnitNumber();
		StdMeterRecordCount = 0;
		{ IOFlags flags; flags.ACTION( "write" ); flags.STATUS( "UNKNOWN" ); gio::open( OutputFileMeters, "eplusout.mtr", flags ); write_stat = flags.ios(); }
		if ( write_stat != 0 ) {
			ShowFatalError( "OpenOutputFiles: Could not open file \"eplusout.mtr\" for output (write)." );
		}
		gio::write( OutputFileMeters, "(A)" ) << "Program Version," + trim( VerString );

		// Open the Branch-Node Details Output File
		OutputFileBNDetails = GetNewUnitNumber();
		{ IOFlags flags; flags.ACTION( "write" ); flags.STATUS( "UNKNOWN" ); gio::open( OutputFileBNDetails, "eplusout.bnd", flags ); write_stat = flags.ios(); }
		if ( write_stat != 0 ) {
			ShowFatalError( "OpenOutputFiles: Could not open file \"eplusout.bnd\" for output (write)." );
		}
		gio::write( OutputFileBNDetails, "(A)" ) << "Program Version," + trim( VerString );

	}

	void
	CloseOutputFiles()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Rick Strand
		//       DATE WRITTEN   June 1997
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// This subroutine closes all of the input and output files needed for
		// an EnergyPlus run.  It also prints the end of data marker for each
		// output file.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using namespace DataOutputs;
		using OutputProcessor::MaxRVariable;
		using OutputProcessor::NumOfRVariable_Setup;
		using OutputProcessor::NumOfRVariable_Sum;
		using OutputProcessor::NumOfRVariable_Meter;
		using OutputProcessor::MaxIVariable;
		using OutputProcessor::NumOfIVariable_Setup;
		using OutputProcessor::NumOfIVariable_Sum;
		using OutputProcessor::NumOfRVariable;
		using OutputProcessor::NumOfIVariable;
		using OutputProcessor::NumEnergyMeters;
		using OutputProcessor::NumVarMeterArrays;
		using OutputProcessor::NumTotalRVariable;
		using OutputProcessor::NumTotalIVariable;
		using OutputProcessor::NumReportList;
		using OutputProcessor::InstMeterCacheSize;
		using OutputReportTabular::maxUniqueKeyCount;
		using OutputReportTabular::MonthlyFieldSetInputCount;
		using SolarShading::maxNumberOfFigures;
		using SolarShading::MAXHCArrayBounds;
		using namespace DataRuntimeLanguage;
		using DataBranchNodeConnections::NumOfNodeConnections;
		using DataBranchNodeConnections::MaxNumOfNodeConnections;
		using DataHeatBalance::CondFDRelaxFactor;
		using DataHeatBalance::HeatTransferAlgosUsed;
		using DataHeatBalance::UseCondFD;
		using DataHeatBalance::CondFDRelaxFactorInput;
		using General::RoundSigDigits;
		using namespace DataSystemVariables; // , ONLY: MaxNumberOfThreads,NumberIntRadThreads,iEnvSetThreads
		using DataSurfaces::MaxVerticesPerSurface;
		using namespace DataTimings;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		static Fstring const EndOfDataFormat( "(\"End of Data\")" ); // Signifies the end of the data block in the output file
		static Fstring const ThreadingHeader( "! <Program Control Information:Threads/Parallel Sims>, " "Threading Supported,Maximum Number of Threads, Env Set Threads (OMP_NUM_THREADS), " "EP Env Set Threads (EP_OMP_NUM_THREADS), IDF Set Threads, Number of Threads Used (Interior Radiant Exchange), " "Number Nominal Surfaces, Number Parallel Sims" );
		static Fstring const fmtA( "(A)" );

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		int EchoInputFile; // found unit number for 'eplusout.audit'
		Fstring cEnvSetThreads( 10 );
		Fstring cepEnvSetThreads( 10 );
		Fstring cIDFSetThreads( 10 );

		EchoInputFile = FindUnitNumber( "eplusout.audit" );
		// Record some items on the audit file
		gio::write( EchoInputFile, "*" ) << "NumOfRVariable=" << NumOfRVariable_Setup;
		gio::write( EchoInputFile, "*" ) << "NumOfRVariable(Total)=" << NumTotalRVariable;
		gio::write( EchoInputFile, "*" ) << "NumOfRVariable(Actual)=" << NumOfRVariable;
		gio::write( EchoInputFile, "*" ) << "NumOfRVariable(Summed)=" << NumOfRVariable_Sum;
		gio::write( EchoInputFile, "*" ) << "NumOfRVariable(Meter)=" << NumOfRVariable_Meter;
		gio::write( EchoInputFile, "*" ) << "NumOfIVariable=" << NumOfIVariable_Setup;
		gio::write( EchoInputFile, "*" ) << "NumOfIVariable(Total)=" << NumTotalIVariable;
		gio::write( EchoInputFile, "*" ) << "NumOfIVariable(Actual)=" << NumOfIVariable;
		gio::write( EchoInputFile, "*" ) << "NumOfIVariable(Summed)=" << NumOfIVariable_Sum;
		gio::write( EchoInputFile, "*" ) << "MaxRVariable=" << MaxRVariable;
		gio::write( EchoInputFile, "*" ) << "MaxIVariable=" << MaxIVariable;
		gio::write( EchoInputFile, "*" ) << "NumEnergyMeters=" << NumEnergyMeters;
		gio::write( EchoInputFile, "*" ) << "NumVarMeterArrays=" << NumVarMeterArrays;
		gio::write( EchoInputFile, "*" ) << "maxUniqueKeyCount=" << maxUniqueKeyCount;
		gio::write( EchoInputFile, "*" ) << "maxNumberOfFigures=" << maxNumberOfFigures;
		gio::write( EchoInputFile, "*" ) << "MAXHCArrayBounds=" << MAXHCArrayBounds;
		gio::write( EchoInputFile, "*" ) << "MaxVerticesPerSurface=" << MaxVerticesPerSurface;
		gio::write( EchoInputFile, "*" ) << "NumReportList=" << NumReportList;
		gio::write( EchoInputFile, "*" ) << "InstMeterCacheSize=" << InstMeterCacheSize;
		if ( SutherlandHodgman ) {
			gio::write( EchoInputFile, "*" ) << "ClippingAlgorithm=SutherlandHodgman";
		} else {
			gio::write( EchoInputFile, "*" ) << "ClippingAlgorithm=ConvexWeilerAtherton";
		}
		gio::write( EchoInputFile, "*" ) << "MonthlyFieldSetInputCount=" << MonthlyFieldSetInputCount;
		gio::write( EchoInputFile, "*" ) << "NumConsideredOutputVariables=" << NumConsideredOutputVariables;
		gio::write( EchoInputFile, "*" ) << "MaxConsideredOutputVariables=" << MaxConsideredOutputVariables;

		gio::write( EchoInputFile, "*" ) << "numActuatorsUsed=" << numActuatorsUsed;
		gio::write( EchoInputFile, "*" ) << "numEMSActuatorsAvailable=" << numEMSActuatorsAvailable;
		gio::write( EchoInputFile, "*" ) << "maxEMSActuatorsAvailable=" << maxEMSActuatorsAvailable;
		gio::write( EchoInputFile, "*" ) << "numInternalVariablesUsed=" << NumInternalVariablesUsed;
		gio::write( EchoInputFile, "*" ) << "numEMSInternalVarsAvailable=" << numEMSInternalVarsAvailable;
		gio::write( EchoInputFile, "*" ) << "maxEMSInternalVarsAvailable=" << maxEMSInternalVarsAvailable;

		gio::write( EchoInputFile, "*" ) << "NumOfNodeConnections=" << NumOfNodeConnections;
		gio::write( EchoInputFile, "*" ) << "MaxNumOfNodeConnections=" << MaxNumOfNodeConnections;
#ifdef EP_Count_Calls
		gio::write( EchoInputFile, "*" ) << "NumShadow_Calls=" << NumShadow_Calls;
		gio::write( EchoInputFile, "*" ) << "NumShadowAtTS_Calls=" << NumShadowAtTS_Calls;
		gio::write( EchoInputFile, "*" ) << "NumClipPoly_Calls=" << NumClipPoly_Calls;
		gio::write( EchoInputFile, "*" ) << "NumInitSolar_Calls=" << NumInitSolar_Calls;
		gio::write( EchoInputFile, "*" ) << "NumAnisoSky_Calls=" << NumAnisoSky_Calls;
		gio::write( EchoInputFile, "*" ) << "NumDetPolyOverlap_Calls=" << NumDetPolyOverlap_Calls;
		gio::write( EchoInputFile, "*" ) << "NumCalcPerSolBeam_Calls=" << NumCalcPerSolBeam_Calls;
		gio::write( EchoInputFile, "*" ) << "NumDetShadowCombs_Calls=" << NumDetShadowCombs_Calls;
		gio::write( EchoInputFile, "*" ) << "NumIntSolarDist_Calls=" << NumIntSolarDist_Calls;
		gio::write( EchoInputFile, "*" ) << "NumIntRadExchange_Calls=" << NumIntRadExchange_Calls;
		gio::write( EchoInputFile, "*" ) << "NumIntRadExchangeZ_Calls=" << NumIntRadExchangeZ_Calls;
		gio::write( EchoInputFile, "*" ) << "NumIntRadExchangeMain_Calls=" << NumIntRadExchangeMain_Calls;
		gio::write( EchoInputFile, "*" ) << "NumIntRadExchangeOSurf_Calls=" << NumIntRadExchangeOSurf_Calls;
		gio::write( EchoInputFile, "*" ) << "NumIntRadExchangeISurf_Calls=" << NumIntRadExchangeISurf_Calls;
		gio::write( EchoInputFile, "*" ) << "NumMaxInsideSurfIterations=" << NumMaxInsideSurfIterations;
		gio::write( EchoInputFile, "*" ) << "NumCalcScriptF_Calls=" << NumCalcScriptF_Calls;
#endif

		gio::write( OutputFileStandard, EndOfDataFormat );
		gio::write( OutputFileStandard, "*" ) << "Number of Records Written=" << StdOutputRecordCount;
		if ( StdOutputRecordCount > 0 ) {
			gio::close( OutputFileStandard );
		} else {
			{ IOFlags flags; flags.DISPOSE( "DELETE" ); gio::close( OutputFileStandard, flags ); }
		}

		if ( any_eq( HeatTransferAlgosUsed, UseCondFD ) ) { // echo out relaxation factor, it may have been changed by the program
			gio::write( OutputFileInits, "(A)" ) << "! <ConductionFiniteDifference Numerical Parameters>, " "Starting Relaxation Factor, Final Relaxation Factor";
			gio::write( OutputFileInits, "(A)" ) << "ConductionFiniteDifference Numerical Parameters, " + trim( RoundSigDigits( CondFDRelaxFactorInput, 3 ) ) + ", " + trim( RoundSigDigits( CondFDRelaxFactor, 3 ) );
		}
		// Report number of threads to eio file
		if ( Threading ) {
			if ( iEnvSetThreads == 0 ) {
				cEnvSetThreads = "Not Set";
			} else {
				cEnvSetThreads = RoundSigDigits( iEnvSetThreads );
			}
			if ( iepEnvSetThreads == 0 ) {
				cepEnvSetThreads = "Not Set";
			} else {
				cepEnvSetThreads = RoundSigDigits( iepEnvSetThreads );
			}
			if ( iIDFSetThreads == 0 ) {
				cIDFSetThreads = "Not Set";
			} else {
				cIDFSetThreads = RoundSigDigits( iIDFSetThreads );
			}
			if ( lnumActiveSims ) {
				gio::write( OutputFileInits, fmtA ) << ThreadingHeader;
				gio::write( OutputFileInits, "(A)" ) << "Program Control:Threads/Parallel Sims, Yes," + trim( RoundSigDigits( MaxNumberOfThreads ) ) + ", " + trim( cEnvSetThreads ) + ", " + trim( cepEnvSetThreads ) + ", " + trim( cIDFSetThreads ) + ", " + trim( RoundSigDigits( NumberIntRadThreads ) ) + ", " + trim( RoundSigDigits( iNominalTotSurfaces ) ) + ", " + trim( RoundSigDigits( inumActiveSims ) );
			} else {
				gio::write( OutputFileInits, fmtA ) << ThreadingHeader;
				gio::write( OutputFileInits, "(A)" ) << "Program Control:Threads/Parallel Sims, Yes," + trim( RoundSigDigits( MaxNumberOfThreads ) ) + ", " + trim( cEnvSetThreads ) + ", " + trim( cepEnvSetThreads ) + ", " + trim( cIDFSetThreads ) + ", " + trim( RoundSigDigits( NumberIntRadThreads ) ) + ", " + trim( RoundSigDigits( iNominalTotSurfaces ) ) + ", " "N/A";
			}
		} else { // no threading
			if ( lnumActiveSims ) {
				gio::write( OutputFileInits, fmtA ) << ThreadingHeader;
				gio::write( OutputFileInits, "(A)" ) << "Program Control:Threads/Parallel Sims, No," + trim( RoundSigDigits( MaxNumberOfThreads ) ) + ", " "N/A, N/A, N/A, N/A, N/A, " + trim( RoundSigDigits( inumActiveSims ) );
			} else {
				gio::write( OutputFileInits, fmtA ) << ThreadingHeader;
				gio::write( OutputFileInits, "(A)" ) << "Program Control:Threads/Parallel Sims, No," + trim( RoundSigDigits( MaxNumberOfThreads ) ) + ", " "N/A, N/A, N/A, N/A, N/A, N/A";
			}
		}

		// Close the Initialization Output File
		gio::write( OutputFileInits, EndOfDataFormat );
		gio::close( OutputFileInits );

		// Close the Meters Output File
		gio::write( OutputFileMeters, EndOfDataFormat );
		gio::write( OutputFileMeters, "*" ) << "Number of Records Written=" << StdMeterRecordCount;
		if ( StdMeterRecordCount > 0 ) {
			gio::close( OutputFileMeters );
		} else {
			{ IOFlags flags; flags.DISPOSE( "DELETE" ); gio::close( OutputFileMeters, flags ); }
		}

	}

	void
	SetupSimulation( bool & ErrorsFound )
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         B. Griffith/L. Lawrie
		//       DATE WRITTEN   May 2008
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		//  execute a few time steps of a simulation to facilitate setting up model
		//  developed to resolve reverse DD problems caused be the differences
		//  that stem from setup and information gathering that occurs during the first pass.

		// METHODOLOGY EMPLOYED:
		// Using global flag (kickoff simulation), only a few time steps are executed.
		// global flag is used in other parts of simulation to terminate quickly.

		// REFERENCES:
		// na

		// Using/Aliasing
		using ExteriorEnergyUse::ManageExteriorEnergyUse;
		using DataEnvironment::EndMonthFlag;
		using DataEnvironment::EnvironmentName;
		using InputProcessor::GetNumRangeCheckErrorsFound;
		using CostEstimateManager::SimCostEstimate;
		using General::TrimSigDigits;
		using namespace DataTimings;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:

		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		static bool Available( false ); // an environment is available to process
		//  integer :: env_iteration=0
		//  CHARACTER(len=32) :: cEnvChar

		//  return  ! remove comment to do 'old way'

		Available = true;

		while ( Available ) { // do for each environment

			GetNextEnvironment( Available, ErrorsFound );

			if ( ! Available ) break;
			if ( ErrorsFound ) break;

			BeginEnvrnFlag = true;
			EndEnvrnFlag = false;
			EndMonthFlag = false;
			WarmupFlag = true;
			DayOfSim = 0;

			++DayOfSim;
			BeginDayFlag = true;
			EndDayFlag = false;

			HourOfDay = 1;

			BeginHourFlag = true;
			EndHourFlag = false;

			TimeStep = 1;

			if ( DeveloperFlag ) DisplayString( "Initializing Simulation - timestep 1:" + trim( EnvironmentName ) );

			BeginTimeStepFlag = true;

			ManageWeather();

			ManageExteriorEnergyUse();

			ManageHeatBalance();

			//  After the first iteration of HeatBalance, all the 'input' has been gotten
			if ( BeginFullSimFlag ) {
				if ( GetNumRangeCheckErrorsFound() > 0 ) {
					ShowFatalError( "Out of \"range\" values found in input" );
				}
			}

			BeginHourFlag = false;
			BeginDayFlag = false;
			BeginEnvrnFlag = false;
			BeginSimFlag = false;
			BeginFullSimFlag = false;

			//          ! do another timestep=1
			if ( DeveloperFlag ) DisplayString( "Initializing Simulation - 2nd timestep 1:" + trim( EnvironmentName ) );

			ManageWeather();

			ManageExteriorEnergyUse();

			ManageHeatBalance();

			//         do an end of day, end of environment time step

			HourOfDay = 24;
			TimeStep = NumOfTimeStepInHour;
			EndEnvrnFlag = true;

			if ( DeveloperFlag ) DisplayString( "Initializing Simulation - hour 24 timestep 1:" + trim( EnvironmentName ) );
			ManageWeather();

			ManageExteriorEnergyUse();

			ManageHeatBalance();

		} // ... End environment loop.

		if ( ! ErrorsFound ) SimCostEstimate(); // basically will get and check input
		if ( ErrorsFound ) ShowFatalError( "Previous Conditions cause program termination." );

	}

	void
	ReportNodeConnections()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   February 2004
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// This subroutine 'reports' the NodeConnection data structure.  It groups the
		// report/dump by parent, non-parent objects.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using InputProcessor::SameString;
		using InputProcessor::MakeUPPERCase;
		using namespace DataBranchNodeConnections;
		using DataGlobals::OutputFileBNDetails;
		using DataLoopNode::NumOfNodes;
		using DataLoopNode::NodeID;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS
		// na

		// DERIVED TYPE DEFINITIONS
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		int Loop;
		int Loop1;
		int NumParents;
		int NumNonParents;
		int NumNonConnected;
		Fstring ChrOut( 20 );
		FArray1D_bool NonConnectedNodes;
		bool ParentComponentFound;

		// Formats
		std::string const Format_701( "(A)" );
		std::string const Format_702( "('! <#',A,' Node Connections>,<Number of ',A,' Node Connections>')" );
		std::string const Format_703( "('! <',A,' Node Connection>,<Node Name>,<Node ObjectType>,<Node ObjectName>,','<Node ConnectionType>,<Node FluidStream>')" );
		std::string const Format_705( "('! <#NonConnected Nodes>,<Number of NonConnected Nodes>',/,' #NonConnected Nodes,',A)" );
		std::string const Format_706( "('! <NonConnected Node>,<NonConnected Node Number>,<NonConnected Node Name>')" );

		NonConnectedNodes.allocate( NumOfNodes );
		NonConnectedNodes = true;

		NumParents = 0;
		NumNonParents = 0;
		for ( Loop = 1; Loop <= NumOfNodeConnections; ++Loop ) {
			if ( NodeConnections( Loop ).ObjectIsParent ) continue;
			++NumNonParents;
		}
		NumParents = NumOfNodeConnections - NumNonParents;
		ParentNodeList.allocate( NumParents );

		//  Do Parent Objects
		gio::write( OutputFileBNDetails, Format_701 ) << "! ===============================================================";
		gio::write( OutputFileBNDetails, Format_702 ) << "Parent" << "Parent";
		gio::write( ChrOut, "*" ) << NumParents;
		gio::write( OutputFileBNDetails, Format_701 ) << " #Parent Node Connections," + trim( adjustl( ChrOut ) );
		gio::write( OutputFileBNDetails, Format_703 ) << "Parent";

		for ( Loop = 1; Loop <= NumOfNodeConnections; ++Loop ) {
			if ( ! NodeConnections( Loop ).ObjectIsParent ) continue;
			NonConnectedNodes( NodeConnections( Loop ).NodeNumber ) = false;
			gio::write( ChrOut, "*" ) << NodeConnections( Loop ).FluidStream;
			ChrOut = adjustl( ChrOut );
			gio::write( OutputFileBNDetails, Format_701 ) << " Parent Node Connection," + trim( NodeConnections( Loop ).NodeName ) + "," + trim( NodeConnections( Loop ).ObjectType ) + "," + trim( NodeConnections( Loop ).ObjectName ) + "," + trim( NodeConnections( Loop ).ConnectionType ) + "," + trim( ChrOut );
			// Build ParentNodeLists
			if ( SameString( NodeConnections( Loop ).ConnectionType, "Inlet" ) || SameString( NodeConnections( Loop ).ConnectionType, "Outlet" ) ) {
				ParentComponentFound = false;
				for ( Loop1 = 1; Loop1 <= NumOfActualParents; ++Loop1 ) {
					if ( ParentNodeList( Loop1 ).CType != NodeConnections( Loop ).ObjectType || ParentNodeList( Loop1 ).CName != NodeConnections( Loop ).ObjectName ) continue;
					ParentComponentFound = true;
					{ auto const SELECT_CASE_var( MakeUPPERCase( NodeConnections( Loop ).ConnectionType ) );
					if ( SELECT_CASE_var == "INLET" ) {
						ParentNodeList( Loop1 ).InletNodeName = NodeConnections( Loop ).NodeName;
					} else if ( SELECT_CASE_var == "OUTLET" ) {
						ParentNodeList( Loop1 ).OutletNodeName = NodeConnections( Loop ).NodeName;
					}}
				}
				if ( ! ParentComponentFound ) {
					++NumOfActualParents;
					ParentNodeList( NumOfActualParents ).CType = NodeConnections( Loop ).ObjectType;
					ParentNodeList( NumOfActualParents ).CName = NodeConnections( Loop ).ObjectName;
					{ auto const SELECT_CASE_var( MakeUPPERCase( NodeConnections( Loop ).ConnectionType ) );
					if ( SELECT_CASE_var == "INLET" ) {
						ParentNodeList( NumOfActualParents ).InletNodeName = NodeConnections( Loop ).NodeName;
					} else if ( SELECT_CASE_var == "OUTLET" ) {
						ParentNodeList( NumOfActualParents ).OutletNodeName = NodeConnections( Loop ).NodeName;
					}}
				}
			}
		}

		//  Do non-Parent Objects
		gio::write( OutputFileBNDetails, Format_701 ) << "! ===============================================================";
		gio::write( OutputFileBNDetails, Format_702 ) << "Non-Parent" << "Non-Parent";
		gio::write( ChrOut, "*" ) << NumNonParents;
		gio::write( OutputFileBNDetails, Format_701 ) << " #Non-Parent Node Connections," + trim( adjustl( ChrOut ) );
		gio::write( OutputFileBNDetails, Format_703 ) << "Non-Parent";

		for ( Loop = 1; Loop <= NumOfNodeConnections; ++Loop ) {
			if ( NodeConnections( Loop ).ObjectIsParent ) continue;
			NonConnectedNodes( NodeConnections( Loop ).NodeNumber ) = false;
			gio::write( ChrOut, "*" ) << NodeConnections( Loop ).FluidStream;
			ChrOut = adjustl( ChrOut );
			gio::write( OutputFileBNDetails, Format_701 ) << " Non-Parent Node Connection," + trim( NodeConnections( Loop ).NodeName ) + "," + trim( NodeConnections( Loop ).ObjectType ) + "," + trim( NodeConnections( Loop ).ObjectName ) + "," + trim( NodeConnections( Loop ).ConnectionType ) + "," + trim( ChrOut );
		}

		NumNonConnected = 0;
		for ( Loop = 1; Loop <= NumOfNodes; ++Loop ) {
			if ( NonConnectedNodes( Loop ) ) ++NumNonConnected;
		}

		if ( NumNonConnected > 0 ) {
			gio::write( OutputFileBNDetails, Format_701 ) << "! ===============================================================";
			gio::write( ChrOut, "*" ) << NumNonConnected;
			gio::write( OutputFileBNDetails, Format_705 ) << trim( adjustl( ChrOut ) );
			gio::write( OutputFileBNDetails, Format_706 );
			for ( Loop = 1; Loop <= NumOfNodes; ++Loop ) {
				if ( ! NonConnectedNodes( Loop ) ) continue;
				gio::write( ChrOut, "*" ) << Loop;
				ChrOut = adjustl( ChrOut );
				gio::write( OutputFileBNDetails, Format_701 ) << " NonConnected Node," + trim( ChrOut ) + "," + trim( NodeID( Loop ) );
			}
		}

		NonConnectedNodes.deallocate();

	}

	void
	ReportLoopConnections()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   December 2001
		//       MODIFIED       March 2003; added other reporting
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// This subroutine reports on the node connections in various parts of the
		// HVAC syste: Component Sets, Air Loop, Plant and Condenser Loop, Supply and
		// return air paths, controlled zones.
		// This information should be useful in diagnosing node connection input errors.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using InputProcessor::SameString;
		using namespace DataAirLoop;
		using namespace DataBranchNodeConnections;
		using DataLoopNode::NumOfNodes;
		using DataLoopNode::NodeID;
		using namespace DataHVACGlobals;
		using DataHeatBalance::Zone;
		using namespace DataPlant;
		using namespace DataZoneEquipment;
		using OutAirNodeManager::OutsideAirNodeList;
		using OutAirNodeManager::NumOutsideAirNodes;
		using DataErrorTracking::AbortProcessing; // used here to turn off Node Connection Error reporting
		using DataErrorTracking::AskForConnectionsReport;
		using DualDuct::ReportDualDuctConnections;
		using DataGlobals::OutputFileBNDetails;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		static Fstring const errstring( "**error**" );
		static Fstring const Blank;

		// INTERFACE BLOCK SPECIFICATIONS
		// na

		// DERIVED TYPE DEFINITIONS
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		Fstring ChrOut( 20 );
		Fstring ChrOut2( 20 );
		Fstring ChrOut3( 20 );
		Fstring LoopString( 6 );
		Fstring ChrName( MaxNameLength );
		int Count;
		int Count1;
		int LoopSideNum;
		int Num;
		static bool WarningOut( true );
		int NumOfControlledZones;

		// Formats
		std::string const Format_700( "('! <#Component Sets>,<Number of Component Sets>')" );
		std::string const Format_701( "(A)" );
		std::string const Format_702( "('! <Component Set>,<Component Set Count>,<Parent Object Type>,<Parent Object Name>,','<Component Type>,<Component Name>,<Inlet Node ID>,<Outlet Node ID>,<Description>')" );
		std::string const Format_707( "(1X,A)" );
		std::string const Format_713( "(A)" );
		std::string const Format_720( "('! <#Zone Equipment Lists>,<Number of Zone Equipment Lists>')" );
		std::string const Format_721( "(A)" );
		std::string const Format_722( "('! <Zone Equipment List>,<Zone Equipment List Count>,<Zone Equipment List Name>,<Zone Name>,<Number of Components>')" );
		std::string const Format_723( "('! <Zone Equipment Component>,<Component Count>,<Component Type>,<Component Name>,','<Zone Name>,<Heating Priority>,<Cooling Priority>')" );

		// Report outside air node names on the Branch-Node Details file
		gio::write( OutputFileBNDetails, Format_701 ) << "! ===============================================================";
		gio::write( OutputFileBNDetails, Format_701 ) << "! #Outdoor Air Nodes,<Number of Outdoor Air Nodes>";
		gio::write( ChrOut, "*" ) << NumOutsideAirNodes;
		gio::write( OutputFileBNDetails, Format_701 ) << " #Outdoor Air Nodes," + adjustl( ChrOut );
		if ( NumOutsideAirNodes > 0 ) {
			gio::write( OutputFileBNDetails, Format_701 ) << "! <Outdoor Air Node>,<NodeNumber>,<Node Name>";
		}
		for ( Count = 1; Count <= NumOutsideAirNodes; ++Count ) {
			gio::write( ChrOut, "*" ) << OutsideAirNodeList( Count );
			ChrOut = adjustl( ChrOut );
			gio::write( OutputFileBNDetails, Format_701 ) << " Outdoor Air Node," + trim( ChrOut ) + "," + trim( NodeID( OutsideAirNodeList( Count ) ) );
		}
		// Component Sets
		gio::write( OutputFileBNDetails, Format_701 ) << "! ===============================================================";
		gio::write( OutputFileBNDetails, Format_700 );
		gio::write( ChrOut, "*" ) << NumCompSets;
		gio::write( OutputFileBNDetails, Format_701 ) << " #Component Sets," + trim( adjustl( ChrOut ) );
		gio::write( OutputFileBNDetails, Format_702 );

		for ( Count = 1; Count <= NumCompSets; ++Count ) {
			gio::write( ChrOut, "*" ) << Count;
			ChrOut = adjustl( ChrOut );
			gio::write( OutputFileBNDetails, Format_701 ) << " Component Set," + trim( ChrOut ) + "," + trim( CompSets( Count ).ParentCType ) + "," + trim( CompSets( Count ).ParentCName ) + "," + trim( CompSets( Count ).CType ) + "," + trim( CompSets( Count ).CName ) + "," + trim( CompSets( Count ).InletNodeName ) + "," + trim( CompSets( Count ).OutletNodeName ) + "," + trim( CompSets( Count ).Description );

			if ( CompSets( Count ).ParentCType == "UNDEFINED" || CompSets( Count ).InletNodeName == "UNDEFINED" || CompSets( Count ).OutletNodeName == "UNDEFINED" ) {
				if ( AbortProcessing && WarningOut ) {
					ShowWarningError( "Node Connection errors shown during \"fatal error\" processing may be false " "because not all inputs may have been retrieved." );
					WarningOut = false;
				}
				ShowWarningError( "Node Connection Error for object " + trim( CompSets( Count ).CType ) + ", name=" + trim( CompSets( Count ).CName ) );
				ShowContinueError( "  " + trim( CompSets( Count ).Description ) + " not on any Branch or Parent Object" );
				ShowContinueError( "  Inlet Node : " + trim( CompSets( Count ).InletNodeName ) );
				ShowContinueError( "  Outlet Node: " + trim( CompSets( Count ).OutletNodeName ) );
				++NumNodeConnectionErrors;
				if ( SameString( CompSets( Count ).CType, "SolarCollector:UnglazedTranspired" ) ) {
					ShowContinueError( "This report does not necessarily indicate a problem for a MultiSystem Transpired Collector" );
				}
			}
			if ( CompSets( Count ).Description == "UNDEFINED" ) {
				if ( AbortProcessing && WarningOut ) {
					ShowWarningError( "Node Connection errors shown during \"fatal error\" processing may be false " "because not all inputs may have been retrieved." );
					WarningOut = false;
				}
				ShowWarningError( "Potential Node Connection Error for object " + trim( CompSets( Count ).CType ) + ", name=" + trim( CompSets( Count ).CName ) );
				ShowContinueError( "  Node Types are still UNDEFINED -- See Branch/Node Details file for further information" );
				ShowContinueError( "  Inlet Node : " + trim( CompSets( Count ).InletNodeName ) );
				ShowContinueError( "  Outlet Node: " + trim( CompSets( Count ).OutletNodeName ) );
				++NumNodeConnectionErrors;

			}
		}

		for ( Count = 1; Count <= NumCompSets; ++Count ) {
			for ( Count1 = Count + 1; Count1 <= NumCompSets; ++Count1 ) {
				if ( CompSets( Count ).CType != CompSets( Count1 ).CType ) continue;
				if ( CompSets( Count ).CName != CompSets( Count1 ).CName ) continue;
				if ( CompSets( Count ).InletNodeName != CompSets( Count1 ).InletNodeName ) continue;
				if ( CompSets( Count ).OutletNodeName != CompSets( Count1 ).OutletNodeName ) continue;
				if ( AbortProcessing && WarningOut ) {
					ShowWarningError( "Node Connection errors shown during \"fatal error\" processing may be false " "because not all inputs may have been retrieved." );
					WarningOut = false;
				}
				ShowWarningError( "Component plus inlet/outlet node pair used more than once:" );
				ShowContinueError( "  Component  : " + trim( CompSets( Count ).CType ) + ", name=" + trim( CompSets( Count ).CName ) );
				ShowContinueError( "  Inlet Node : " + trim( CompSets( Count ).InletNodeName ) );
				ShowContinueError( "  Outlet Node: " + trim( CompSets( Count ).OutletNodeName ) );
				ShowContinueError( "  Used by    : " + trim( CompSets( Count ).ParentCType ) + " " + trim( CompSets( Count ).ParentCName ) );
				ShowContinueError( "  and  by    : " + trim( CompSets( Count1 ).ParentCType ) + " " + trim( CompSets( Count1 ).ParentCName ) );
				++NumNodeConnectionErrors;
			}
		}
		//  Plant Loops
		gio::write( OutputFileBNDetails, Format_701 ) << "! ===============================================================";
		gio::write( ChrOut, "*" ) << NumPlantLoops;
		ChrOut = adjustl( ChrOut );
		gio::write( OutputFileBNDetails, Format_713 ) << "! <# Plant Loops>,<Number of Plant Loops>";
		gio::write( OutputFileBNDetails, Format_707 ) << "#Plant Loops," + trim( ChrOut );
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Plant Loop>,<Plant Loop Name>,<Loop Type>,<Inlet Node Name>," "<Outlet Node Name>,<Branch List>,<Connector List>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Plant Loop Connector>,<Connector Type>,<Connector Name>," "<Loop Name>,<Loop Type>,<Number of Inlets/Outlets>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Plant Loop Connector Branches>,<Connector Node Count>,<Connector Type>," "<Connector Name>,<Inlet Branch>,<Outlet Branch>," "<Loop Name>,<Loop Type>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Plant Loop Connector Nodes>,<Connector Node Count>,<Connector Type>," "<Connector Name>,<Inlet Node>,<Outlet Node>," "<Loop Name>,<Loop Type>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Plant Loop Supply Connection>,<Plant Loop Name>,<Supply Side Outlet Node Name>," "<Demand Side Inlet Node Name>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Plant Loop Return Connection>,<Plant Loop Name>,<Demand Side Outlet Node Name>," "<Supply Side Inlet Node Name>";
		for ( Count = 1; Count <= NumPlantLoops; ++Count ) {
			for ( LoopSideNum = DemandSide; LoopSideNum <= SupplySide; ++LoopSideNum ) {
				//  Plant Supply Side Loop
				// Demandside and supplyside is parametrized in DataPlant
				if ( LoopSideNum == DemandSide ) {
					LoopString = "Demand";
				} else if ( LoopSideNum == SupplySide ) {
					LoopString = "Supply";
				}

				gio::write( OutputFileBNDetails, Format_713 ) << " Plant Loop," + trim( PlantLoop( Count ).Name ) + "," + LoopString + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).NodeNameIn ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).NodeNameOut ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).BranchList ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).ConnectList );
				//  Plant Supply Side Splitter
				for ( Num = 1; Num <= PlantLoop( Count ).LoopSide( LoopSideNum ).NumSplitters; ++Num ) {
					if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).Exists ) {
						gio::write( ChrOut, "*" ) << PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).TotalOutletNodes;
						gio::write( OutputFileBNDetails, Format_713 ) << "   Plant Loop Connector,Splitter," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).Name ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString + "," + trim( adjustl( ChrOut ) );
						for ( Count1 = 1; Count1 <= PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).TotalOutletNodes; ++Count1 ) {
							gio::write( ChrOut, "*" ) << Count1;
							ChrOut2 = Blank;
							ChrOut3 = Blank;
							if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).BranchNumIn <= 0 ) {
								ChrOut2 = errstring;
							}
							if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).BranchNumOut( Count1 ) <= 0 ) {
								ChrOut3 = errstring;
							}
							{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << "     Plant Loop Connector Branches," + trim( adjustl( ChrOut ) ) + ",Splitter," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).Name ) + ","; }
							if ( ChrOut2 != errstring ) {
								{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Branch( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).BranchNumIn ).Name ) + ","; }
							} else {
								{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << trim( ChrOut2 ) + ","; }
							}
							if ( ChrOut3 != errstring ) {
								gio::write( OutputFileBNDetails, Format_713 ) << trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Branch( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).BranchNumOut( Count1 ) ).Name ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
							} else {
								gio::write( OutputFileBNDetails, Format_713 ) << trim( ChrOut3 ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
							}
							gio::write( OutputFileBNDetails, Format_713 ) << "     Plant Loop Connector Nodes,   " + trim( adjustl( ChrOut ) ) + ",Splitter," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).Name ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).NodeNameIn ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).NodeNameOut( Count1 ) ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
						}
					}
				}
				//  Plant Supply Side Mixer
				for ( Num = 1; Num <= PlantLoop( Count ).LoopSide( LoopSideNum ).NumMixers; ++Num ) {
					if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).Exists ) {
						gio::write( ChrOut, "*" ) << PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).TotalInletNodes;
						gio::write( OutputFileBNDetails, Format_713 ) << "   Plant Loop Connector,Mixer," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).Name ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString + "," + trim( adjustl( ChrOut ) ); //',Supply,'//  &
						for ( Count1 = 1; Count1 <= PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).TotalInletNodes; ++Count1 ) {
							gio::write( ChrOut, "*" ) << Count1;
							ChrOut2 = Blank;
							ChrOut3 = Blank;
							if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).BranchNumIn( Count1 ) <= 0 ) {
								ChrOut2 = errstring;
							}
							if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).BranchNumOut <= 0 ) {
								ChrOut3 = errstring;
							}
							{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << "     Plant Loop Connector Branches," + trim( adjustl( ChrOut ) ) + ",Mixer," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).Name ) + ","; }
							if ( ChrOut2 != errstring ) {
								{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Branch( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).BranchNumIn( Count1 ) ).Name ) + ","; }
							} else {
								{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << trim( ChrOut2 ) + ","; }
							}
							if ( ChrOut3 != errstring ) {
								gio::write( OutputFileBNDetails, Format_713 ) << trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Branch( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).BranchNumOut ).Name ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
							} else {
								gio::write( OutputFileBNDetails, Format_713 ) << trim( ChrOut3 ) + "," + trim( PlantLoop( Count ).Name ) + ",Supply";
							}
							gio::write( OutputFileBNDetails, Format_713 ) << "     Plant Loop Connector Nodes,   " + trim( adjustl( ChrOut ) ) + ",Mixer," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).Name ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).NodeNameIn( Count1 ) ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).NodeNameOut ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
						}
					}
				}
			}
			gio::write( OutputFileBNDetails, Format_713 ) << " Plant Loop Supply Connection," + trim( PlantLoop( Count ).Name ) + "," + trim( PlantLoop( Count ).LoopSide( SupplySide ).NodeNameOut ) + "," + trim( PlantLoop( Count ).LoopSide( DemandSide ).NodeNameIn );
			gio::write( OutputFileBNDetails, Format_713 ) << " Plant Loop Return Connection," + trim( PlantLoop( Count ).Name ) + "," + trim( PlantLoop( Count ).LoopSide( DemandSide ).NodeNameOut ) + "," + trim( PlantLoop( Count ).LoopSide( SupplySide ).NodeNameIn );

		} //  Plant Demand Side Loop

		//  Condenser Loops
		gio::write( OutputFileBNDetails, Format_701 ) << "! ===============================================================";
		gio::write( ChrOut, "*" ) << NumCondLoops;
		ChrOut = adjustl( ChrOut );
		gio::write( OutputFileBNDetails, Format_713 ) << "! <# Condenser Loops>,<Number of Condenser Loops>";
		gio::write( OutputFileBNDetails, Format_707 ) << "#Condenser Loops," + trim( ChrOut );
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Condenser Loop>,<Condenser Loop Name>,<Loop Type>,<Inlet Node Name>," "<Outlet Node Name>,<Branch List>,<Connector List>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Condenser Loop Connector>,<Connector Type>,<Connector Name>," "<Loop Name>,<Loop Type>,<Number of Inlets/Outlets>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Condenser Loop Connector Branches>,<Connector Node Count>,<Connector Type>," "<Connector Name>,<Inlet Branch>,<Outlet Branch>," "<Loop Name>,<Loop Type>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Condenser Loop Connector Nodes>,<Connector Node Count>,<Connector Type>," "<Connector Name>,<Inlet Node>,<Outlet Node>," "<Loop Name>,<Loop Type>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Condenser Loop Supply Connection>,<Condenser Loop Name>,<Supply Side Outlet Node Name>," "<Demand Side Inlet Node Name>";
		gio::write( OutputFileBNDetails, Format_713 ) << "! <Condenser Loop Return Connection>,<Condenser Loop Name>,<Demand Side Outlet Node Name>," "<Supply Side Inlet Node Name>";

		for ( Count = NumPlantLoops + 1; Count <= TotNumLoops; ++Count ) {
			for ( LoopSideNum = DemandSide; LoopSideNum <= SupplySide; ++LoopSideNum ) {
				//  Plant Supply Side Loop
				// Demandside and supplyside is parametrized in DataPlant
				if ( LoopSideNum == DemandSide ) {
					LoopString = "Demand";
				} else if ( LoopSideNum == SupplySide ) {
					LoopString = "Supply";
				}

				gio::write( OutputFileBNDetails, Format_713 ) << " Plant Loop," + trim( PlantLoop( Count ).Name ) + "," + LoopString + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).NodeNameIn ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).NodeNameOut ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).BranchList ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).ConnectList );
				//  Plant Supply Side Splitter
				for ( Num = 1; Num <= PlantLoop( Count ).LoopSide( LoopSideNum ).NumSplitters; ++Num ) {
					if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).Exists ) {
						gio::write( ChrOut, "*" ) << PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).TotalOutletNodes;
						gio::write( OutputFileBNDetails, Format_713 ) << "   Plant Loop Connector,Splitter," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).Name ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString + "," + trim( adjustl( ChrOut ) );
						for ( Count1 = 1; Count1 <= PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).TotalOutletNodes; ++Count1 ) {
							gio::write( ChrOut, "*" ) << Count1;
							ChrOut2 = Blank;
							ChrOut3 = Blank;
							if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).BranchNumIn <= 0 ) {
								ChrOut2 = errstring;
							}
							if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).BranchNumOut( Count1 ) <= 0 ) {
								ChrOut3 = errstring;
							}
							{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << "     Plant Loop Connector Branches," + trim( adjustl( ChrOut ) ) + ",Splitter," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).Name ) + ","; }
							if ( ChrOut2 != errstring ) {
								{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Branch( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).BranchNumIn ).Name ) + ","; }
							} else {
								{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << trim( ChrOut2 ) + ","; }
							}
							if ( ChrOut3 != errstring ) {
								gio::write( OutputFileBNDetails, Format_713 ) << trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Branch( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).BranchNumOut( Count1 ) ).Name ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
							} else {
								gio::write( OutputFileBNDetails, Format_713 ) << trim( ChrOut3 ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
							}
							gio::write( OutputFileBNDetails, Format_713 ) << "     Plant Loop Connector Nodes,   " + trim( adjustl( ChrOut ) ) + ",Splitter," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).Name ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).NodeNameIn ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Splitter( Num ).NodeNameOut( Count1 ) ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
						}
					}
				}
				//  Plant Supply Side Mixer
				for ( Num = 1; Num <= PlantLoop( Count ).LoopSide( LoopSideNum ).NumMixers; ++Num ) {
					if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).Exists ) {
						gio::write( ChrOut, "*" ) << PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).TotalInletNodes;
						gio::write( OutputFileBNDetails, Format_713 ) << "   Plant Loop Connector,Mixer," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).Name ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString + "," + trim( adjustl( ChrOut ) ); //',Supply,'//  &
						for ( Count1 = 1; Count1 <= PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).TotalInletNodes; ++Count1 ) {
							gio::write( ChrOut, "*" ) << Count1;
							ChrOut2 = Blank;
							ChrOut3 = Blank;
							if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).BranchNumIn( Count1 ) <= 0 ) {
								ChrOut2 = errstring;
							}
							if ( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).BranchNumOut <= 0 ) {
								ChrOut3 = errstring;
							}
							{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << "     Plant Loop Connector Branches," + trim( adjustl( ChrOut ) ) + ",Mixer," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).Name ) + ","; }
							if ( ChrOut2 != errstring ) {
								{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Branch( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).BranchNumIn( Count1 ) ).Name ) + ","; }
							} else {
								{ IOFlags flags; flags.ADVANCE( "No" ); gio::write( OutputFileBNDetails, Format_713, flags ) << trim( ChrOut2 ) + ","; }
							}
							if ( ChrOut3 != errstring ) {
								gio::write( OutputFileBNDetails, Format_713 ) << trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Branch( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).BranchNumOut ).Name ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
							} else {
								gio::write( OutputFileBNDetails, Format_713 ) << trim( ChrOut3 ) + "," + trim( PlantLoop( Count ).Name ) + ",Supply";
							}
							gio::write( OutputFileBNDetails, Format_713 ) << "     Plant Loop Connector Nodes,   " + trim( adjustl( ChrOut ) ) + ",Mixer," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).Name ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).NodeNameIn( Count1 ) ) + "," + trim( PlantLoop( Count ).LoopSide( LoopSideNum ).Mixer( Num ).NodeNameOut ) + "," + trim( PlantLoop( Count ).Name ) + "," + LoopString;
						}
					}
				}
			}
			gio::write( OutputFileBNDetails, Format_713 ) << " Plant Loop Supply Connection," + trim( PlantLoop( Count ).Name ) + "," + trim( PlantLoop( Count ).LoopSide( SupplySide ).NodeNameOut ) + "," + trim( PlantLoop( Count ).LoopSide( DemandSide ).NodeNameIn );
			gio::write( OutputFileBNDetails, Format_713 ) << " Plant Loop Return Connection," + trim( PlantLoop( Count ).Name ) + "," + trim( PlantLoop( Count ).LoopSide( DemandSide ).NodeNameOut ) + "," + trim( PlantLoop( Count ).LoopSide( SupplySide ).NodeNameIn );

		} //  Plant Demand Side Loop

		gio::write( OutputFileBNDetails, Format_701 ) << "! ===============================================================";
		NumOfControlledZones = 0;
		for ( Count = 1; Count <= NumOfZones; ++Count ) {
			if ( ! allocated( ZoneEquipConfig ) ) continue;
			if ( ZoneEquipConfig( Count ).IsControlled ) ++NumOfControlledZones;
		}
		gio::write( ChrOut, "*" ) << NumOfControlledZones;
		ChrOut = adjustl( ChrOut );
		if ( NumOfControlledZones > 0 ) {
			gio::write( OutputFileBNDetails, Format_713 ) << "! <# Controlled Zones>,<Number of Controlled Zones>";
			gio::write( OutputFileBNDetails, Format_707 ) << "#Controlled Zones," + trim( ChrOut );
			gio::write( OutputFileBNDetails, Format_713 ) << "! <Controlled Zone>,<Controlled Zone Name>,<Equip List Name>,<Control List Name>," "<Zone Node Name>,<Return Air Node Name>,<# Inlet Nodes>,<# Exhaust Nodes>";
			gio::write( OutputFileBNDetails, Format_713 ) << "! <Controlled Zone Inlet>,<Inlet Node Count>,<Controlled Zone Name>," "<Supply Air Inlet Node Name>,<SD Sys:Cooling/Heating [DD:Cooling] Inlet Node Name>," "<DD Sys:Heating Inlet Node Name>";
			gio::write( OutputFileBNDetails, Format_713 ) << "! <Controlled Zone Exhaust>,<Exhaust Node Count>,<Controlled Zone Name>," "<Exhaust Air Node Name>";
			for ( Count = 1; Count <= NumOfZones; ++Count ) {
				if ( ! ZoneEquipConfig( Count ).IsControlled ) continue;
				gio::write( ChrOut, "*" ) << ZoneEquipConfig( Count ).NumInletNodes;
				gio::write( ChrOut2, "*" ) << ZoneEquipConfig( Count ).NumExhaustNodes;
				ChrOut = adjustl( ChrOut );
				ChrOut2 = adjustl( ChrOut2 );
				gio::write( OutputFileBNDetails, Format_713 ) << " Controlled Zone," + trim( ZoneEquipConfig( Count ).ZoneName ) + "," + trim( ZoneEquipConfig( Count ).EquipListName ) + "," + trim( ZoneEquipConfig( Count ).ControlListName ) + "," + trim( NodeID( ZoneEquipConfig( Count ).ZoneNode ) ) + "," + trim( NodeID( ZoneEquipConfig( Count ).ReturnAirNode ) ) + "," + trim( ChrOut ) + "," + trim( ChrOut2 );
				for ( Count1 = 1; Count1 <= ZoneEquipConfig( Count ).NumInletNodes; ++Count1 ) {
					gio::write( ChrOut, "*" ) << Count1;
					ChrOut = adjustl( ChrOut );
					ChrName = NodeID( ZoneEquipConfig( Count ).AirDistUnitHeat( Count1 ).InNode );
					if ( ChrName == "Undefined" ) ChrName = "N/A";
					gio::write( OutputFileBNDetails, Format_713 ) << "   Controlled Zone Inlet," + trim( ChrOut ) + "," + trim( ZoneEquipConfig( Count ).ZoneName ) + "," + trim( NodeID( ZoneEquipConfig( Count ).InletNode( Count1 ) ) ) + "," + trim( NodeID( ZoneEquipConfig( Count ).AirDistUnitCool( Count1 ).InNode ) ) + "," + trim( ChrName );
				}
				for ( Count1 = 1; Count1 <= ZoneEquipConfig( Count ).NumExhaustNodes; ++Count1 ) {
					gio::write( ChrOut, "*" ) << Count1;
					ChrOut = adjustl( ChrOut );
					gio::write( OutputFileBNDetails, Format_713 ) << "   Controlled Zone Exhaust," + trim( ChrOut ) + "," + trim( ZoneEquipConfig( Count ).ZoneName ) + "," + trim( NodeID( ZoneEquipConfig( Count ).ExhaustNode( Count1 ) ) );
				}
			}

			//Report Zone Equipment Lists to BND File
			gio::write( OutputFileBNDetails, Format_721 ) << "! ===============================================================";
			gio::write( OutputFileBNDetails, Format_720 );
			gio::write( ChrOut, "*" ) << NumOfControlledZones;
			gio::write( OutputFileBNDetails, Format_721 ) << " #Zone Equipment Lists," + trim( adjustl( ChrOut ) );
			gio::write( OutputFileBNDetails, Format_722 );
			gio::write( OutputFileBNDetails, Format_723 );

			for ( Count = 1; Count <= NumOfZones; ++Count ) {
				// Zone equipment list array parallels controlled zone equipment array, so
				// same index finds corresponding data from both arrays
				if ( ! ZoneEquipConfig( Count ).IsControlled ) continue;
				gio::write( ChrOut, "*" ) << Count;
				gio::write( ChrOut2, "*" ) << ZoneEquipList( Count ).NumOfEquipTypes;
				gio::write( OutputFileBNDetails, Format_721 ) << " Zone Equipment List," + trim( adjustl( ChrOut ) ) + "," + trim( ZoneEquipList( Count ).Name ) + "," + trim( ZoneEquipConfig( Count ).ZoneName ) + "," + trim( adjustl( ChrOut2 ) );

				for ( Count1 = 1; Count1 <= ZoneEquipList( Count ).NumOfEquipTypes; ++Count1 ) {
					gio::write( ChrOut, "*" ) << Count1;
					gio::write( ChrOut2, "*" ) << ZoneEquipList( Count ).CoolingPriority( Count1 );
					gio::write( ChrOut3, "*" ) << ZoneEquipList( Count ).HeatingPriority( Count1 );
					gio::write( OutputFileBNDetails, Format_721 ) << "   Zone Equipment Component," + trim( adjustl( ChrOut ) ) + "," + trim( ZoneEquipList( Count ).EquipType( Count1 ) ) + "," + trim( ZoneEquipList( Count ).EquipName( Count1 ) ) + "," + trim( ZoneEquipConfig( Count ).ZoneName ) + "," + trim( adjustl( ChrOut2 ) ) + "," + trim( adjustl( ChrOut3 ) );
				}
			}
		}

		//Report Dual Duct Dampers to BND File
		ReportDualDuctConnections();

		if ( NumNodeConnectionErrors == 0 ) {
			ShowMessage( "No node connection errors were found." );
		} else {
			gio::write( ChrOut, "*" ) << NumNodeConnectionErrors;
			ChrOut = adjustl( ChrOut );
			if ( NumNodeConnectionErrors > 1 ) {
				ShowMessage( "There were " + trim( ChrOut ) + " node connection errors noted." );
			} else {
				ShowMessage( "There was " + trim( ChrOut ) + " node connection error noted." );
			}
		}

		AskForConnectionsReport = false;

	}

	void
	ReportParentChildren()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   May 2005
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// Reports parent compsets with ensuing children data.

		// METHODOLOGY EMPLOYED:
		// Uses IsParentObject,GetNumChildren,GetChildrenData

		// REFERENCES:
		// na

		// USE STATEMENTS:
		// na
		// Using/Aliasing
		using DataGlobals::OutputFileDebug;
		using General::TrimSigDigits;
		using namespace DataBranchNodeConnections;
		using namespace BranchNodeConnections;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		static Fstring const Blank;

		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		int Loop;
		int Loop1;
		FArray1D_Fstring ChildCType( sFstring( MaxNameLength ) );
		FArray1D_Fstring ChildCName( sFstring( MaxNameLength ) );
		FArray1D_Fstring ChildInNodeName( sFstring( MaxNameLength ) );
		FArray1D_Fstring ChildOutNodeName( sFstring( MaxNameLength ) );
		FArray1D_int ChildInNodeNum;
		FArray1D_int ChildOutNodeNum;
		int NumChildren;
		bool ErrorsFound;

		ErrorsFound = false;
		gio::write( OutputFileDebug, "(A)" ) << "Node Type,CompSet Name,Inlet Node,OutletNode";
		for ( Loop = 1; Loop <= NumOfActualParents; ++Loop ) {
			NumChildren = GetNumChildren( ParentNodeList( Loop ).CType, ParentNodeList( Loop ).CName );
			if ( NumChildren > 0 ) {
				ChildCType.allocate( NumChildren );
				ChildCName.allocate( NumChildren );
				ChildInNodeName.allocate( NumChildren );
				ChildOutNodeName.allocate( NumChildren );
				ChildInNodeNum.allocate( NumChildren );
				ChildOutNodeNum.allocate( NumChildren );
				ChildCType = Blank;
				ChildCName = Blank;
				ChildInNodeName = Blank;
				ChildOutNodeName = Blank;
				ChildInNodeNum = 0;
				ChildOutNodeNum = 0;
				GetChildrenData( ParentNodeList( Loop ).CType, ParentNodeList( Loop ).CName, NumChildren, ChildCType, ChildCName, ChildInNodeName, ChildInNodeNum, ChildOutNodeName, ChildOutNodeNum, ErrorsFound );
				if ( Loop > 1 ) gio::write( OutputFileDebug, "(1X,60('='))" );
				gio::write( OutputFileDebug, "(A)" ) << " Parent Node," + trim( ParentNodeList( Loop ).CType ) + ":" + trim( ParentNodeList( Loop ).CName ) + "," + trim( ParentNodeList( Loop ).InletNodeName ) + "," + trim( ParentNodeList( Loop ).OutletNodeName );
				for ( Loop1 = 1; Loop1 <= NumChildren; ++Loop1 ) {
					gio::write( OutputFileDebug, "(A)" ) << "..ChildNode," + trim( ChildCType( Loop1 ) ) + ":" + trim( ChildCName( Loop1 ) ) + "," + trim( ChildInNodeName( Loop1 ) ) + "," + trim( ChildOutNodeName( Loop1 ) );
				}
				ChildCType.deallocate();
				ChildCName.deallocate();
				ChildInNodeName.deallocate();
				ChildOutNodeName.deallocate();
				ChildInNodeNum.deallocate();
				ChildOutNodeNum.deallocate();
			} else {
				if ( Loop > 1 ) gio::write( OutputFileDebug, "(1X,60('='))" );
				gio::write( OutputFileDebug, "(A)" ) << " Parent Node (no children)," + trim( ParentNodeList( Loop ).CType ) + ":" + trim( ParentNodeList( Loop ).CName ) + "," + trim( ParentNodeList( Loop ).InletNodeName ) + "," + trim( ParentNodeList( Loop ).OutletNodeName );
			}
		}

	}

	void
	ReportCompSetMeterVariables()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   May 2005
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// Reports comp set meter variables.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using DataGlobals::OutputFileDebug;
		using namespace DataBranchNodeConnections;
		using namespace BranchNodeConnections;
		using namespace DataGlobalConstants;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS:

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		int Loop;
		int Loop1;
		int NumVariables;
		FArray1D_int VarIndexes;
		FArray1D_int VarIDs;
		FArray1D_int IndexTypes;
		FArray1D_int VarTypes;
		FArray1D_Fstring UnitsStrings( sFstring( MaxNameLength ) );
		FArray1D_Fstring VarNames( sFstring( MaxNameLength * 2 + 1 ) );
		FArray1D_int ResourceTypes;
		FArray1D_Fstring EndUses( sFstring( MaxNameLength ) );
		FArray1D_Fstring Groups( sFstring( MaxNameLength ) );

		gio::write( OutputFileDebug, "(A)" ) << " CompSet,ComponentType,ComponentName,NumMeteredVariables";
		gio::write( OutputFileDebug, "(A)" ) << " RepVar,ReportIndex,ReportID,ReportName,Units,ResourceType,EndUse,Group,IndexType";

		for ( Loop = 1; Loop <= NumCompSets; ++Loop ) {
			NumVariables = GetNumMeteredVariables( CompSets( Loop ).CType, CompSets( Loop ).CName );
			gio::write( OutputFileDebug, "(1X,'CompSet,',A,',',A,',',I5)" ) << trim( CompSets( Loop ).CType ) << trim( CompSets( Loop ).CName ) << NumVariables;
			if ( NumVariables <= 0 ) continue;
			VarIndexes.allocate( NumVariables );
			VarIndexes = 0;
			VarIDs.allocate( NumVariables );
			VarIDs = 0;
			IndexTypes.allocate( NumVariables );
			IndexTypes = 0;
			VarTypes.allocate( NumVariables );
			VarTypes = 0;
			VarNames.allocate( NumVariables );
			VarNames = " ";
			UnitsStrings.allocate( NumVariables );
			UnitsStrings = " ";
			ResourceTypes.allocate( NumVariables );
			ResourceTypes = 0;
			EndUses.allocate( NumVariables );
			EndUses = " ";
			Groups.allocate( NumVariables );
			Groups = " ";
			GetMeteredVariables( CompSets( Loop ).CType, CompSets( Loop ).CName, VarIndexes, VarTypes, IndexTypes, UnitsStrings, ResourceTypes, EndUses, Groups, VarNames, _, VarIDs );
			for ( Loop1 = 1; Loop1 <= NumVariables; ++Loop1 ) {
				gio::write( OutputFileDebug, "(1X,'RepVar,',I5,',',I5,',',A,',[',A,'],',A,',',A,',',A,',',I5)" ) << VarIndexes( Loop1 ) << VarIDs( Loop1 ) << trim( VarNames( Loop1 ) ) << trim( UnitsStrings( Loop1 ) ) << trim( GetResourceTypeChar( ResourceTypes( Loop1 ) ) ) << trim( EndUses( Loop1 ) ) << trim( Groups( Loop1 ) ) << IndexTypes( Loop1 );
			}
			VarIndexes.deallocate();
			IndexTypes.deallocate();
			VarTypes.deallocate();
			VarIDs.deallocate();
			VarNames.deallocate();
			UnitsStrings.deallocate();
			ResourceTypes.deallocate();
			EndUses.deallocate();
			Groups.deallocate();
		}

	}

	void
	PostIPProcessing()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   August 2010
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// This provides post processing (for errors, etc) directly after the InputProcessor
		// finishes.  Code originally in the Input Processor.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using SQLiteProcedures::CreateSQLiteDatabase;
		using InputProcessor::PreProcessorCheck;
		using InputProcessor::OverallErrorFlag;
		using InputProcessor::CompactObjectsCheck;
		using InputProcessor::ParametricObjectsCheck;
		using InputProcessor::GetNumSectionsFound;
		using InputProcessor::PreScanReportingVariables;
		using InputProcessor::NumOutOfRangeErrorsFound;
		using InputProcessor::NumBlankReqFieldFound;
		using InputProcessor::NumMiscErrorsFound;
		using FluidProperties::FluidIndex_Water;
		using FluidProperties::FluidIndex_EthyleneGlycol;
		using FluidProperties::FluidIndex_PropoleneGlycol;
		using FluidProperties::FindGlycol;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		// na

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		static bool PreP_Fatal( false ); // True if a preprocessor flags a fatal error

		DoingInputProcessing = false;

		CreateSQLiteDatabase();

		PreProcessorCheck( PreP_Fatal ); // Check Preprocessor objects for warning, severe, etc errors.

		CheckCachedIPErrors();

		if ( PreP_Fatal ) {
			ShowFatalError( "Preprocessor condition(s) cause termination." );
		}

		if ( OverallErrorFlag ) {
			ShowFatalError( "IP: Errors occurred on processing IDF file. Preceding condition(s) cause termination." );
		}

		CompactObjectsCheck(); // Check to see if Compact Objects (CompactHVAC, etc) are in input file.
		// If so, ExpandObjects didn't get called...
		ParametricObjectsCheck(); // check to see if any parametric objects are in the input file
		// parametric preprocessor was not run

		if ( NumOutOfRangeErrorsFound + NumBlankReqFieldFound + NumMiscErrorsFound > 0 ) {
			ShowSevereError( "IP: Out of \"range\" values and/or blank required fields found in input" );
			ShowFatalError( "IP: Errors occurred on processing IDF file. Preceding condition(s) cause termination." );
		}

		if ( GetNumSectionsFound( "DISPLAYALLWARNINGS" ) > 0 ) {
			DisplayAllWarnings = true;
			DisplayExtraWarnings = true;
			DisplayUnusedSchedules = true;
			DisplayUnusedObjects = true;
		}

		if ( GetNumSectionsFound( "DISPLAYEXTRAWARNINGS" ) > 0 ) {
			DisplayExtraWarnings = true;
		}

		if ( GetNumSectionsFound( "DISPLAYUNUSEDOBJECTS" ) > 0 ) {
			DisplayUnusedObjects = true;
		}

		if ( GetNumSectionsFound( "DISPLAYUNUSEDSCHEDULES" ) > 0 ) {
			DisplayUnusedSchedules = true;
		}

		if ( GetNumSectionsFound( "DisplayZoneAirHeatBalanceOffBalance" ) > 0 ) {
			DisplayZoneAirHeatBalanceOffBalance = true;
		}

		if ( GetNumSectionsFound( "DISPLAYADVANCEDREPORTVARIABLES" ) > 0 ) {
			DisplayAdvancedReportVariables = true;
		}

		//Set up more globals - process fluid input.
		FluidIndex_Water = FindGlycol( "Water" );
		FluidIndex_EthyleneGlycol = FindGlycol( "EthyleneGlycol" );
		FluidIndex_PropoleneGlycol = FindGlycol( "PropoleneGlycol" );

		PreScanReportingVariables();

	}

	void
	CheckCachedIPErrors()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   August 2010
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// This routine displays the cached error messages after the preprocessor
		// errors have been checked and produced.

		// METHODOLOGY EMPLOYED:
		// na

		// REFERENCES:
		// na

		// Using/Aliasing
		using SQLiteProcedures::WriteOutputToSQLite;
		using SQLiteProcedures::CreateSQLiteErrorRecord;
		using SQLiteProcedures::UpdateSQLiteErrorRecord;

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		static Fstring const fmta( "(A)" );

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		int iostatus;
		Fstring ErrorMessage( 500 );

		gio::close( CacheIPErrorFile );
		gio::open( CacheIPErrorFile, "eplusout.iperr" );
		iostatus = 0;
		while ( iostatus == 0 ) {
			{ IOFlags flags; gio::read( CacheIPErrorFile, fmta, flags ) >> ErrorMessage; iostatus = flags.ios(); }
			if ( iostatus != 0 ) break;
			ShowErrorMessage( trim( ErrorMessage ) );
			if ( WriteOutputToSQLite ) {
				// Following code relies on specific formatting of Severes, Warnings, and continues
				// that occur in the IP processing.  Later ones -- i.e. Fatals occur after the
				// automatic sending of error messages to SQLite are turned on.
				if ( ErrorMessage( 5, 5 ) == "S" ) {
					CreateSQLiteErrorRecord( 1, 1, ErrorMessage, 0 );
				} else if ( ErrorMessage( 5, 5 ) == "W" ) {
					CreateSQLiteErrorRecord( 1, 0, ErrorMessage, 0 );
				} else if ( ErrorMessage( 7, 7 ) == "~" ) {
					UpdateSQLiteErrorRecord( ErrorMessage );
				}
			}
		}

		{ IOFlags flags; flags.DISPOSE( "delete" ); gio::close( CacheIPErrorFile, flags ); }

	}

	void
	CheckThreading()
	{

		// SUBROUTINE INFORMATION:
		//       AUTHOR         Linda Lawrie
		//       DATE WRITTEN   April 2012
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// Check number of threads available versus number of surfaces, etc.

		// METHODOLOGY EMPLOYED:
		// Check Max Threads (OMP_NUM_THREADS) = MaxNumberOfThreads, iEnvSetThreads
		// Check EP Max Threads (EP_OMP_NUM_THREADS) = iepEnvSetThreads
		// Check if IDF input (ProgramControl) = iIDFSetThreads
		// Check # active sims (cntActv) = inumActiveSims [report only?]

		// REFERENCES:
		// na

		// Using/Aliasing
		using namespace DataSystemVariables;
		using InputProcessor::GetNumObjectsFound;
		using InputProcessor::GetObjectItem;
		using namespace DataIPShortCuts;
		using General::RoundSigDigits;
#if defined(_OPENMP) && defined(HBIRE_USE_OMP)
		using omp_lib::omp_get_max_threads;
		using omp_lib::omp_get_num_threads;
		using omp_lib::omp_set_num_threads;
#endif

		// Locals
		// SUBROUTINE ARGUMENT DEFINITIONS:
		// na

		// SUBROUTINE PARAMETER DEFINITIONS:
		static Fstring const Blank;

		// INTERFACE BLOCK SPECIFICATIONS:
		// na

		// DERIVED TYPE DEFINITIONS:
		// na

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		Fstring cEnvValue( 10 );
		int ios;
		int TotHTSurfs; // Number of BuildingSurface:Detailed items to obtain
		int TotDetailedWalls; // Number of Wall:Detailed items to obtain
		int TotDetailedRoofs; // Number of RoofCeiling:Detailed items to obtain
		int TotDetailedFloors; // Number of Floor:Detailed items to obtain
		int TotHTSubs; // Number of FenestrationSurface:Detailed items to obtain
		int TotIntMass; // Number of InternalMass items to obtain
		// Simple Surfaces (Rectangular)
		int TotRectExtWalls; // Number of Exterior Walls to obtain
		int TotRectIntWalls; // Number of Adiabatic Walls to obtain
		int TotRectIZWalls; // Number of Interzone Walls to obtain
		int TotRectUGWalls; // Number of Underground to obtain
		int TotRectRoofs; // Number of Roofs to obtain
		int TotRectCeilings; // Number of Adiabatic Ceilings to obtain
		int TotRectIZCeilings; // Number of Interzone Ceilings to obtain
		int TotRectGCFloors; // Number of Floors with Ground Contact to obtain
		int TotRectIntFloors; // Number of Adiabatic Walls to obtain
		int TotRectIZFloors; // Number of Interzone Floors to obtain
		int TotRectWindows;
		int TotRectDoors;
		int TotRectGlazedDoors;
		int TotRectIZWindows;
		int TotRectIZDoors;
		int TotRectIZGlazedDoors;
		int iIDFsetThreadsInput;
		int NumAlphas;
		int NumNumbers;

		// Figure out how many surfaces there are.
		TotHTSurfs = GetNumObjectsFound( "BuildingSurface:Detailed" );
		TotDetailedWalls = GetNumObjectsFound( "Wall:Detailed" );
		TotDetailedRoofs = GetNumObjectsFound( "RoofCeiling:Detailed" );
		TotDetailedFloors = GetNumObjectsFound( "Floor:Detailed" );
		TotHTSubs = GetNumObjectsFound( "FenestrationSurface:Detailed" );
		TotIntMass = GetNumObjectsFound( "InternalMass" );
		TotRectWindows = GetNumObjectsFound( "Window" );
		TotRectDoors = GetNumObjectsFound( "Door" );
		TotRectGlazedDoors = GetNumObjectsFound( "GlazedDoor" );
		TotRectIZWindows = GetNumObjectsFound( "Window:Interzone" );
		TotRectIZDoors = GetNumObjectsFound( "Door:Interzone" );
		TotRectIZGlazedDoors = GetNumObjectsFound( "GlazedDoor:Interzone" );
		TotRectExtWalls = GetNumObjectsFound( "Wall:Exterior" );
		TotRectIntWalls = GetNumObjectsFound( "Wall:Adiabatic" );
		TotRectIZWalls = GetNumObjectsFound( "Wall:Interzone" );
		TotRectUGWalls = GetNumObjectsFound( "Wall:Underground" );
		TotRectRoofs = GetNumObjectsFound( "Roof" );
		TotRectCeilings = GetNumObjectsFound( "Ceiling:Adiabatic" );
		TotRectIZCeilings = GetNumObjectsFound( "Ceiling:Interzone" );
		TotRectGCFloors = GetNumObjectsFound( "Floor:GroundContact" );
		TotRectIntFloors = GetNumObjectsFound( "Floor:Adiabatic " );
		TotRectIZFloors = GetNumObjectsFound( "Floor:Interzone" );

		iNominalTotSurfaces = TotHTSurfs + TotDetailedWalls + TotDetailedRoofs + TotDetailedFloors + TotHTSubs + TotIntMass + TotRectWindows + TotRectDoors + TotRectGlazedDoors + TotRectIZWindows + TotRectIZDoors + TotRectIZGlazedDoors + TotRectExtWalls + TotRectIntWalls + TotRectIZWalls + TotRectUGWalls + TotRectRoofs + TotRectCeilings + TotRectIZCeilings + TotRectGCFloors + TotRectIntFloors + TotRectIZFloors;

#ifdef HBIRE_USE_OMP
		MaxNumberOfThreads = MAXTHREADS();
		Threading = true;

		cEnvValue = " ";
		get_environment_variable( cNumThreads, cEnvValue );
		if ( cEnvValue != Blank ) {
			lEnvSetThreadsInput = true;
			{ IOFlags flags; gio::read( cEnvValue, "*", flags ) >> iEnvSetThreads; ios = flags.ios(); }
			if ( ios != 0 ) iEnvSetThreads = MaxNumberOfThreads;
			if ( iEnvSetThreads == 0 ) iEnvSetThreads = MaxNumberOfThreads;
		}

		cEnvValue = " ";
		get_environment_variable( cepNumThreads, cEnvValue );
		if ( cEnvValue != Blank ) {
			lepSetThreadsInput = true;
			{ IOFlags flags; gio::read( cEnvValue, "*", flags ) >> iepEnvSetThreads; ios = flags.ios(); }
			if ( ios != 0 ) iepEnvSetThreads = MaxNumberOfThreads;
			if ( iepEnvSetThreads == 0 ) iepEnvSetThreads = MaxNumberOfThreads;
		}

		cCurrentModuleObject = "ProgramControl";
		if ( GetNumObjectsFound( cCurrentModuleObject ) > 0 ) {
			GetObjectItem( cCurrentModuleObject, 1, cAlphaArgs, NumAlphas, rNumericArgs, NumNumbers, ios, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
			iIDFSetThreads = int( rNumericArgs( 1 ) );
			lIDFsetThreadsInput = true;
			if ( iIDFSetThreads <= 0 ) {
				iIDFSetThreads = MaxNumberOfThreads;
				if ( lEnvSetThreadsInput ) iIDFSetThreads = iEnvSetThreads;
				if ( lepSetThreadsInput ) iIDFSetThreads = iepEnvSetThreads;
			}
			if ( iIDFSetThreads > MaxNumberOfThreads ) {
				ShowWarningError( "CheckThreading: Your chosen number of threads=[" + trim( RoundSigDigits( iIDFSetThreads ) ) + "] is greater than the maximum number of threads=[" + trim( RoundSigDigits( MaxNumberOfThreads ) ) + "]." );
				ShowContinueError( "...execution time for this run may be degraded." );
			}
		}

		if ( iNominalTotSurfaces <= 30 ) {
			NumberIntRadThreads = 1;
			if ( lEnvSetThreadsInput ) NumberIntRadThreads = iEnvSetThreads;
			if ( lepSetThreadsInput ) NumberIntRadThreads = iepEnvSetThreads;
			if ( lIDFSetThreadsInput ) NumberIntRadThreads = iIDFSetThreads;
		} else {
			NumberIntRadThreads = MaxNumberOfThreads;
			if ( lEnvSetThreadsInput ) NumberIntRadThreads = iEnvSetThreads;
			if ( lepSetThreadsInput ) NumberIntRadThreads = iepEnvSetThreads;
			if ( lIDFSetThreadsInput ) NumberIntRadThreads = iIDFSetThreads;
		}
#else
		Threading = false;
		cCurrentModuleObject = "ProgramControl";
		if ( GetNumObjectsFound( cCurrentModuleObject ) > 0 ) {
			GetObjectItem( cCurrentModuleObject, 1, cAlphaArgs, NumAlphas, rNumericArgs, NumNumbers, ios, lNumericFieldBlanks, lAlphaFieldBlanks, cAlphaFieldNames, cNumericFieldNames );
			iIDFsetThreadsInput = int( rNumericArgs( 1 ) );
			if ( iIDFSetThreads > 1 ) {
				ShowWarningError( "CheckThreading: " + trim( cCurrentModuleObject ) + " is not available in this version." );
				ShowContinueError( "...user requested [" + trim( RoundSigDigits( iIDFsetThreadsInput ) ) + "] threads." );
			}
		}
		MaxNumberOfThreads = 1;
#endif
		// just reporting
		cEnvValue = " ";
		get_environment_variable( cNumActiveSims, cEnvValue );
		if ( cEnvValue != Blank ) {
			lnumActiveSims = true;
			{ IOFlags flags; gio::read( cEnvValue, "*", flags ) >> inumActiveSims; ios = flags.ios(); }
		}

	}

} // SimulationManager

// EXTERNAL SUBROUTINES:

void
Resimulate(
	bool & ResimExt, // Flag to resimulate the exterior energy use simulation
	bool & ResimHB, // Flag to resimulate the heat balance simulation (including HVAC)
	bool & ResimHVAC // Flag to resimulate the HVAC simulation
)
{

	// SUBROUTINE INFORMATION:
	//       AUTHOR         Peter Graham Ellis
	//       DATE WRITTEN   August 2005
	//       MODIFIED       Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems
	//       RE-ENGINEERED  na

	// PURPOSE OF THIS SUBROUTINE:
	// This subroutine is called as necessary by the Demand Manager to resimulate some of the modules that have
	// already been simulated for the current timestep.  For example, if LIGHTS are demand limited, the lighting
	// power is reduced which also impacts the zone internal heat gains and therefore requires that the entire
	// zone heat balance must be resimulated.

	// METHODOLOGY EMPLOYED:
	// If the zone heat balance must be resimulated, all the major subroutines are called sequentially in order
	// to recalculate the impacts of demand limiting.  This routine is called from ManageHVAC _before_ any variables
	// are reported or histories are updated.  This routine can be called multiple times without the overall
	// simulation moving forward in time.
	// If only HVAC components are demand limited, then the HVAC system is resimulated, not the entire heat balance.
	// Similarly, if ony exterior lights and equipment are demand limited, it is only necessary to resimulate the
	// exterior energy use, not the entire heat balance, nor the HVAC system.
	// Below is the hierarchy of subroutine calls.  The calls marked with an asterisk are resimulated here.
	// ManageSimulation
	//     ManageWeather
	//     ManageDemand
	//   * ManageExteriorEnergyUse
	//     ManageHeatBalance
	//       * InitHeatBalance
	//             PerformSolarCalculations
	//         ManageSurfaceHeatBalance
	//           * InitSurfaceHeatBalance
	//                 ManageInternalHeatGains
	//           * CalcHeatBalanceOutsideSurf
	//           * CalcHeatBalanceInsideSurf
	//             ManageAirHeatBalance
	//                *InitAirHeatBalance
	//                 CalcHeatBalanceAir
	//                   * CalcAirFlow
	//                   * ManageRefrigeratedCaseRacks
	//                     ManageHVAC
	//                       * ManageZoneAirUpdates 'GET ZONE SETPOINTS'
	//                       * ManageZoneAirUpdates 'PREDICT'
	//                       * SimHVAC
	//                         UpdateDataandReport
	//                 ReportAirHeatBalance
	//             UpdateFinalSurfaceHeatBalance
	//             UpdateThermalHistories
	//             UpdateMoistureHistories
	//             ManageThermalComfort
	//             ReportSurfaceHeatBalance
	//         RecKeepHeatBalance
	//         ReportHeatBalance

	// Using/Aliasing
	using namespace DataPrecisionGlobals;
	using DemandManager::DemandManagerExtIterations;
	using DemandManager::DemandManagerHBIterations;
	using DemandManager::DemandManagerHVACIterations;
	using ExteriorEnergyUse::ManageExteriorEnergyUse;
	using HeatBalanceSurfaceManager::InitSurfaceHeatBalance;
	using HeatBalanceAirManager::InitAirHeatBalance;
	using RefrigeratedCase::ManageRefrigeratedCaseRacks;
	using ZoneTempPredictorCorrector::ManageZoneAirUpdates;
	using DataHeatBalFanSys::iGetZoneSetPoints;
	using DataHeatBalFanSys::iPredictStep;
	using DataHeatBalFanSys::iCorrectStep;
	using HVACManager::SimHVAC;
	using HVACManager::CalcAirFlowSimple;
	using DataHVACGlobals::UseZoneTimeStepHistory; // , InitDSwithZoneHistory
	using ZoneContaminantPredictorCorrector::ManageZoneContaminanUpdates;
	using DataContaminantBalance::Contaminant;

	// Locals
	// SUBROUTINE ARGUMENT DEFINITIONS:

	// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
	Real64 ZoneTempChange( 0.0 ); // Dummy variable needed for calling ManageZoneAirUpdates

	// FLOW:
	if ( ResimExt ) {
		ManageExteriorEnergyUse();

		++DemandManagerExtIterations;
	}

	if ( ResimHB ) {
		// Surface simulation
		InitSurfaceHeatBalance();
		CalcHeatBalanceOutsideSurf();
		CalcHeatBalanceInsideSurf();

		// Air simulation
		InitAirHeatBalance();
		ManageRefrigeratedCaseRacks();

		++DemandManagerHBIterations;
		ResimHVAC = true; // Make sure HVAC is resimulated too
	}

	if ( ResimHVAC ) {
		// HVAC simulation
		ManageZoneAirUpdates( iGetZoneSetPoints, ZoneTempChange, false, UseZoneTimeStepHistory, 0.0 );
		if ( Contaminant.SimulateContaminants ) ManageZoneContaminanUpdates( iGetZoneSetPoints, false, UseZoneTimeStepHistory, 0.0 );
		CalcAirFlowSimple();
		ManageZoneAirUpdates( iPredictStep, ZoneTempChange, false, UseZoneTimeStepHistory, 0.0 );
		if ( Contaminant.SimulateContaminants ) ManageZoneContaminanUpdates( iPredictStep, false, UseZoneTimeStepHistory, 0.0 );
		SimHVAC();

		++DemandManagerHVACIterations;
	}

}

//     NOTICE
//     Copyright � 1996-2014 The Board of Trustees of the University of Illinois
//     and The Regents of the University of California through Ernest Orlando Lawrence
//     Berkeley National Laboratory.  All rights reserved.
//     Portions of the EnergyPlus software package have been developed and copyrighted
//     by other individuals, companies and institutions.  These portions have been
//     incorporated into the EnergyPlus software package under license.   For a complete
//     list of contributors, see "Notice" located in EnergyPlus.f90.
//     NOTICE: The U.S. Government is granted for itself and others acting on its
//     behalf a paid-up, nonexclusive, irrevocable, worldwide license in this data to
//     reproduce, prepare derivative works, and perform publicly and display publicly.
//     Beginning five (5) years after permission to assert copyright is granted,
//     subject to two possible five year renewals, the U.S. Government is granted for
//     itself and others acting on its behalf a paid-up, non-exclusive, irrevocable
//     worldwide license in this data to reproduce, prepare derivative works,
//     distribute copies to the public, perform publicly and display publicly, and to
//     permit others to do so.
//     TRADEMARKS: EnergyPlus is a trademark of the US Department of Energy.


} // EnergyPlus
