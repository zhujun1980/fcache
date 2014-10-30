/* 
 * File:   log.h
 * Author: zhujun
 *
 * Created on 2014年8月18日, 下午5:41
 */

#ifndef LOG_H
#define	LOG_H

#include "utils.h"

class Logger {
public:
    enum Type {
        INVALID = -1,
        ERROR = 0,
        WARNING,
        NOTICE,
    };
    
    Logger() {
    }

    virtual ~Logger() {
    }

    virtual void operator()(int type, const char *format, ...) = 0;

private:
    DISABLE_ASSIGN_AND_COPY(Logger);
};

extern Logger* logger_global;

#define DoLog logger_global->operator()

#endif	/* LOG_H */
