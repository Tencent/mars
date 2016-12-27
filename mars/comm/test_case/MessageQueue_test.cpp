
#include "../messagequeue/message_queue.h"
#include "gtest/gtest.h"

#include "boost/bind.hpp"
#include "thread/thread.h"


namespace
{

static int test_0()
{
	return 100;
}

static void test(int i)
{
}


static int test_1(int _i)
{
	return _i;
}

static int& test_2(int _i)
{
	static int i = 0;
	i = _i;
	return i;
}

static const int& test_3(int _i)
{
	static int i = 0;
	i = _i;
	return i;
}

static void callback0(bool _v)
{
}

static int sg_callback = 0;
static void callback(int& _i, bool _v)
{
	if (_v) sg_callback = _i;
}

static void callback1(const int& _i, bool _v)
{
	if (_v) sg_callback = _i;
}

static int sg_callback_false = 0;
static void callback_false(const int& _i, bool _v)
{
	if (_v)
		sg_callback_false = _i;
	else
		sg_callback_false = -1;
}

template <typename R>
class AsyncResult1
{
public:
	template<typename T>
	AsyncResult1(const T& _func)
		: m_function(m_function_holder), m_callback_function(m_callback_function_holder), m_result(m_result_holder), m_result_valid(m_result_valid_holder)
		, m_function_holder(_func), m_result_valid_holder(false)
	{
		BOOST_STATIC_ASSERT(boost::is_same<boost::result_of<T()>::type, R>::value);
	}

	template<typename T>
	AsyncResult1(const T& _func, R& _return_holder)
		: m_function(m_function_holder), m_callback_function(m_callback_function_holder), m_result(_return_holder), m_result_valid(m_result_valid_holder)
		, m_function_holder(_func), m_result_valid_holder(false)
	{
		BOOST_STATIC_ASSERT(boost::is_same<boost::result_of<T()>::type, R>::value);
	}

	template<typename T, typename C>
	AsyncResult1(const T& _func, const C& _callback)
		: m_function(m_function_holder), m_callback_function(m_callback_function_holder), m_result(m_result_holder), m_result_valid(m_result_valid_holder)
		, m_function_holder(_func), m_callback_function_holder(_callback), m_result_valid_holder(false)
	{
		BOOST_STATIC_ASSERT(boost::is_same<boost::result_of<T()>::type, R>::value);
	}

	template<typename T, typename C>
	AsyncResult1(const T& _func, R& _return_holder, const C& _callback)
		: m_function(m_function_holder), m_callback_function(m_callback_function_holder), m_result(_return_holder), m_result_valid(m_result_valid_holder)
		, m_function_holder(_func), m_callback_function_holder(_callback), m_result_valid_holder(false)
	{
		BOOST_STATIC_ASSERT(boost::is_same<boost::result_of<T()>::type, R>::value);
	}

	AsyncResult1(const AsyncResult1& _ref)
		: m_function(_ref.m_function), m_callback_function(_ref.m_callback_function), m_result(_ref.m_result), m_result_valid(_ref.m_result_valid) {}

	void operator()()
	{
		m_result = m_function();
		m_result_valid = true;
		if (m_callback_function)
			m_callback_function(Result());
	}

	boost::function<R ()>& Function() { return m_function;}
	boost::function<void (R&)>& CallFunction() { return m_callback_function;}
	R& Result() { return m_result;}
	operator bool() const { return m_result_valid;}

private:
	AsyncResult1& operator=(const AsyncResult1&);

private:
	boost::function<R ()>& m_function;
	boost::function<void (R&)>& m_callback_function;
	R& m_result;
	bool& m_result_valid;

	boost::function<R ()> m_function_holder;
	boost::function<void (R&)> m_callback_function_holder;
	R  m_result_holder;
	bool m_result_valid_holder;
};

}
TEST(MessageQueue_test, AsyncResult_test_sync)
{
	{
		MessageQueue::AsyncResult<void> result(boost::bind(&test, 99), &callback0);
		MessageQueue::WaitAsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));
		EXPECT_EQ(result, true);
	}

	{
		int ret = 0;
		MessageQueue::AsyncResult<int> result(&test_0, ret, &callback);
		MessageQueue::WaitAsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));
		EXPECT_EQ(result, true);

		EXPECT_EQ(result.Result(), 100);
		EXPECT_EQ(ret, 100);
		EXPECT_EQ(sg_callback, 100);
	}
	{
		sg_callback = 0;
		MessageQueue::AsyncResult<int> result(&test_0, &callback);
		MessageQueue::WaitAsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));
		EXPECT_EQ(result, true);

		EXPECT_EQ(result.Result(), 100);
		EXPECT_EQ(sg_callback, 100);
	}

	{
		sg_callback = 0;
		MessageQueue::AsyncResult<int> result(boost::bind(test_1, 101), &callback1);
		MessageQueue::WaitAsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));
		EXPECT_EQ(result, true);

		EXPECT_EQ(result.Result(), 101);
		EXPECT_EQ(sg_callback, 101);
	}

	{
		int ret = 0;
		MessageQueue::AsyncResult<int> result(boost::bind(test_1, 102), ret);
		MessageQueue::WaitAsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));

		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 102);
		EXPECT_EQ(ret, 102);
	}

	{
		sg_callback = 0;
		MessageQueue::AsyncResult<int&> result(boost::bind(test_2, 103), &callback1);
		MessageQueue::WaitAsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));

		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 103);
		EXPECT_EQ(sg_callback, 103);
	}
	{
		sg_callback = 0;
		MessageQueue::AsyncResult<const int&> result(boost::bind(test_3, 104), &callback1);
		MessageQueue::WaitAsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));

		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 104);
		EXPECT_EQ(sg_callback, 104);
	}

	{
		sg_callback = 0;
		MessageQueue::AsyncResult<int&> result(boost::bind(test_2, 105), &callback1);
		MessageQueue::WaitAsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));

		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 105);
		EXPECT_EQ(sg_callback, 105);
	}
}


