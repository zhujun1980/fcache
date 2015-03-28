/* 
 * File:   fc.h
 * Author: zhujun
 *
 * Created on 2014年8月15日, 下午3:11
 */

#ifndef FC_H
#define	FC_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <time.h>

#ifdef	__cplusplus
}
#endif

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "utils.h"
#include "murmur_hash3.h"
#include "file_mapping.h"
#include "log.h"

template<typename _Tp>
class FCache {
public:
    typedef uint32_t                                Key;
    typedef FileMapping<_Tp>                        Value;
    typedef typename Value::FileStatus              FileStatus;
    typedef std::map<Key, Value*>                   FileCache;
    typedef typename FileCache::const_iterator      FileCacheIter;

public:
    FCache() {
    }

    virtual ~FCache() {
    }

    const _Tp* GetMappingObject(const std::string& base, const std::string& dbname, const std::string& filename, const std::string& extname) {
        //base + db + filename as key
        std::string filepath(base);
        filepath.append(dbname);
        filepath.append(filename);
        uint32_t hv = Hash(filepath);
        FileCacheIter iter = _caches.find(hv);

        if(iter != _caches.end()) {
            //Found, check file's status
            FileStatus status = iter->second->CheckFileStatus();
            if(status == Value::UNAVAILABLE) {
                return NULL;
            }
            if(status == Value::UNCHANGED) {
                iter->second->atime = time(NULL);
                return iter->second->object;
            }
            //Changed, get file path
            filepath = iter->second->filepath;
            delete iter->second;
            _caches.erase(hv);
        }
        else {
            if(!GetFilePath(base, dbname, filename, extname, filepath)) {
                //TODO RETURN ERR TO CLIENT?
                //-1, ERR_INVALID_PATH
                DoLog(Logger::WARNING, "invalid filepath: %s (%s:%d)", filepath.c_str(), __FILE__, __LINE__);
                return NULL;
            }
        }

        FileMapping<_Tp>* mapping = FileMappingCreate<_Tp>(filepath);
        if(!mapping) {
            return NULL;
        }
        _caches[hv] = mapping;
        return mapping->object;
    }

    inline bool GetFilePath(const std::string& base, const std::string& dbname, const std::string& filename, const std::string& extname, std::string& filepath) {
        filepath.clear();
        filepath.append(base);
        filepath.append(1, '/');
        if(dbname.size() > 0) {
            filepath.append(dbname);
            filepath.append(1, '/');
        }
        filepath.append(filename);
        if(extname.size() > 0) {
            filepath.append(1, '.');
            filepath.append(extname);
        }
        std::string parent = "..";
        if(std::search(filepath.begin(), filepath.end(), parent.begin(), parent.end()) != filepath.end())
            return false;
        return true;
    }

    static void Clear() {
        for(FileCacheIter iter = _caches.begin(); iter != _caches.end(); iter++) {
            delete iter->second;
        }
        _caches.clear();
    }

    static void Purge(long unused_threshold) {
        std::vector<typename FileCache::key_type> deleted;
        for(FileCacheIter iter = _caches.begin(); iter != _caches.end(); iter++) {
            if(time(NULL) - unused_threshold > iter->second->atime) {
                delete iter->second;
                deleted.push_back(iter->first);
            }
        }
        for(typename std::vector<typename FileCache::key_type>::const_iterator iter = deleted.begin(); iter != deleted.end(); iter++) {
            _caches.erase(*iter);
        }
    }

private:
    uint32_t Hash(const std::string& kb) const {
        uint32_t index;
        MurmurHash3_x86_32(kb.c_str(), static_cast<int>(kb.size()), 0, &index);
        return index;
    }

    DISABLE_ASSIGN_AND_COPY(FCache);

    //HashMap Cache
    static FileCache _caches;
};

template<typename _Tp>
typename FCache<_Tp>::FileCache FCache<_Tp>::_caches;

#endif	/* FC_H */
