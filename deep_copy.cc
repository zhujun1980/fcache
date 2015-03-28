/* 
 * File:   deep_copy.cc
 * Author: zhujun
 * 
 * Created on 2014年10月08日, 上午09:30
 */

#ifdef	__cplusplus
extern "C" {
#endif

#include "php.h"
#include "Zend/zend.h"

#undef NDEBUG
#include <assert.h>

#ifdef	__cplusplus
}
#endif

#include "deep_copy.h"
#include "fcache/utils.h"

struct MemBuffer {
    u_char *first;
    u_char *end;
    u_char *cur;

    MemBuffer(u_char *buf, size_t m) : first(buf), end(buf + m), cur(buf) {
    }

    inline void* Alloc(size_t size) {
        void *r = NULL;

        if(cur + size <= end) {
            r = cur;
            cur += size;
            fcache_align_ptr(cur, FCACHE_ALIGNMENT);
        }
        assert(r);
        return r;
    }
};

void* fixup_array(void *base, HashTable* ht);
void* fixup_zval(void *base, zval *value);

static inline size_t add_size(size_t len, size_t num);
static size_t calc_array(HashTable* ht);
static size_t calc_zval(zval* value);

static void copy_array(HashTable* src, HashTable** dst, MemBuffer& buf);
static void copy_zval(zval* src, zval** dst, MemBuffer& buf);

static inline size_t add_size(size_t len, size_t num) {
    len += num;
    fcache_align(len, FCACHE_ALIGNMENT);
    return len;
}

static size_t calc_array(HashTable* ht) {
    size_t len;
    Bucket* p;
    zval* data;

    len = sizeof(HashTable);
    len = add_size(len, ht->nTableSize * sizeof(Bucket*));

    p = ht->pInternalPointer;
    while(p != NULL) {
        if(p->nKeyLength) {
            len = add_size(len, sizeof(Bucket) + p->nKeyLength);
        }
        else {
            len = add_size(len, sizeof(Bucket));
        }

        void* zdata;
        if(p->pData == &p->pDataPtr)
            zdata = p->pDataPtr;
        else
            zdata = p->pData;

        data = reinterpret_cast<zval*>(zdata);
        if(!data) {
            return 0;
        }
        len = add_size(len, calc_zval(data));
        p = p->pListNext;
    }
    return len;
}

static size_t calc_zval(zval* value) {
    size_t len;

    len = sizeof(zval);
    switch(Z_TYPE_P(value)) {
        case IS_CONSTANT:
        case IS_OBJECT:
        case IS_STRING:
            //calc_string
            len = add_size(len, value->value.str.len);
            break;
        case IS_ARRAY:
        case IS_CONSTANT_ARRAY:
            //calc_array_size
            len = add_size(len, calc_array(value->value.ht));
            break;
        case IS_RESOURCE:
            //error report
            break;
        default:
            break;
    }
    return len;
}

static void copy_array(HashTable* src, HashTable** dst, MemBuffer& buf) {
    zend_bool persistent;
    HashTable* ht;
    Bucket* p, *q;
    zval* data;
    uint nIndex;

    persistent = 1;
    *dst = (HashTable*) buf.Alloc(sizeof(HashTable));
    if(*dst == NULL) {
        return;
    }
    **dst = *src;
    ht = *dst;

    _zend_hash_init(ht, src->nNumOfElements, NULL, ZVAL_PTR_DTOR, persistent ZEND_FILE_LINE_RELAY_CC);
    ht->arBuckets = (Bucket **) buf.Alloc(ht->nTableSize * sizeof(Bucket*));
    ht->nTableMask = ht->nTableSize - 1;
    memset(ht->arBuckets, 0, ht->nTableSize * sizeof(Bucket*));

    q = src->pInternalPointer;
    while(q != NULL) {
        if(q->nKeyLength) {
            p = (Bucket*) buf.Alloc(sizeof(Bucket) + q->nKeyLength);
            p->arKey = (const char*)(p + 1);
            memcpy(const_cast<char*>(p->arKey), q->arKey, q->nKeyLength);
        }
        else {
            p = (Bucket*) buf.Alloc(sizeof(Bucket));
            p->arKey = NULL;
        }
        p->nKeyLength = q->nKeyLength;
        p->h = q->h;
        nIndex = p->h & ht->nTableMask;

        void* zdata;
        zval** dst1;
        if(q->pData == &q->pDataPtr) {
            zdata = q->pDataPtr;
            dst1 = reinterpret_cast<zval**>(&p->pDataPtr);
            p->pData = &p->pDataPtr;
        }
        else {
            zdata = q->pData;
            dst1 = reinterpret_cast<zval**>(&p->pData);
            p->pDataPtr = NULL;
        }

        data = reinterpret_cast<zval*>(zdata);
        if(!data) {
            //Free MemBuffer;
            return;
        }
        copy_zval(data, dst1, buf);

        p->pNext = ht->arBuckets[nIndex];
        p->pLast = NULL;
        if(p->pNext) {
            p->pNext->pLast = p;
        }

		p->pListLast = ht->pListTail;
		ht->pListTail = p;
		p->pListNext = NULL;
		if(p->pListLast != NULL) {
			p->pListLast->pListNext = p;
		}
		if(!ht->pListHead) {
			ht->pListHead = p;
		}
		if(ht->pInternalPointer == NULL) {
			ht->pInternalPointer = p;
		}

        ht->arBuckets[nIndex] = p;
        ht->nNumOfElements++;
        ht->pDestructor = src->pDestructor;

        q = q->pListNext;
    }
}

