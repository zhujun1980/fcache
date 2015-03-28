/* 
 * File:   fcache_set.h
 * Author: zhujun
 *
 * Created on 2014年9月15日, 上午10:18
 */

#ifndef FCACHE_SET_H
#define	FCACHE_SET_H

#ifdef	__cplusplus
extern "C" {
#endif

#define FCACHE_BLOOM_FILTER_N   "FCacheBloomFilter"

#define BLOOM_FILTER_STAT_VER   "version"
#define BLOOM_FILTER_STAT_HC    "hash_cnt"
#define BLOOM_FILTER_STAT_BITS  "bit_size"
#define BLOOM_FILTER_STAT_BYTES "byte_size"

extern zend_class_entry *fcache_bloom_filter_cls;

FCACHE_STARTUP_D(set);

FCACHE_SHUTDOWN_D(set);

FCACHE_DEACTIVATE_D(set);

FCACHE_INFO_D(set);

#ifdef	__cplusplus
}
#endif

#endif	/* FCACHE_SET_H */
