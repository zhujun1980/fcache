/* 
 * File:   php_logger.h
 * Author: zhujun
 *
 * Created on 2014年9月16日, 上午10:29
 */

#ifndef PHP_LOGGER_H
#define	PHP_LOGGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdarg.h>    
#include "php.h"
#include "Zend/zend.h"

#ifdef	__cplusplus
}
#endif

#include "fcache/log.h"

class PHPLogger : public Logger {
    const int MAXLINE;
#ifdef ZTS
    TSRMLS_D;
#endif

public:
    PHPLogger(TSRMLS_D) : MAXLINE(256) {
#ifdef ZTS
        this->tsrm_ls = tsrm_ls;
#endif
    }

    virtual ~PHPLogger() {
    }

    virtual void operator()(int type, const char *format, ...) {
        char buf[MAXLINE + 1];
        type = 1 << type;   //convert error level to php level

        va_list	ap;
        va_start(ap, format);
	vsnprintf(buf, MAXLINE, format, ap);	/* safe */
#ifdef ZTS
	php_error_docref(NULL, this->tsrm_ls, type, "%s", buf);
#else
	php_error_docref(NULL, type, "%s", buf);
#endif
        va_end(ap);
    }
};

#endif	/* PHP_LOGGER_H */
