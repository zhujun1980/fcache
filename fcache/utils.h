/* 
 * File:   utils.h
 * Author: zhujun
 *
 * Created on 2014年8月4日, 下午2:05
 */

#ifndef UTILS_H
#define	UTILS_H

#define DISABLE_ASSIGN_AND_COPY(TypeName)       \
        TypeName(const TypeName&);              \
        TypeName& operator=(const TypeName&)

//Re-runs fn until it doesn't cause EINTR.
#define NO_INTR(fn)   do {} while ((fn) < 0 && errno == EINTR)

typedef union align_union {
    double d;
    void *v;
    int (*func)(int);
    long l;
} align_union;

#define FCACHE_ALIGNMENT sizeof(align_union)

#define fcache_align(n, a) (n) = ((((size_t)(n) - 1) & ~((a) - 1)) + (a))
#define fcache_align_ptr(n, a) (n) = (u_char*)((((size_t)(n) - 1) & ~((size_t)(a) - 1)) + (size_t)(a))

#endif	/* UTILS_H */
