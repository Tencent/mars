// Boost.Signals2 library

// Copyright Douglas Gregor 2001-2003.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/signals2.hpp>
#define BOOST_TEST_MODULE deletion_test
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <string>

static mars_boost::signals2::connection connections[5];

static std::string test_output;

struct remove_connection {
  explicit remove_connection(int v = 0, int i = -1) : value(v), idx(i) {}

  void operator()() const {
    if (idx >= 0)
      connections[idx].disconnect();

    //return value;
    std::cout << value << " ";

    test_output += static_cast<char>(value + '0');
  }

  int value;
  int idx;
};

bool operator==(const remove_connection& x, const remove_connection& y)
{ return x.value == y.value && x.idx == y.idx; }

static void
test_remove_self()
{
  mars_boost::signals2::signal<void ()> s0;

  connections[0] = s0.connect(remove_connection(0));
  connections[1] = s0.connect(remove_connection(1));
  connections[2] = s0.connect(remove_connection(2, 2));
  connections[3] = s0.connect(remove_connection(3));

  std::cout << "Deleting 2" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "0123");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "013");

  s0.disconnect_all_slots();
  BOOST_CHECK(s0.empty());

  connections[0] = s0.connect(remove_connection(0));
  connections[1] = s0.connect(remove_connection(1));
  connections[2] = s0.connect(remove_connection(2));
  connections[3] = s0.connect(remove_connection(3, 3));

  std::cout << "Deleting 3" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "0123");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "012");

  s0.disconnect_all_slots();
  BOOST_CHECK(s0.num_slots() == 0);

  connections[0] = s0.connect(remove_connection(0, 0));
  connections[1] = s0.connect(remove_connection(1));
  connections[2] = s0.connect(remove_connection(2));
  connections[3] = s0.connect(remove_connection(3));

  std::cout << "Deleting 0" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "0123");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "123");

  s0.disconnect_all_slots();
  BOOST_CHECK(s0.empty());

  connections[0] = s0.connect(remove_connection(0, 0));
  connections[1] = s0.connect(remove_connection(1, 1));
  connections[2] = s0.connect(remove_connection(2, 2));
  connections[3] = s0.connect(remove_connection(3, 3));

  std::cout << "Mass suicide" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "0123");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "");
}

static void
test_remove_prior()
{
  mars_boost::signals2::signal<void ()> s0;

  connections[0] = s0.connect(remove_connection(0));
  connections[1] = s0.connect(remove_connection(1, 0));
  connections[2] = s0.connect(remove_connection(2));
  connections[3] = s0.connect(remove_connection(3));

  std::cout << "1 removes 0" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "0123");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "123");

  s0.disconnect_all_slots();
  BOOST_CHECK(s0.empty());

  connections[0] = s0.connect(remove_connection(0));
  connections[1] = s0.connect(remove_connection(1));
  connections[2] = s0.connect(remove_connection(2));
  connections[3] = s0.connect(remove_connection(3, 2));

  std::cout << "3 removes 2" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "0123");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "013");
}

static void
test_remove_after()
{
  mars_boost::signals2::signal<void ()> s0;

  connections[0] = s0.connect(remove_connection(0, 1));
  connections[1] = s0.connect(remove_connection(1));
  connections[2] = s0.connect(remove_connection(2));
  connections[3] = s0.connect(remove_connection(3));

  std::cout << "0 removes 1" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "023");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "023");

  s0.disconnect_all_slots();
  BOOST_CHECK(s0.empty());

  connections[0] = s0.connect(remove_connection(0));
  connections[1] = s0.connect(remove_connection(1, 3));
  connections[2] = s0.connect(remove_connection(2));
  connections[3] = s0.connect(remove_connection(3));

  std::cout << "1 removes 3" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "012");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "012");
}

static void
test_bloodbath()
{
  mars_boost::signals2::signal<void ()> s0;

  connections[0] = s0.connect(remove_connection(0, 1));
  connections[1] = s0.connect(remove_connection(1, 1));
  connections[2] = s0.connect(remove_connection(2, 0));
  connections[3] = s0.connect(remove_connection(3, 2));

  std::cout << "0 removes 1, 2 removes 0, 3 removes 2" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "023");

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "3");
}

static void
test_disconnect_equal()
{
  mars_boost::signals2::signal<void ()> s0;

  connections[0] = s0.connect(remove_connection(0));
  connections[1] = s0.connect(remove_connection(1));
  connections[2] = s0.connect(remove_connection(2));
  connections[3] = s0.connect(remove_connection(3));

  std::cout << "Deleting 2" << std::endl;

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "0123");

#if BOOST_WORKAROUND(BOOST_MSVC, <= 1300)
  connections[2].disconnect();
#else
  s0.disconnect(remove_connection(2));
#endif

  test_output = "";
  s0(); std::cout << std::endl;
  BOOST_CHECK(test_output == "013");
}

struct signal_deletion_tester 
{ 
public:
  signal_deletion_tester() {
    b_has_run = false;
    sig = new mars_boost::signals2::signal<void(void)>();
    connection0 = sig->connect(0, mars_boost::bind(&signal_deletion_tester::a, this)); 
    connection1 = sig->connect(1, mars_boost::bind(&signal_deletion_tester::b, this));
  }
  
  ~signal_deletion_tester()
  {
    if(sig != 0)
      delete sig;
  }
  
  void a() 
  {
    if(sig != 0)
      delete sig;
    sig = 0;
  }
  
  void b() 
  {
    b_has_run = true;
  } 
  
  mars_boost::signals2::signal<void(void)> *sig;
  bool b_has_run;
  mars_boost::signals2::connection connection0;
  mars_boost::signals2::connection connection1;
}; 

// If a signal is deleted mid-invocation, the invocation in progress
// should complete normally.  Once all invocations complete, all
// slots which were connected to the deleted signal should be in the 
// disconnected state.
static void test_signal_deletion()
{
  signal_deletion_tester tester;
  (*tester.sig)();
  BOOST_CHECK(tester.b_has_run);
  BOOST_CHECK(tester.connection0.connected() == false);
  BOOST_CHECK(tester.connection1.connected() == false);
}

BOOST_AUTO_TEST_CASE(test_main)
{
  test_remove_self();
  test_remove_prior();
  test_remove_after();
  test_bloodbath();
  test_disconnect_equal();
  test_signal_deletion();
}
