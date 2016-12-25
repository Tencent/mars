/*
 * baseprj.cpp
 *
 *  Created on: 2014-7-7
 *      Author: yerungui
 */

#include "mars/baseevent/baseprjevent.h"

#include "mars/comm/compiler_util.h"
#include "mars/comm/bootregister.h"

namespace mars{
    namespace baseevent{
        
        void OnCreate()
        {
            GetSignalOnCreate()();
        }
        
        void OnDestroy()
        {
            GetSignalOnDestroy()();
        }
        
        void OnSingalCrash(int _sig)
        {
            GetSignalOnSingalCrash()(_sig);
        }
        
        void OnExceptionCrash()
        {
            GetSignalOnExceptionCrash()();
        }
        
        void OnForeground(bool _isforeground)
        {
            GetSignalOnForeground()(_isforeground);
        }
        
        void OnNetworkChange()
        {
            GetSignalOnNetworkChange()();
        }
    }
}

