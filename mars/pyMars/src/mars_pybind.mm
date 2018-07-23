// author:dj


#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <pybind11/embed.h> 
#include <pybind11/eval.h>
#include "pybind11/functional.h"
#include <xlogger/xlogger.h>

namespace py = pybind11;

// ----------------------------------------- //

#include <log/appender.h>
#include <mars/app/app_logic.h>
#include <mars/baseevent/base_logic.h>

#include <stn_callback.h>
#include <stn_logic.h>
#include <app_callback.h>
#include <stnproto_logic.h>
#include "CGITask.h"
#include <CommandID.h>

#include <mars/comm/xlogger/xloggerbase.h>


//////////////////////////////////////////////////////
// private methods.
//////////////////////////////////////////////////////
int _setCallBack(py::object &func) {
  mars::stn::SetCallback(mars::stn::StnCallBack::Instance(func));
  mars::app::SetCallback(mars::app::AppCallBack::Instance());

  return 0;
}

int _createMars() {
  mars::baseevent::OnCreate();

  return 0;
}


// 设置服务器地址和长连端口，以及 debug ip，如果需要的话.
int _set_address_and_port(const std::string ip_address, \
                          const std::string debug_ip, \
                          const unsigned short port ) {
   std::vector<uint16_t> ports;
   ports.push_back(port);
   mars::stn::SetLonglinkSvrAddr(ip_address, ports, debug_ip);
   return 0;
}

// 设置服务器短连端口
int _set_short_link_port(const unsigned short port) {
  mars::stn::SetShortlinkSvrAddr(port, "");
  return 0;
}

// 换到前台需要调用
int _report_event_on_forground(int is_forground) {
  mars::baseevent::OnForeground(is_forground);
  return 0;
}

// 
int _makesure_longlink_connected() {
  mars::stn::MakesureLonglinkConnected();
  return 0;
}

// 
int _stop_task(const int taskid) {
  mars::stn::StopTask((int32_t)taskid);
  return 0;
}

// callbacks
int _is_authed() {
  return 1;
}

//////////////////////////////////////////////////////
// To be or not to be.
//////////////////////////////////////////////////////

int _start_task(uint32_t cmdid, ChannelType channel_select, \
                std::string str_cgi_uri, std::string str_host, \
                std::string str_msg) {
  mars::stn::Task ctask;
  ctask.cmdid = cmdid;
  ctask.channel_select = channel_select;
  ctask.cgi = str_cgi_uri;
  ctask.shortlink_host_list.push_back(str_host);

  ctask.user_context = static_cast<void*>(new std::string(str_msg.c_str()));
    
  mars::stn::StartTask(ctask);
  printf("taskid:%d\n", ctask.taskid);
  return ctask.taskid;
}

int send_message(std::string str_msg){
  _start_task(kSendMsg, ChannelType_LongConn, \
                "/mars/sendmessage", "106.75.93.248", str_msg);
  return 0;
} 

int mars_init(py::object &func) {
    xlogger_SetLevel(kLevelAll);
    appender_set_console_log(true);
    // appender_open(kAppednerSync, "/Users/lapsule/lapsule/mars/mars/maimai/t.log", "GTEST");

    _setCallBack(func);

    mars::stn::SetClientVersion(200);
    _createMars();
    _set_address_and_port("106.75.93.248", "", 8112);
    _set_short_link_port(8209);
    _report_event_on_forground(1);
    _makesure_longlink_connected();
    return 0;
}

int mars_destroy() {
  mars::baseevent::OnDestroy();
  appender_close();
  return 0;
}

// ----------------------------------------- //

namespace mars {
    namespace stn {
PyThreadState* main_tstate = NULL;

int createMars(py::object &func) {
  mars_init(func);
  main_tstate =  PyThreadState_Get();
  return 0;
}

int destroyMars() {
  mars_destroy();
  return 0;
}

int send_msg(const char* str_msg) {
  send_message(str_msg);
  return 0;
}

int set_push_callback(py::object &func);
py::object get_push_callback();



bool has_pybind11_internals_builtin() {
    auto builtins = py::handle(PyEval_GetBuiltins());
    return builtins.contains(PYBIND11_INTERNALS_ID);
};

bool has_pybind11_internals_static() {
    auto **&ipp = py::detail::get_internals_pp();
    return ipp && *ipp;
}


void* _push_cb_run(std::string str_py, py::object &func, 
                    uint64_t _channel_id, uint32_t _cmdid, 
                    uint32_t _taskid, const void* _body, 
                    const void* _extend) {

     /* Acquire GIL before calling Python code */
    py::gil_scoped_acquire acquire;

    if (func) {
        py::object _py_body_str = py::str((char*)_body);
        func(_py_body_str, _cmdid, _taskid, _channel_id);
    };

     /* Release GIL before calling into (potentially long-running) C++ code */
    py::gil_scoped_release release;
    return NULL; 
}

std::string _get_version() {
  return "stage1.4";
}

PYBIND11_MODULE(mars, m) {
  m.doc() = "Mars module."; 
  m.def("createMars", &createMars, "Init mars.");
  m.def("destroyMars", &destroyMars, "mars destroy.");
  m.def("sendMsg", &send_msg, "sending msg.");
  m.def("setPushCallback", &set_push_callback, "set push notify callback.");
  m.def("getPushCallback", &get_push_callback, "get push notify callback.");
  // m.def("__version__", &_get_version, "get mars' version");
  m.def("version",  &_get_version, "get mars' version");

}

    }
}