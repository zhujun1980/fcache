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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef	__cplusplus
extern "C" {
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#ifdef	__cplusplus
}
#endif

#include "php_fcache.h"
#include "fcache_php_class.h"

ZEND_DECLARE_MODULE_GLOBALS(fcache)

/* {{{ fcache_functions[]
 *
 * Every user visible function must have an entry in fcache_functions[].
 */
const zend_function_entry fcache_functions[] = {
    PHP_FE_END	/* Must be the last line in fcache_functions[] */
};

zend_module_entry fcache_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "fcache",
    fcache_functions,
    PHP_MINIT(fcache),
    PHP_MSHUTDOWN(fcache),
    NULL, //PHP_RINIT(fcache),
    PHP_RSHUTDOWN(fcache),
    PHP_MINFO(fcache),
#if ZEND_MODULE_API_NO >= 20010901
    FCACHE_VERSION,
#endif
    PHP_MODULE_GLOBALS(fcache),
    NULL,
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_FCACHE
ZEND_GET_MODULE(fcache)
#endif

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("fcache.data_dir", "/usr/share", PHP_INI_ALL, OnUpdateString, data_dir, zend_fcache_globals, fcache_globals)
    STD_PHP_INI_ENTRY("fcache.purge_period", "43200", PHP_INI_ALL, OnUpdateLong, purge_period, zend_fcache_globals, fcache_globals)
    STD_PHP_INI_ENTRY("fcache.unused_threshold", "86400", PHP_INI_ALL, OnUpdateLong, unused_threshold, zend_fcache_globals, fcache_globals)
PHP_INI_END()

static inline void fcache_globals_ctor(zend_fcache_globals *fcache_globals_p TSRMLS_DC) {
    FCACHE_G(data_dir) = NULL;
    FCACHE_G(purge_period) = 0;
    FCACHE_G(unused_threshold) = 0;
}

static inline void fcache_globals_dtor(zend_fcache_globals *fcache_globals_p TSRMLS_DC) {
}

PHP_MINIT_FUNCTION(fcache) {
    fcache_globals_ctor(&fcache_globals TSRMLS_CC);
    REGISTER_INI_ENTRIES();
    FCACHE_STARTUP(core);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(fcache) {
    FCACHE_SHUTDOWN(core);
    UNREGISTER_INI_ENTRIES();
    fcache_globals_dtor(&fcache_globals TSRMLS_CC);
    return SUCCESS;
}

PHP_RINIT_FUNCTION(fcache) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(fcache) {
    FCACHE_DEACTIVATE(core);
    return SUCCESS;
}

PHP_MINFO_FUNCTION(fcache) {
    php_info_print_table_start();
    php_info_print_table_header(2, "fcache support", "enabled");
    php_info_print_table_row(2, "version", FCACHE_VERSION);
    php_info_print_table_end();
    FCACHE_INFO(core);
    DISPLAY_INI_ENTRIES();
}
