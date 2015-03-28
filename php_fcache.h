/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_FCACHE_H
#define PHP_FCACHE_H

#ifdef	__cplusplus
extern "C" {
#endif

extern zend_module_entry fcache_module_entry;
#define phpext_fcache_ptr &fcache_module_entry

#ifdef PHP_WIN32
#	define PHP_FCACHE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_FCACHE_API __attribute__ ((visibility("default")))
#else
#	define PHP_FCACHE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define FCACHE_VERSION  "1.0.3"

ZEND_BEGIN_MODULE_GLOBALS(fcache)
    char* data_dir;
    long purge_period;
    long unused_threshold;
ZEND_END_MODULE_GLOBALS(fcache)

PHP_MINIT_FUNCTION(fcache);
PHP_MSHUTDOWN_FUNCTION(fcache);
PHP_RINIT_FUNCTION(fcache);
PHP_RSHUTDOWN_FUNCTION(fcache);
PHP_MINFO_FUNCTION(fcache);

#define FCACHE_STARTUP_D(module)      ZEND_MODULE_STARTUP_D(fcache_##module)
#define FCACHE_SHUTDOWN_D(module)     ZEND_MODULE_SHUTDOWN_D(fcache_##module)
#define FCACHE_ACTIVATE_D(module)     ZEND_MODULE_ACTIVATE_D(fcache_##module)
#define FCACHE_DEACTIVATE_D(module)   ZEND_MODULE_DEACTIVATE_D(fcache_##module)
#define FCACHE_INFO_D(module)         ZEND_MODULE_INFO_D(fcache_##module)

#define FCACHE_STARTUP(module)        ZEND_MODULE_STARTUP_N(fcache_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define FCACHE_SHUTDOWN(module)       ZEND_MODULE_SHUTDOWN_N(fcache_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)
#define FCACHE_ACTIVATE(module)       ZEND_MODULE_ACTIVATE_N(fcache_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define FCACHE_DEACTIVATE(module)     ZEND_MODULE_DEACTIVATE_N(fcache_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)
#define FCACHE_INFO(module)           ZEND_MODULE_INFO_N(fcache_##module)(ZEND_MODULE_INFO_FUNC_ARGS_PASSTHRU)

ZEND_EXTERN_MODULE_GLOBALS(fcache)

/* In every utility function you add that needs to use variables 
   in php_fcache_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as FCACHE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define FCACHE_G(v) TSRMG(fcache_globals_id, zend_fcache_globals *, v)
#else
#define FCACHE_G(v) (fcache_globals.v)
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* PHP_FCACHE_H */