TEST(MessageQueue_test, AsyncResult_test_Async)
{
	{
		int ret = 0;
		MessageQueue::AsyncResult<int> result(&test_0, ret, &callback);
		EXPECT_NE(MessageQueue::KNullPost, MessageQueue::AsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue())));
		ThreadUtil::sleep(2);
		
		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 100);
		EXPECT_EQ(ret, 100);
		EXPECT_EQ(sg_callback, 100);
	}

	{
		sg_callback = 0;
		MessageQueue::AsyncResult<int> result(&test_0, &callback);
		EXPECT_NE(MessageQueue::KNullPost, MessageQueue::AsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue())));
		ThreadUtil::sleep(2);
		EXPECT_EQ(result, true);

		EXPECT_EQ(result.Result(), 100);
		EXPECT_EQ(sg_callback, 100);
	}

	{
		sg_callback = 0;
		MessageQueue::AsyncResult<int> result(boost::bind(test_1, 101), &callback1);
		EXPECT_NE(MessageQueue::KNullPost, MessageQueue::AsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue())));
		ThreadUtil::sleep(2);
		EXPECT_EQ(result, true);

		EXPECT_EQ(result.Result(), 101);
		EXPECT_EQ(sg_callback, 101);
	}

	{
		int ret = 0;
		MessageQueue::AsyncResult<int> result(boost::bind(test_1, 102), ret);
		EXPECT_NE(MessageQueue::KNullPost, MessageQueue::AsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue())));
		ThreadUtil::sleep(2);

		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 102);
		EXPECT_EQ(ret, 102);
	}

	{
		sg_callback = 0;
		MessageQueue::AsyncResult<int&> result(boost::bind(test_2, 103), &callback1);
		EXPECT_NE(MessageQueue::KNullPost, MessageQueue::AsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue())));
		ThreadUtil::sleep(2);

		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 103);
		EXPECT_EQ(sg_callback, 103);
	}
	{
		sg_callback = 0;
		MessageQueue::AsyncResult<const int&> result(boost::bind(test_3, 104), &callback1);
		EXPECT_NE(MessageQueue::KNullPost, MessageQueue::AsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue())));
		ThreadUtil::sleep(2);

		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 104);
		EXPECT_EQ(sg_callback, 104);
	}

	{
		sg_callback = 0;
		MessageQueue::AsyncResult<int&> result(boost::bind(test_2, 105), &callback1);
		EXPECT_NE(MessageQueue::KNullPost, MessageQueue::AsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue())));
		ThreadUtil::sleep(2);

		EXPECT_EQ(result, true);
		EXPECT_EQ(result.Result(), 105);
		EXPECT_EQ(sg_callback, 105);
	}
	{
		sg_callback_false = 0;
		MessageQueue::AsyncResult<int&> result(boost::bind(test_2, 106), &callback_false);
		MessageQueue::MessagePost_t post = MessageQueue::AsyncInvoke(result, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));
		MessageQueue::CancelMessage(post);


		EXPECT_NE(result, true);
		EXPECT_NE(sg_callback_false, 106);
	}
	{
		sg_callback_false = 0;
		MessageQueue::MessagePost_t post = MessageQueue::AsyncInvoke(MessageQueue::AsyncResult<int&>(boost::bind(test_2, 107), &callback_false), MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()));
		MessageQueue::CancelMessage(post);

		EXPECT_EQ(sg_callback_false, -1);
	}

}

static void WaitMessage_SingletonMessage_test()
{


}

TEST(MessageQueue_test, WaitMessage_SingletonMessage)
{
	{
		SingletonMessage(true, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()),
			MessageQueue::Message(123, WaitMessage_SingletonMessage_test ), MessageQueue::MessageTiming(2*1000));
		MessageQueue::WaitMessage(MessageQueue::SingletonMessage(false, MessageQueue::GetDefAsyncInvokeHandler(MessageQueue::GetDefTaskQueue()),
			MessageQueue::Message(123, WaitMessage_SingletonMessage_test ), MessageQueue::MessageTiming(2*1000)));
	}

}
