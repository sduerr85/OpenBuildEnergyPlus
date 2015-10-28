#ifndef OBN_ENERGYPLUS_OPENBUILDNET_H
#define OBN_ENERGYPLUS_OPENBUILDNET_H

#include <atomic>

namespace EnergyPlus {
    namespace ExternalInterface {
        
        /** Initialize the OBN node and make it run on a separate thread. */
        bool initOBNNode();
        
        /** (Try to) Stop the OBN thread if it's running. */
        void stopOBNNode();
        
        // Internal Communication between the main (EnergyPlus) execution and the OBN node thread
        enum EPlusSignalToOBN {
            OBNSIG_NONE,     // no command
            OBNSIG_EXIT      // exit immediately (e.g. if there is an error in EnergyPlus)
        };
        
        /** Set the signal from E+ to OBN. */
        void signalOBN(EPlusSignalToOBN sig);
    }
}
#endif
