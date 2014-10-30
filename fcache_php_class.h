/* 
 * File:   fcache_php_class.h
 * Author: zhujun
 *
 * Created on 2014年8月5日, 上午10:18
 */

#ifndef FCACHE_H
#define	FCACHE_H

#ifdef	__cplusplus
extern "C" {
#endif

#define FCACHE_BLOOM_FILTER_N "FCacheBloomFilter"
    
#define BLOOM_FILTER_STAT_VER   "version"
#define BLOOM_FILTER_STAT_HC    "hash_cnt"
#define BLOOM_FILTER_STAT_BITS  "bit_size"
#define BLOOM_FILTER_STAT_BYTES "byte_size"

extern zend_class_entry *fcache_bloom_filter_cls;

PHP_MINIT_FUNCTION(fcache_core);

PHP_MSHUTDOWN_FUNCTION(fcache_core);

PHP_RSHUTDOWN_FUNCTION(fcache_core);

PHP_MINFO_FUNCTION(fcache_core);

#ifdef	__cplusplus
}
#endif

#endif	/* FCACHE_H */
