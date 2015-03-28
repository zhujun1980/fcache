/* 
 * File:   fcache_dict.h
 * Author: zhujun
 *
 * Created on 2014年9月15日, 上午10:18
 */

#ifndef FCACHE_DICT_H
#define	FCACHE_DICT_H

#ifdef	__cplusplus
extern "C" {
#endif

#define FIXED_ARRAY_N   "FixedArray"

extern zend_class_entry *fcache_fixed_array_ce;

FCACHE_STARTUP_D(dict);

FCACHE_SHUTDOWN_D(dict);

FCACHE_DEACTIVATE_D(dict);

FCACHE_INFO_D(dict);

#ifdef	__cplusplus
}
#endif

#endif	/* FCACHE_DICT_H */
