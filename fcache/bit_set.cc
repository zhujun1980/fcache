//
//  bit_set.cc
//  BloomFilter
//
//  Created by zhu jun on 14-7-28.
//  Copyright (c) 2014年 Weibo. All rights reserved.
//
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <new>
#include "bit_set.h"
#include "file_utils.h"
#include "log.h"

BitSet::BitSet(long _bit_size, long _byte_size, char* _bytes, DataLoc _data_loc) : 
        bit_size(_bit_size), byte_size(_byte_size), 
        bytes(_bytes), data_loc(_data_loc) {
}

BitSet::BitSet(long numBits) : data_loc(IN_HEAP) {
    if(numBits == 0)
        numBits = 1;
    byte_size = (numBits + 7) >> 3;
    bit_size = byte_size * 8;

    bytes = new char[byte_size];
    memset(bytes, 0, byte_size);
}

BitSet::~BitSet() {
    if(data_loc == IN_HEAP)
        delete[] bytes;
}

bool BitSet::Set(long index) {
    if(index < 0L || index >= bit_size)
        return false;

    bytes[index >> 3] |= 1 << (index & 0x7);
    return true;
}

bool BitSet::Clear(long index) {
    if(index < 0 || index >= bit_size)
        return false;

    bytes[index >> 3] &= ~(1 << (index & 0x7));
    return true;
}

void BitSet::Clear() {
    memset(bytes, 0, byte_size);
}

bool BitSet::Get(long index) {
    if(index < 0 || index >= bit_size)
        return false;

    return bytes[index >> 3] & (1 << (index & 0x7));
}

long BitSet::Capacity() {
    return bit_size;
}

bool BitSet::Save(int fd) {
    uint64_t sec;
    int ret;
    
    sec = byte_size;
    ret = WriteFD(fd, reinterpret_cast<const char*>(&sec), sizeof(sec));
    if(ret < 0) {
        DoLog(Logger::ERROR, "write bitset data failed %d (%s:%d)", errno, __FILE__, __LINE__);
        return false;
    }
    ret = WriteFD(fd, reinterpret_cast<const char*>(bytes), byte_size);
    if(ret < 0) {
        DoLog(Logger::ERROR, "write bitset data failed %d (%s:%d)", errno, __FILE__, __LINE__);
        return false;
    }
    return true;
}

//static
BitSet* BitSet::Load(void* addr, size_t size) {
    uint64_t* bs;
    uint64_t byte_size;

    bs = reinterpret_cast<uint64_t*>(addr);
    byte_size = *bs++;
    char* bytes = reinterpret_cast<char*>(bs);
    return new(std::nothrow) BitSet(byte_size * 8, byte_size, bytes, SHM_MEM);
}

BitSet* BitSet::Load(int fd) {
    uint64_t bs;
    bool ret = ReadFromFD(fd, reinterpret_cast<char*>(&bs), sizeof(bs));
    if(!ret) {
        DoLog(Logger::WARNING, "read bitset failed %d (%s:%d)", errno, __FILE__, __LINE__);
        return NULL;
    }
    char* bytes = new(std::nothrow) char[bs];
    if(bytes == NULL) {
        DoLog(Logger::WARNING, "not enough memory (%s:%d)", __FILE__, __LINE__);
        return NULL;
    }
    ret = ReadFromFD(fd, bytes, bs);
    if(!ret) {
        DoLog(Logger::WARNING, "read bitset failed %d (%s:%d)", errno, __FILE__, __LINE__);
        return NULL;
    }
    return new(std::nothrow) BitSet(bs * 8, bs, bytes, IN_HEAP);
}
