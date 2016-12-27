
#if 1
#include <stdlib.h>
#include "util_func.h"

#define  SIGNALLING_KEEPER_TEST
#ifdef SIGNALLING_KEEPER_TEST

#include "gtest/gtest.h"

#include "TestUtils.h"
#include "GtestEx.h"
#include "../../../comm/singleton.h"
#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"
#include "../../../network/net_core.h"
#include "../../../network/longlink_task_manager.h"
#include "../signalling_keeper.h"

extern std::string getAppPrivatePath();
static void onCreate()
{
	UtilFunc::del_files(getAppPrivatePath()+"host");
	CMMNetCore::Singleton();	
}
static void onDestroy()
{
	CMMNetCore::Release();
	SINGLETON_RELEASE_ALL();
	UtilFunc::del_files(getAppPrivatePath()+"host");
}

PUBC_TEST(SignallingKeeper_test, test0)
{
	onCreate();
	SignallingKeeper signallingKeeper(CMMNetCore::Singleton()->m_longLinkTaskManager->LongLinkChannel());
	signallingKeeper.SetStrategy(5 * 1000, 20*1000);
	signallingKeeper.Keep();
	BaseScene baseScene;
	PUBC_EXPECT_TRUE(signallingKeeper.m_keeping, &baseScene);
	signallingKeeper.OnDataSent(NULL);
	signallingKeeper.Stop();
	PUBC_EXPECT_FALSE(signallingKeeper.m_keeping, &baseScene);
	onDestroy();
}











#endif
#endif
