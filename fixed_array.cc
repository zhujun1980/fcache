/* 
 * File:   fixed_array.cc
 * Author: zhujun
 * 
 * Created on 2014年9月29日, 下午3:59
 */

#include <string>
#include "fcache/consts.h"
#include "fcache/file_utils.h"
#include "fcache/log.h"
#include "fixed_array.h"
#include "deep_copy.h"

const char* FixedArray::EXT_NAME = "pa";

FixedArray::FixedArray(zval* v, size_t s, int loc) : value(v), size(s), data_loc(loc) {
}

FixedArray::~FixedArray() {
    if(data_loc == DATA_LOC_HEAP)
        free_copy(value);
}

void FixedArray::CopyZVal(zval *src, zval **dst) const {
    MAKE_STD_ZVAL(*dst);

    **dst = *src;
    switch(Z_TYPE_P(src)) {
        case IS_CONSTANT:
        case IS_OBJECT:
        case IS_STRING:
            ZVAL_STRINGL(*dst, REALADDR(char*, value, Z_STRVAL_P(src)), Z_STRLEN_P(src), 1);
            break;
        case IS_ARRAY:
        case IS_CONSTANT_ARRAY:
        case IS_RESOURCE:
            efree(*dst);
            *dst = NULL;
            break;
        default:
            break;
    }
}

const Bucket* FixedArray::Head() const {
    HashTable *ht;

    ht = REALADDR(HashTable*, value, Z_ARRVAL_P(value));
    return REALADDR(Bucket*, value, ht->pListHead);
}

const Bucket* FixedArray::Tail() const {
    HashTable *ht;

    ht = REALADDR(HashTable*, value, Z_ARRVAL_P(value));
    return REALADDR(Bucket*, value, ht->pListTail);
}

const Bucket* FixedArray::NextBucket(const Bucket *current) const {
    return REALADDR(Bucket*, value, current->pListNext);
}

const Bucket* FixedArray::PrevBucket(const Bucket *current) const {
    return REALADDR(Bucket*, value, current->pListLast);
}

bool FixedArray::Key(const Bucket *current, zval **k) const {
    const char *key;

    if(!current) {
        *k = NULL;
        return false;
    }

    MAKE_STD_ZVAL(*k);
    if(current->nKeyLength == 0) {
        ZVAL_LONG(*k, current->h);
    }
    else {
        key = REALADDR(const char*, value, current->arKey);
        ZVAL_STRINGL(*k, key, current->nKeyLength, 1);
    }
    return true;
}

bool FixedArray::Current(const Bucket *current, zval **v) const {
    if(!current) {
        *v = NULL;
        return false;
    }

    CopyZVal(REALADDR(zval*, value, current->pDataPtr), v);
    if(*v == NULL)
        return false;
    return true;
}

bool FixedArray::Get(const std::string& offset, zval **v) const {
    ulong h;
    uint nIndex;
    Bucket *p;
    HashTable *ht;
    Bucket **bs;
    const char *key;

    ht = REALADDR(HashTable*, value, Z_ARRVAL_P(value));
    h = zend_inline_hash_func(offset.c_str(), offset.size());
    nIndex = h & ht->nTableMask;

    bs = REALADDR(Bucket**, value, ht->arBuckets);
    p = REALADDR(Bucket*, value, bs[nIndex]);
    while (p != NULL) {
        key = REALADDR(const char*, value, p->arKey);
        if ((p->h == h) && (p->nKeyLength == offset.size()) && !memcmp(key, offset.c_str(), offset.size())) {
            CopyZVal(REALADDR(zval*, value, p->pDataPtr), v);
            if(*v == NULL)
                return false;
            return true;
        }
        p = REALADDR(Bucket*, value, p->pNext);
    }
    return false;
}

bool FixedArray::Get(ulong h, zval **v) const {
    Bucket *p;
    Bucket **bs;
    uint nIndex;
    HashTable *ht;

    ht = REALADDR(HashTable*, value, Z_ARRVAL_P(value));
    nIndex = h & ht->nTableMask;
    bs = REALADDR(Bucket**, value, ht->arBuckets);

    p = REALADDR(Bucket*, value, bs[nIndex]);
    while (p != NULL) {
        if ((p->h == h) && (p->nKeyLength == 0)) {
            CopyZVal(REALADDR(zval*, value, p->pDataPtr), v);
            if(*v == NULL)
                return false;
            return true;
        }
        p = REALADDR(Bucket*, value, p->pNext);
    }
    return false;
}

long FixedArray::Count() const {
    HashTable *ht;

    ht = REALADDR(HashTable*, value, Z_ARRVAL_P(value));
    return ht->nNumOfElements;
}

/**
 * Save array data to file.
 * 
 * [data_type 8 bytes]
 * [memory_align 8 bytes]
 * [data_size 8 bytes]
 * [data......]
 * 
 * @param filepath
 */
bool FixedArray::Save(const std::string& filepath) const {
    FILE* idx = OpenFile(filepath.c_str(), "w+");
    if(idx == NULL) {
        DoLog(Logger::ERROR, "Open FixedArray file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        return false;
    }
    int fd = fileno(idx);
    int ret;
    uint64_t sec;

    sec = FIXED_ARRAY;
    ret = WriteFD(fd, reinterpret_cast<const char*>(&sec), sizeof(sec));
    if(ret < 0) {
        DoLog(Logger::ERROR, "write FixedArray file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return false;
    }

    sec = FCACHE_ALIGNMENT;
    ret = WriteFD(fd, reinterpret_cast<const char*>(&sec), sizeof(sec));
    if(ret < 0) {
        DoLog(Logger::ERROR, "write FixedArray file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return false;
    }

    ret = WriteFD(fd, reinterpret_cast<const char*>(&size), sizeof(size));
    if(ret < 0) {
        DoLog(Logger::ERROR, "write FixedArray file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return false;
    }

    ret = WriteFD(fd, reinterpret_cast<const char*>(value), size);
    if(ret < 0) {
        DoLog(Logger::ERROR, "write FixedArray file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return false;
    }

    CloseFile(idx);
    return true;
}

//static 
//Load object from array
FixedArray* FixedArray::Load(zval *value) {
    zval* z;

    z = NULL;
    size_t size = deep_copy(value, &z);
    if(size == 0 || z == NULL) {
        return NULL;
    }
    fixup_value(z);
    return new FixedArray(z, size, DATA_LOC_HEAP);
}

//Load object from shared memory
FixedArray* FixedArray::Load(void* addr, size_t size) {
    uint64_t *sec, zval_size;

    if(size == 0) {
        DoLog(Logger::WARNING, "size equal zero (%s:%d)", __FILE__, __LINE__);
        return NULL;
    }
    sec = reinterpret_cast<uint64_t*>(addr);
    if(*sec++ != FIXED_ARRAY) {
        DoLog(Logger::WARNING, "file invalid (%s:%d)", __FILE__, __LINE__);
        return NULL;
    }
    sec++; //skip memory align area
    zval_size = *sec++;
    zval *value = reinterpret_cast<zval*>(sec);
    return new FixedArray(value, zval_size, DATA_LOC_SHARED);
}
