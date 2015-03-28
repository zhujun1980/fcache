/* 
 * File:   bloom_filter.h
 * Author: zhujun
 *
 * Created on 2014年8月5日, 上午10:38
 */

#ifndef BLOOM_FILTER_H
#define	BLOOM_FILTER_H

#include <stdint.h>
#include <iostream>
#include <memory>
#include <vector>
#include "utils.h"
#include "bit_set.h"

class BloomFilter {
public:
    struct Stats {
        uint64_t bit_size;
        uint64_t byte_size;
        uint16_t hashs_cnt;
        uint16_t data_vers;
    };

    static const char* EXT_NAME;
    static const int BITSET_EXCESS = 20;

    typedef uint64_t Seed;
    typedef std::vector<Seed> HashSeeds;
    typedef BitSet* BitSetPointer;
    //typedef std::shared_ptr<BitSet> BitSetPointer;

public:
    BloomFilter(int hash, int numElement, int bitsPerElement, unsigned int data_ver);

    BloomFilter(uint16_t data_ver, const HashSeeds& seeds, BitSetPointer bs) : data_version(data_ver), hash_seeds(seeds), bitset(bs) {
    }
    
    virtual ~BloomFilter() {
        delete bitset;
    }

    void Add(const std::string& kb);
    bool IsPresent(const std::string& kb) const;

    bool Save(const std::string& filepath);
    
    Stats Stat() {
        Stats s;
        s.hashs_cnt = hash_seeds.size();
        s.data_vers = data_version;
        s.bit_size = bitset->bit_size;
        s.byte_size = bitset->byte_size;
        return s;
    }

    //static functions
    static BloomFilter* Load(void* addr, size_t size);
    static BloomFilter* Load(const std::string& filepath);

private:
    uint64_t Hash(const std::string& kb, Seed seed) const;

    DISABLE_ASSIGN_AND_COPY(BloomFilter);

    uint16_t data_version;
    HashSeeds hash_seeds;
    BitSetPointer bitset;
};

#endif	/* BLOOM_FILTER_H */
