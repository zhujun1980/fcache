//
//  file_utils.cc
//  BloomFilter
//
//  Created by zhu jun on 14-7-29.
//  Copyright (c) 2014年 Weibo. All rights reserved.
//

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef	__cplusplus
}
#endif

#include "utils.h"
#include "file_utils.h"

FILE* OpenFile(const char* filepath, const char* mode) {
    FILE* result = NULL;
    do {
        result = fopen(filepath, mode);
    } while (NULL == result && errno == EINTR);
    return result;
}

bool ReadFromFD(int fd, char* buffer, size_t bytes) {
    size_t total_read = 0;
    while (total_read < bytes) {
        ssize_t bytes_read;
        NO_INTR(bytes_read = read(fd, buffer + total_read, bytes - total_read));
        if (bytes_read <= 0)
            break;
        total_read += bytes_read;
    }
    return total_read == bytes;
}

int WriteFile(const char* filepath, const char* data, int size) {
    int fd;
    NO_INTR(fd = creat(filepath, 0640));
    if(fd < 0)
        return -1;
    int bytes_written = WriteFD(fd, data, size);
    if(close(fd) < 0)
        return -1;
    return bytes_written;
}

int WriteFD(const int fd, const char* data, int size) {
    ssize_t total = 0;
    for (ssize_t part = 0; total < size; total += part) {
        NO_INTR(part = write(fd, data + total, size - total));
        if (part < 0)
            return -1;
    }
    return total;
}

bool CloseFile(FILE* file) {
    if (file == NULL) 
        return true;
    return fclose(file) == 0;
}

bool GetFileSize(const char *file, size_t &size) {
    struct stat st;
    int ret;
    
    NO_INTR(ret = stat(file, &st));
    if (ret < 0) {
        return false;
    } else {
        if (st.st_blocks == 0) {
            size = 0;
        } else {
            size = st.st_size;
        }
    }
    return true;
}

bool GetFileInfo(const char* file, struct stat* st) {
    int ret;
    
    NO_INTR(ret = stat(file, st));
    if (ret < 0) {
        return false;
    }
    return true;
}
