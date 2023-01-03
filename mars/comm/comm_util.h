//
// Created by Changpeng Pan on 2022/9/9.
//

#ifndef MMNET_COMM_UTIL_H
#define MMNET_COMM_UTIL_H

template <typename T>
void safe_delete(T*& p) {
    if (p) {
        delete p;
        p = 0;
    }
}

#endif  // MMNET_COMM_UTIL_H