static void copy_zval(zval* src, zval** dst, MemBuffer& buf) {
    *dst = (zval*)buf.Alloc(sizeof(zval));
    if(*dst == NULL) {
        //error report
        return;
    }
    **dst = *src;
    switch(Z_TYPE_P(src)) {
        case IS_CONSTANT:
        case IS_OBJECT:
        case IS_STRING:
            Z_STRLEN_P(*dst) = Z_STRLEN_P(src);
            Z_STRVAL_P(*dst) = (char*)buf.Alloc(Z_STRLEN_P(src));
            memcpy(Z_STRVAL_P(*dst), Z_STRVAL_P(src), Z_STRLEN_P(src));
            break;
        case IS_ARRAY:
        case IS_CONSTANT_ARRAY:
            copy_array(Z_ARRVAL_P(src), &(Z_ARRVAL_P(*dst)), buf);
            break;
        case IS_RESOURCE:
            //error report
            break;
        default:
            break;
    }
}

void* fixup_array(void *base, HashTable* ht) {
    void *q;
    Bucket *p;
    zval *data;
    uint num;

    q = ht + 1;
    fcache_align_ptr(q, FCACHE_ALIGNMENT);
    ht->arBuckets = reinterpret_cast<Bucket**>(q);
    q = ht->arBuckets + ht->nTableSize;

    for(Bucket **bucketHead = ht->arBuckets; bucketHead != q; ++bucketHead) {
        *bucketHead = FIXUP(Bucket*, *bucketHead, base);
    }
    ht->arBuckets = FIXUP(Bucket**, ht->arBuckets, base);
    ht->pListHead = FIXUP(Bucket*, ht->pListHead, base);
    ht->pListTail = FIXUP(Bucket*, ht->pListTail, base);
    ht->pInternalPointer = NULL;

    fcache_align_ptr(q, FCACHE_ALIGNMENT);
    num = ht->nNumOfElements;
    while(num > 0) {
        p = reinterpret_cast<Bucket*>(q);
        if(p->nKeyLength) {
            p->arKey = FIXUP(const char*, p + 1, base);
            q = reinterpret_cast<char*>(p + 1) + p->nKeyLength;
        }
        else {
            p->arKey = NULL;
            q = p + 1;
        }
        fcache_align_ptr(q, FCACHE_ALIGNMENT);

        if(p->pData == &p->pDataPtr) {
            p->pData = FIXUP(void*, p->pData, base);
            p->pDataPtr = FIXUP(void*, p->pDataPtr, base);
        }
        else {
            p->pData = FIXUP(void*, p->pData, base);
        }

        data = reinterpret_cast<zval*>(q);
        q = fixup_zval(base, data);

        p->pNext = FIXUP(Bucket*, p->pNext, base);
        p->pLast = FIXUP(Bucket*, p->pLast, base);
        p->pListLast = FIXUP(Bucket*, p->pListLast, base);
        p->pListNext = FIXUP(Bucket*, p->pListNext, base);

        ht->pDestructor = NULL;
        num--;
    }
    return q;
}

void* fixup_zval(void *base, zval *value) {
    void* z;

    z = value + 1;
    fcache_align_ptr(z, FCACHE_ALIGNMENT);
    switch(Z_TYPE_P(value)) {
        case IS_CONSTANT:
        case IS_OBJECT:
        case IS_STRING:
            Z_STRVAL_P(value) = reinterpret_cast<char*>(z);
            z = Z_STRVAL_P(value) + Z_STRLEN_P(value);
            fcache_align_ptr(z, FCACHE_ALIGNMENT);
            Z_STRVAL_P(value) = FIXUP(char*, Z_STRVAL_P(value), base);
            break;
        case IS_ARRAY:
        case IS_CONSTANT_ARRAY:
            Z_ARRVAL_P(value) = reinterpret_cast<HashTable*>(z);
            z = fixup_array(base, Z_ARRVAL_P(value));
            fcache_align_ptr(z, FCACHE_ALIGNMENT);
            Z_ARRVAL_P(value) = FIXUP(HashTable*, Z_ARRVAL_P(value), base);
            break;
        case IS_RESOURCE:
            //error report
            break;
        default:
            break;
    }
    return z;
}

size_t calc_size(zval *value) {
    return calc_zval(value);
}

size_t deep_copy(zval *src, zval **dst) {
    size_t size;

    size = calc_size(src);
    if(size == 0) {
        return 0;
    }

    MemBuffer buf(static_cast<u_char*>(pemalloc(size, 1)), size);
    copy_zval(src, dst, buf);
    return size;
}

void free_copy(zval *value) {
    pefree(value, 1);
}

void fixup_value(zval *value) {
    fixup_zval(value, value);
}
