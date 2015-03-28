/* 
 * File:   deep_copy.h
 * Author: zhujun
 *
 * Created on 2014年10月7日, 上午6:50
 */

#ifndef DEEP_COPY_H
#define	DEEP_COPY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "php.h"
#include "Zend/zend.h"

#define FIXUP(t, p, b) ((p) ? (reinterpret_cast<t>((u_char*)(p) - (u_char*)(b))) : NULL)

#define REALADDR(t, b, s) ((s) ? (reinterpret_cast<t>((u_char*)(b) + (size_t)(s))) : NULL)

size_t calc_size(zval *value);

size_t deep_copy(zval *src, zval **dst);

void free_copy(zval *value);

void fixup_value(zval *value);

/**
 * restore in fixedarray
 * fixedarray implement arrayobject
 * big-end
 */

#ifdef	__cplusplus
}
#endif

#endif	/* DEEP_COPY_H */
