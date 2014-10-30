//
//  bit_set.h
//  BloomFilter
//
//  Created by zhu jun on 14-7-28.
//  Copyright (c) 2014年 Weibo. All rights reserved.
//

#ifndef __BloomFilter__bit_set__
#define __BloomFilter__bit_set__

#include <iostream>
#include <ostream>
#include <vector>
#include <memory>
#include "utils.h"

class BitSet {
public:
    enum DataLoc {
        INVALID = 0,
        SHM_MEM,
        IN_HEAP,
    };

    BitSet(long _bit_size, long _byte_size, char* _bytes, DataLoc _data_loc);
    BitSet(long numBits);
    virtual ~BitSet();

    bool Set(long index);
    bool Get(long index);
    bool Clear(long index);
    void Clear();
    long Capacity();

    bool Save(int fd);
    
    static BitSet* Load(void* addr, size_t size);
    static BitSet* Load(int fd);
    
    friend class BloomFilter;
    
    long BitSize() const {
        return bit_size;
    }
    
    long ByteSize() const {
        return byte_size;
    }

private:
    DISABLE_ASSIGN_AND_COPY(BitSet);
    
    long bit_size;
    long byte_size;
    char* bytes;
    DataLoc data_loc;
};

#endif /* defined(__BloomFilter__bit_set__) */
