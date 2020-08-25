#pragma once

#include <tuple>
#include <list>
#include <map>

#include "longlink.h"
#include "mars/comm/singleton.h"

namespace mars {

	class LongLinkConnectInfo {
	public:
		LongLinkConnectInfo() {}
		uint64_t connectTime = 0;   //unix timestamp
		int retType = 0;
		int netType = 0;
	};

//	enum DisconnectReason {
//		kUnknown = 1,
//		kRemoteDisconnect,
//		kRemoteError,
//		kNetworkChange,
//		kDecodeError,
//		kSessionTimeout,
//		kChangeIp,
//		kNoopTimeout,
//		kCgiTimeout,
//	};

	class LongLinkDisConnectInfo {
	public:
		LongLinkDisConnectInfo(){}
		uint64_t disconnectTime = 0;    //unix timestamp
		stn::LongLink::TDisconnectInternalCode retType = stn::LongLink::kNone;
		bool isForeground = false;
		uint32_t noopExpected = 0;      //noop span
		uint32_t dataSpan = 0;          //any send data span
	};

	enum ReconnectType {
		kReNormal = 1,
		kReActive,
		kReNetworkChange,
		kReBackNormal,
		kReTask,
	};

	class LongLinkSpanInfo {
	public:
		LongLinkSpanInfo(){}
		ReconnectType type = kReNormal;
		int32_t spanExpected = -1;
		int32_t connectSpan = -1;
	};

	typedef std::tuple<LongLinkConnectInfo, LongLinkDisConnectInfo, LongLinkSpanInfo> LongLinkLife;
	class LongLinkHistory {
	public:
		SINGLETON_INTRUSIVE(LongLinkHistory, new LongLinkHistory, delete);

		LongLinkConnectInfo& ConnectInfo(const std::string& _name) {
			ScopedLock lock(mutex_);
			auto life = Life(_name);
			if(life.size() == 0 || std::get<2>(life.back()).connectSpan != -1) {
				life.push_back(LongLinkLife());
			}
			return std::get<0>(life.back());
		}
		LongLinkDisConnectInfo& DisconnectInfo(const std::string& _name) {
			ScopedLock lock(mutex_);
			auto life = Life(_name);
			if(life.size() == 0 || std::get<0>(life.back()).connectTime == 0) {
				life.push_back(LongLinkLife());
			}
			return std::get<1>(life.back());
		}

		LongLinkSpanInfo& SpanInfo(const std::string& _name) {
			ScopedLock lock(mutex_);
			auto life = Life(_name);
			if(life.size() == 0 || std::get<0>(life.back()).connectTime == 0) {
				life.push_back(LongLinkLife());
			}

			_Trim(_name);
			return std::get<2>(life.back());
		}

		std::deque<LongLinkLife>& Life(const std::string& _name) {
			if(histories_.find(_name) == histories_.end()) {
				histories_[_name] = std::deque<LongLinkLife >();
			}

			return histories_[_name];
		}

		void Clear() {
			histories_.clear();
		}

	private:
		LongLinkHistory() {}

#define MAX_LIFE_TIME (12*60*60*1000)
		void _Trim(const std::string& _name) {
			auto info = Life(_name);
			if(info.size() < 200)   return;

			auto iter = info.begin();
			uint64_t now = timeMs();
			while(iter != info.end()) {
				auto item = std::get<0>(*iter);
				if((now - item.connectTime) < MAX_LIFE_TIME) {
					break;
				}

				iter = info.erase(iter);
			}
		}

	private:
		std::map<std::string, std::deque<LongLinkLife>> histories_;
		Mutex mutex_;
	};

}

