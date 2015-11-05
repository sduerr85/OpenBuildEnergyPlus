#ifndef OBN_ENERGYPLUS_OPENBUILDNET_H
#define OBN_ENERGYPLUS_OPENBUILDNET_H

namespace EnergyPlus {
    namespace ExternalInterface {
        
        /** Initialize the OBN node and make it run on a separate thread. */
        bool initOBNNode();
        
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
    }
}
#endif
