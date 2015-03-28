/* 
 * File:   fixed_array.h
 * Author: zhujun
 *
 * Created on 2014年9月29日, 下午3:59
 */

#ifndef FIXED_ARRAY_H
#define	FIXED_ARRAY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "php.h"
#include "Zend/zend.h"

#ifdef	__cplusplus
}
#endif

#include <memory>
#include "fcache/utils.h"

class FixedArray {
public:
    static const char *EXT_NAME;

public:
    FixedArray(zval *v, size_t s, int loc);
    virtual ~FixedArray();

    static FixedArray* Load(zval *value);
    static FixedArray* Load(void *addr, size_t size);

    bool Save(const std::string& filepath) const;
    zval* Clone();

    long Count() const;

    bool Get(const std::string& offset, zval **v) const;
    bool Get(ulong h, zval **v) const;

    void CopyZVal(zval *src, zval **dst) const;
    
    int DataLoc() const {
        return data_loc;
    }

public:    
    const Bucket* Head() const;
    const Bucket* Tail() const;
    const Bucket* NextBucket(const Bucket *current) const;
    const Bucket* PrevBucket(const Bucket *current) const;
    bool  Key(const Bucket *current, zval **k) const;
    bool  Current(const Bucket *current, zval **v) const;

private:
    DISABLE_ASSIGN_AND_COPY(FixedArray);

    zval    *value;
    size_t  size;
    int     data_loc;
};

#endif	/* FIXED_ARRAY_H */
