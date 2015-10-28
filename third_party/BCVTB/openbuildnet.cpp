#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <obnnode.h>
#include "openbuildnet.h"

using namespace OBNnode;

namespace EnergyPlus {
    namespace ExternalInterface {
        
        EPlusSignalToOBN eplus_signal_to_obn = OBNSIG_NONE;       ///< The signal from E+ to OBN.
        std::mutex eplus_signal_to_obn_mutex;       ///< Mutex to access the signal
        std::condition_variable eplus_signal_to_obn_cond;   ///< Condition variable to wait for the signal
        
        
        /** \brief The class that runs the OBN node thread. */
        class EPlusOBNThread {
            MQTTNode m_obnnode;
            std::thread m_thread;
            
            /** Main function of the thread. */
            void threadMain();
            
        public:
            EPlusOBNThread(const std::string& t_name, const std::string& t_ws = ""): m_obnnode(t_name, t_ws)
            {
            }
            
            ~EPlusOBNThread() {
                // If the thread is running, we need to stop it properly
                stopThread();
            }
            
            /** Start the OBN node thread. */
            bool startThread() {
                if (m_thread.joinable()) {
                    // If the thread is already running, we can't start it again
                    return false;
                }
                
                // Start the node and the thread
                if (!m_obnnode.openSMNPort()) {
                    return false;
                }
                
                m_thread = std::thread(&EPlusOBNThread::threadMain, this);
                
                return true;
            }
            
            /** Stop the OBN node thread properly. */
            void stopThread() {
                if (m_thread.joinable()) {
                    // Signal the thread to exit
                    signalOBN(OBNSIG_EXIT);
                    m_thread.join();
                }
            }
            
        };
        
        /** Run the OBN Node thread, communicating with OBN system as well as EnergyPlus (via signals).
         This thread will terminate when the OBN system terminates or when it receives the EXIT signal from E+.
         */
        void EPlusOBNThread::threadMain() {
            std::unique_lock<std::mutex> mylock(eplus_signal_to_obn_mutex);
            eplus_signal_to_obn_cond.wait(mylock, []{ return eplus_signal_to_obn == OBNSIG_EXIT; });
        }
        
        std::unique_ptr<EPlusOBNThread> obn_thread;
        
        bool initOBNNode() {
            if (!obn_thread) {
                obn_thread.reset(new EPlusOBNThread("energyplusnode", "obnep"));
                
                // Start the thread
                return obn_thread->startThread();
            }
            
            return true;
        }
        
        void stopOBNNode() {
            if (!obn_thread) {
                obn_thread->stopThread();
            }
        }
        
        void signalOBN(EPlusSignalToOBN sig) {
            {
                std::lock_guard<std::mutex> mylock(eplus_signal_to_obn_mutex);
                eplus_signal_to_obn = sig;
            }
            eplus_signal_to_obn_cond.notify_all();
        }
    }
}
