#ifndef DEFS_H
#define DEFS_H

#include "master.h"

struct Arg {
    int left, right;
    int *array;
    WaitGroup *wg;
};

#endif