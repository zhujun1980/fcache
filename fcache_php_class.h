/* 
 * File:   fcache_php_class.h
 * Author: zhujun
 *
 * Created on 2014年8月5日, 上午10:18
 */

#ifndef FCACHE_PHP_CLASS_H
#define	FCACHE_PHP_CLASS_H

#ifdef	__cplusplus
extern "C" {
#endif

FCACHE_STARTUP_D(core);

FCACHE_SHUTDOWN_D(core);

FCACHE_DEACTIVATE_D(core);

FCACHE_INFO_D(core);

#ifdef	__cplusplus
}
#endif

#endif	/* FCACHE_PHP_CLASS_H */
