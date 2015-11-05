/* -*- mode: C++; indent-tabs-mode: nil; -*- */
/** \file
 * \brief Include OBN into EnergyPlus.
 *
 * This file is part of the openBuildNet simulation framework
 * (OBN-Sim) developed at EPFL.
 *
 * \author Truong X. Nghiem (xuan.nghiem@epfl.ch)
 */

#ifndef OBN_ENERGYPLUS_OPENBUILDNET_H
#define OBN_ENERGYPLUS_OPENBUILDNET_H

namespace EnergyPlus {
    namespace ExternalInterface {
        
        /** Initialize the OBN node and make it run on a separate thread.
         \param docname The name of the config file.
         \return true if successful.
         
         The config file is a text file which has the following format:
         <comm> [comm_config]
         <nodename> [workspace]
         where <comm> specifies the communication: currently only "mqtt" is supported.
         [comm_config] is the optional configuration of the communication: for MQTT, it's the server address and port.
         <nodename> is the required name of the node.
         [workspace] is the optional workspace name.
         */
        bool initOBNNode(const char * docname);
        
        /** (Try to) Stop the OBN thread if it's running. */
        void stopOBNNode();
        
        // Internal Communication from the main (EnergyPlus) execution to the OBN node thread
        enum EPlusSignalToOBN {
            OBNSIG_NONE = 0,     // no command
            OBNSIG_DONE,     // the required action is done (e.g. I've done sending out outputs after EPSIG_Y)
            OBNSIG_TERM,     // simulation terminated normally from the E+ side
            OBNSIG_EXIT      // exit immediately (e.g. if there is an error)
        };
        
        /** Set the signal from E+ to OBN. */
        void signalOBN(EPlusSignalToOBN sig);
        
        // Internal Communication from the OBN node thread to the main (EnergyPlus) execution
        enum OBNSignalToEPlus {
            EPSIG_NONE = 0,     // no command
            EPSIG_START,    // start the simulation (after receiving SIM_INIT)
            EPSIG_Y,        // compute outputs (UPDATE_Y)
            EPSIG_X,        // update states (UPDATE_X); all inputs should have been received
            EPSIG_TERM,     // simulation terminated (properly) from OBN side
            EPSIG_QUIT,     // simulation stopped improperly, e.g. due to an OBN error
            EPSIG_TIMEOUT   // Timeout while waiting for OBN's signal; the OBN itself will never set this signal value
        };

        /** Returns the signal from the OBN thread to E+. */
        OBNSignalToEPlus getOBNSignal();
        
        /** Returns the name of the signal from the OBN thread to E+. */
        std::string getOBNSignalName(OBNSignalToEPlus sig);
        
        /** Resets the signal from the OBN thread to E+. */
        void resetOBNSignal();
        
        /** Signal the OBN to terminate (properly from simulation). */
        void signalOBN_TERM();
        
        /** Signal the OBN to exit (improperly, often because of error). */
        void signalOBN_EXIT();
        
        /** Waits for the signal from the OBN thread to be not EPSIG_NONE, and returns the new signal.
         \param timeout Optional timeout value in seconds; non-positive values mean using the default timeout (default).
         \return The new signal from OBN, or EPSIG_TIMEOUT if timeout occurred.
         */
        OBNSignalToEPlus waitforOBNSignal(int timeout = -1);
        
        /** Set the default timeout value for waiting for OBN signals.
         This function should only be called at the beginning.
         \param timeout The default timeout value; non-positive values mean no timeout (default is -1 at startup).
         */
        void setOBNTimeout(int timeout);
        
        /** Exchanges data with openBuildNet.
         */
        int exchangedoublewithOBN(const int *flaWri, int *flaRea,
                                  const int *nDblWri,
                                  int *nDblRea,
                                  double dblValWri[],
                                  double *simTimRea,
                                  double dblValRea[]);
    }
}
#endif
