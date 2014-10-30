//
//  rand_utils.cc
//  SimpleIndex
//
//  Created by zhu jun on 14-7-29.
//  Copyright (c) 2014年 Weibo. All rights reserved.
//

#ifdef	__cplusplus
extern "C" {
#endif

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef	__cplusplus
}
#endif

#include "rand_utils.h"
#include "file_utils.h"

class URandomFd {
public:
    URandomFd() {
        rand_fd = open("/dev/urandom", O_RDONLY);
    }
    
    ~URandomFd() {
        close(rand_fd);
    }
    
    int fd() const {
        return rand_fd;
    }

private:
    int rand_fd;
};

URandomFd g_urandom_fd;

uint64_t RandUint64() {
    uint64_t rand;
    bool success = ReadFromFD(g_urandom_fd.fd(), reinterpret_cast<char*>(&rand), sizeof(rand));
    if(!success) {
        return 0L;
    }
    return rand;
}

uint32_t RandUint32() {
    uint32_t rand;
    bool success = ReadFromFD(g_urandom_fd.fd(), reinterpret_cast<char*>(&rand), sizeof(rand));
    if(!success) {
        return 0L;
    }
    return rand;
}
