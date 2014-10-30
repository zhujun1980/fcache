/* 
 * File:   shmm_posix.cc
 * Author: zhujun
 * 
 * Created on 2014年8月6日, 下午2:21
 */

#ifdef	__cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef	__cplusplus
}
#endif

#include "shmm.h"

void* CreateMmap(const char *file, size_t segsize) {
    mode_t mode = umask(0);
    int fd = open(file, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd == -1) {
        perror("open");
        return NULL;
    }
    umask(mode);
    if (lseek(fd, segsize, SEEK_SET) == -1) {
        perror("lseek");
        return NULL;
    }

    if (write(fd, "1", 1) == -1) {
        perror("write");
        return NULL;
    }
    void *addr = mmap(NULL, segsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return addr;
}

void* MMap(const char *file, size_t segsize, int fg) {
    int flags = fg;
    int prot = 0x0;
    switch (fg) {
        case O_RDONLY:
            prot = PROT_READ;
            break;
        case O_WRONLY:
            prot = PROT_WRITE;
            break;
        case O_RDWR:
            prot = PROT_READ | PROT_WRITE;
            break;
        default:
            return NULL;
    }

    int fd = open(file, flags);
    if (fd == -1) { // 文件不存在
        return NULL;
    }

    struct stat buffer;
    if (fstat(fd, &buffer) == -1) {// 无法获得文件信息
        return NULL;
    }
    if (buffer.st_size == 0) {// 文件0字节
        return NULL;
    }

    void *addr = mmap(NULL, segsize, prot, MAP_SHARED, fd, 0);
    close(fd);
    return addr;
}

bool MUnMap(void *addr, size_t segsize) {
    return munmap(static_cast<char*>(addr), segsize) == 0 ? true : false;
}
