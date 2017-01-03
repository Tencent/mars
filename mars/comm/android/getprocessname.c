/*
 * getprocessname.c
 *
 *  Created on: 2012-9-28
 *      Author: 叶润桂
 */


#ifdef ANDROID

#include <stdio.h>
#include <unistd.h>
#include "assert/__assert.h"

const char* getprocessname() {
    static char data[1024];
    static char *x = 0;
    FILE *fp = 0;

    if (x) return x;

    sprintf(data, "/proc/%d/cmdline", getpid());
    fp = fopen(data, "r");
    if (fp) {
        x = fgets(data, 1024, fp);
        fclose(fp);
    }

   ASSERT(x);
   return x ? x : "UNKNOWN";
}
#endif
