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

#include "configfile.h"
#include "rapidjson/reader.h"
#include <fstream>

using namespace rapidjson;
using namespace std;