/* 
 * File:   shmm.h
 * Author: zhujun
 *
 * Created on 2014年8月6日, 下午2:21
 */

#ifndef SHMM_H
#define	SHMM_H

#ifdef	__cplusplus
extern "C" {
#endif

void* CreateMmap(const char* file, size_t segsize);

void* MMap(const char* file, size_t segsize, int fg);

bool MUnMap(void* addr, size_t segsize);

#ifdef	__cplusplus
}
#endif

#endif	/* SHMM_H */
