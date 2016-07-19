/* -*- mode: C++; indent-tabs-mode: nil; -*- */
/** \file
 * \brief Code to read variable config file in JSON format.
 *
 * This replaces the XML config file in the original EnergyPlus/BCVTB interface.
 * It helps remove the dependency on Java for ExternalInterface.
 *
 * This file is part of the openBuildNet simulation framework
 * (OBN-Sim) developed at EPFL.
 *
 * \author Truong X. Nghiem (xuan.nghiem@epfl.ch)
 */

#ifndef OBN_ENERGYPLUS_CONFIGFILE_H
#define OBN_ENERGYPLUS_CONFIGFILE_H

extern "C" {
    
////////////////////////////////////////////////////////////////
///  This method will return the input and output variable for EnergyPlus
///  in sequence
///
///\param fileName the variable configuration file name.
///\param myOutputVarsName Array to store the output variable names found.
///\param myOutputvarsType Array to store the output variable types found.
///\param myNumOutputVars Integer holder to store number of output variables found.
///\param myInputKeys Array to store the input variable keys.
///\param myNumInputKeys Integer holder to store number of input variable keys.
///\param myInputVars Array to store the name of input variables found.
///\param myNumInputVars Integer holder to store number of input variables found.
///\param myInputVarsType Integer array to store the corresponding input variable types in myInputVars.
///\param myStrLen The length of the string that is passed to this function.
///
////////////////////////////////////////////////////////////////
int
getepvariables(
               char const *	const  fileName,
               char *	const myOutputVarsName,
               char *	const myOutputVarsType,
               int *	const myNumOutputVars,
               char const *	const myInputKeys,
               int const *	const myNumInputKeys,
               char *	const myInputVars,
               int *	const myNumInputVars,
               int *	const myInputVarsType,
               int const *	const myStrLen
               );

////////////////////////////////////////////////////////////////
///  This method will return the input and output variable for EnergyPlus
///  in sequence. The difference with getepvariables is that it does not
///  validate the configuration file
///
///\param fileName the variable configuration file name.
///\param myOutputVarsName Array to store the output variable names found.
///\param myOutputvarsType Array to store the output variable types found.
///\param myNumOutputVars Integer holder to store number of output variables found.
///\param myInputKeys Array to store the input variable keys.
///\param myNumInputKeys Integer holder to store number of input variable keys.
///\param myInputVars Array to store the name of input variables found.
///\param myNumInputVars Integer holder to store number of input variables found.
///\param myInputVarsType Integer array to store the corresponding input variable types in myInputVars.
///\param myStrLen The length of the string that is passed to this function.
///
////////////////////////////////////////////////////////////////
int
getepvariablesFMU(
                  char const *	const fileName,
                  char *	const myOutputVarsName,
                  char *	const myOutputVarsType,
                  int *	const myNumOutputVars,
                  char const *	const myInputKeys,
                  int const *	const myNumInputKeys,
                  char *	const myInputVars,
                  int *	const myNumInputVars,
                  int *	const myInputVarsType,
                  int const *	const myStrLen
                  );

} // extern "C"
#endif /* OBN_ENERGYPLUS_CONFIGFILE_H */
