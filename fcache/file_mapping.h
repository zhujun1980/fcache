/* 
 * File:   file_mapping.h
 * Author: zhujun
 *
 * Created on 2014年8月18日, 上午11:08
 */

#ifndef FILE_MAPPING_H
#define	FILE_MAPPING_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef	__cplusplus
}
#endif

#include <string>
#include "file_utils.h"
#include "shmm.h"
#include "log.h"

static inline size_t GetFileSize(const struct stat& st) {
    size_t filesize;

    if (st.st_blocks == 0) {
        filesize = 0;
    } else {
        filesize = st.st_size;
    }
    return filesize;
}

template<typename _Tp>
struct FileMapping {
    enum FileStatus {
        INVALID = -1,
        UNAVAILABLE = 0,
        CHANGED,
        UNCHANGED,
    };

    void* attachAddr;
    _Tp*  object;
    size_t filesize;
    time_t mtime;
    time_t atime;
    std::string filepath;

    FileMapping(void* _attachAddr, _Tp* _object, size_t _filesize, time_t _mtime, time_t _atime, const std::string _filepath) :
                attachAddr(_attachAddr), object(_object), filesize(_filesize), mtime(_mtime), atime(_atime), filepath(_filepath) {
    }

    ~FileMapping() {
        delete object;
        object = NULL;
        if(!MUnMap(attachAddr, filesize)) {
            DoLog(Logger::WARNING, "unmap file failed (%s:%d)", __FILE__, __LINE__);
        }
        attachAddr = NULL;
    }

    FileStatus CheckFileStatus() {
        struct stat st;
        size_t _filesize;

        if(!GetFileInfo(filepath.c_str(), &st)) {
            DoLog(Logger::WARNING, "get file info failed %s (%s:%d)", filepath.c_str(), __FILE__, __LINE__);
            return UNAVAILABLE;
        }
        _filesize = GetFileSize(st);
        if(_filesize == 0) {
            DoLog(Logger::WARNING, "file size equal zero %s (%s:%d)", filepath.c_str(), __FILE__, __LINE__);
            return UNAVAILABLE;
        }

        if(st.st_mtime == mtime && filesize == _filesize) {
            return UNCHANGED;
        }
        return CHANGED;
    }
};

template<typename _Tp>
FileMapping<_Tp>* FileMappingCreate(const std::string& filepath) {
    struct stat st;
    size_t filesize;

    if(!GetFileInfo(filepath.c_str(), &st)) {
        DoLog(Logger::WARNING, "get file info failed %s (%s:%d)", filepath.c_str(), __FILE__, __LINE__);
	return NULL;
    }
    filesize = GetFileSize(st);
    if(filesize == 0) {
        DoLog(Logger::WARNING, "file size equal zero %s (%s:%d)", filepath.c_str(), __FILE__, __LINE__);
        return NULL;
    }

    void* addr = MMap(filepath.c_str(), filesize, O_RDONLY);
    if(!addr) {
        DoLog(Logger::WARNING, "mmap file failed %s (%s:%d)", filepath.c_str(), __FILE__, __LINE__);
	return NULL;
    }
    _Tp* obj = _Tp::Load(addr, filesize);
    if(!obj) {
        DoLog(Logger::WARNING, "create object failed %s (%s:%d)", filepath.c_str(), __FILE__, __LINE__);
        if(!MUnMap(addr, filesize)) {
            DoLog(Logger::WARNING, "unmap file failed (%s:%d)", __FILE__, __LINE__);
        }
        return NULL;
    }

    FileMapping<_Tp>* mapping = new(std::nothrow) FileMapping<_Tp>(addr, obj, filesize, st.st_mtime, time(NULL), filepath);
    if(!mapping) {
        delete obj;
        if(!MUnMap(addr, filesize)) {
            DoLog(Logger::WARNING, "unmap file failed (%s:%d)", __FILE__, __LINE__);
        }
        return NULL;
    }
    return mapping;
}

#endif	/* FILE_MAPPING_H */
