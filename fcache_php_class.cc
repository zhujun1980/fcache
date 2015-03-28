/* 
 * File:   fcache_php_class.cc
 * Author: zhujun
 *
 * Created on 2014年8月5日, 上午10:18
 */

#ifdef	__cplusplus
extern "C" {
#endif

#include <time.h>
#include "php.h"
#include "Zend/zend.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#ifdef	__cplusplus
}
#endif

#include "php_fcache.h"
#include "fcache_php_class.h"
#include "fcache_set.h"
#include "fcache_dict.h"
#include "php_logger.h"

Logger* logger_global = NULL;

static time_t last_purge_time;

FCACHE_STARTUP_D(core) {
    logger_global = new PHPLogger(TSRMLS_C);
    last_purge_time = time(NULL);
    FCACHE_STARTUP(set);
    FCACHE_STARTUP(dict);
    return SUCCESS;
}

FCACHE_DEACTIVATE_D(core) {
    time_t cur = time(NULL);
    if (FCACHE_G(purge_period) == 0 || cur - last_purge_time > FCACHE_G(purge_period)) {
        last_purge_time = cur;
        FCACHE_DEACTIVATE(set);
        FCACHE_DEACTIVATE(dict);
    }
    return SUCCESS;
}

FCACHE_INFO_D(core) {
    FCACHE_INFO(set);
    FCACHE_INFO(dict);
}

FCACHE_SHUTDOWN_D(core) {
    FCACHE_SHUTDOWN(set);
    FCACHE_SHUTDOWN(dict);
    delete logger_global;
    return SUCCESS;
}
