
#include "boost/utility/result_of.hpp"
#include "comm\platform_comm.h"

#include "runtime2cs.h"
#include "comm\xlogger\xlogger.h"
#include "runtime_utils.h"

namespace mars {
		using namespace Platform;
		using namespace std;


		//static Runtime2Cs_Comm^ Singleton();
		Runtime2Cs_Comm^ Runtime2Cs_Comm::Singleton()
		{
			if (nullptr == instance)
			{
				instance = ref new Runtime2Cs_Comm();
			}

			return instance;
		}

		ICallback_Comm ^ Runtime2Cs_Comm::GetCallBack()
		{
			return m_callback;
		}


		Runtime2Cs_Comm::Runtime2Cs_Comm() :
			m_callback(nullptr)
		{
		}

		void Runtime2Cs_Comm::SetCallback(ICallback_Comm^ _callback)
		{
			m_callback = _callback;
		}

		Runtime2Cs_Comm^ Runtime2Cs_Comm::instance = nullptr;
	




}