/*
 * baseprj.h
 *
 *  Created on: 2014-7-7
 *      Author: yerungui
 */

#ifndef MARS_BASELOGIC_H_
#define MARS_BASELOGIC_H_

namespace mars{
namespace baseevent{
    void OnCreate();
    void OnDestroy();
    void OnSingalCrash(int _sig);
    void OnExceptionCrash();
    void OnExitAppOrAppCrash();
    void OnForeground(bool _isforeground);
    void OnNetworkChange();
}
}

#endif /* MARS_BASELOGIC_H_ */
