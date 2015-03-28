/* 
 * File:   consts.h
 * Author: zhujun
 *
 * Created on 2014年8月12日, 上午6:57
 */

#ifndef CONSTS_H
#define	CONSTS_H

enum CACHE_DATA_FORMAT_TYPE {
    INVALID_TYPE = 0,
    BLOOM_FILTER,
    FIXED_ARRAY,
    UNKNOWN_TYPE
};

#define SHM_IPC_KEYS    "/tmp/fcache"

#define MAX_HASH_COUNT  256

#define DATA_LOC_HEAP   1
#define DATA_LOC_SHARED 2
#endif	/* CONSTS_H */
