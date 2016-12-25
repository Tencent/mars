/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * DumpCrashStack.h
 *
 *  Created on: 2012-9-28
 *      Author: 叶润桂
 */

#ifndef DUMPCRASHSTACK_H_
#define DUMPCRASHSTACK_H_

#include <vector>
#include <string>

class DumpCrashStack {
  public:
    DumpCrashStack() {}
    ~DumpCrashStack() {}

    void Dump(const std::string& _processname);
    const std::vector<std::string>& StackList() const;

  private:
    std::vector<std::string> vecdump_;
};

#endif /* DUMPCRASHSTACK_H_ */
