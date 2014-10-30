//
//  time_utils.cc
//  BloomFilter
//
//  Created by zhu jun on 14-8-20.
//  Copyright (c) 2014年 Weibo. All rights reserved.
//

#include "time_utils.h"

double Now() {
    struct timeval tp = {0};
    if (!gettimeofday(&tp, NULL)) {
        return (double)(tp.tv_sec + tp.tv_usec / 1000000.00);;
    } else {
        return (double)time(0);
    }
}
