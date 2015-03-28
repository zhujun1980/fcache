/* 
 * File:   bloom_filter.cc
 * Author: zhujun
 * 
 * Created on 2014年8月5日, 上午10:38
 */

#include <new>
#include <errno.h>
#include "consts.h"
#include "bloom_filter.h"
#include "murmur_hash3.h"
#include "rand_utils.h"
#include "file_utils.h"
#include "log.h"

const char* BloomFilter::EXT_NAME = "bf";

BloomFilter::BloomFilter(int hash, int numElement, int bitsPerElement, unsigned int data_ver) : data_version(data_ver) {
    for (int i = 0; i < hash; i++)
        hash_seeds.push_back(RandUint64());
    bitset = new BitSet(numElement * bitsPerElement + BloomFilter::BITSET_EXCESS);
}

inline uint64_t BloomFilter::Hash(const std::string& kb, Seed seed) const {
    uint32_t index;
    MurmurHash3_x86_32(kb.c_str(), static_cast<int>(kb.size()), static_cast<uint32_t>(seed), &index);
    return index % bitset->Capacity();
}

void BloomFilter::Add(const std::string& kb) {
    for (int i = 0; i < hash_seeds.size(); i++) {
        bitset->Set(Hash(kb, hash_seeds[i]));
    }
}

bool BloomFilter::IsPresent(const std::string& kb) const {
    for (int i = 0; i < hash_seeds.size(); i++) {
        if(!bitset->Get(Hash(kb, hash_seeds[i])))
            return false;
    }
    return true;
}

/**
 * Save bloom filter data to file.
 * 
 * [data_type 2 bytes][data_version 2 bytes][hash_function_count 2 bytes]
 * [hash_seed 8 bytes] * [hash_function_count]
 * [byte_size 8 bytes][bitset_data...]
 * 
 * @param filepath
 */
bool BloomFilter::Save(const std::string& filepath) {
    FILE* idx = OpenFile(filepath.c_str(), "w+");
    if(idx == NULL) {
        DoLog(Logger::ERROR, "Open BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        return false;
    }
    int fd = fileno(idx);
    
    int ret;
    uint16_t sec;
    
    sec = BLOOM_FILTER;
    ret = WriteFD(fd, reinterpret_cast<const char*>(&sec), sizeof(sec));
    if(ret < 0) {
        DoLog(Logger::ERROR, "write BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return false;
    }

    sec = data_version;
    ret = WriteFD(fd, reinterpret_cast<const char*>(&sec), sizeof(sec));
    if(ret < 0) {
        DoLog(Logger::ERROR, "write BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return false;
    }

    sec = hash_seeds.size();
    ret = WriteFD(fd, reinterpret_cast<const char*>(&sec), sizeof(sec));
    if(ret < 0) {
        DoLog(Logger::ERROR, "write BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return false;
    }

    for (int i = 0; i < hash_seeds.size(); i++) {
        uint64_t seed = hash_seeds[i];
        ret = WriteFD(fd, reinterpret_cast<const char*>(&seed), sizeof(seed));
        if(ret < 0) {
            DoLog(Logger::ERROR, "write BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
            CloseFile(idx);
            return false;
        }
    }
    
    ret = bitset->Save(fd);
    if(!ret) {
        DoLog(Logger::ERROR, "save bitset failed (%s:%d)", __FILE__, __LINE__);
        CloseFile(idx);
        return false;
    }

    CloseFile(idx);
    return true;
}

//static
BloomFilter* BloomFilter::Load(void* addr, size_t size) {
    uint16_t* sec;
    HashSeeds hash_seeds;
    bool ret;
    uint16_t data_ver;
    int hash_cnt;

    if(size == 0) {
        DoLog(Logger::WARNING, "size equal zero (%s:%d)", __FILE__, __LINE__);
        return NULL;
    }
    sec = reinterpret_cast<uint16_t*>(addr);
    if(*sec++ != BLOOM_FILTER) {
        DoLog(Logger::WARNING, "file invalid (%s:%d)", __FILE__, __LINE__);
        return NULL;
    }
    data_ver = *sec++;
    hash_cnt = *sec++;
    uint64_t* seed = reinterpret_cast<uint64_t*>(sec);
    for (int i = 0; i < hash_cnt; i++, seed++) {
        hash_seeds.push_back(*seed);
    }
    BitSet* bitset = BitSet::Load(seed, size);
    if(bitset == NULL) {
        DoLog(Logger::WARNING, "create bitset failed (%s:%d)", __FILE__, __LINE__);
        return NULL;
    }
    return new(std::nothrow) BloomFilter(data_ver, hash_seeds, bitset);
}

BloomFilter* BloomFilter::Load(const std::string& filepath) {
    FILE* idx = OpenFile(filepath.c_str(), "r");
    if(idx == NULL) {
        DoLog(Logger::WARNING, "Open BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        return NULL;
    }

    int fd;
    uint16_t sec;
    HashSeeds hash_seeds;
    bool ret;
    uint16_t data_ver;
    
    fd = fileno(idx);
    //read data type
    ret = ReadFromFD(fd, reinterpret_cast<char*>(&sec), sizeof(sec));
    if(!ret) {
        DoLog(Logger::WARNING, "read BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return NULL;
    }
    if(sec != BLOOM_FILTER) {
        DoLog(Logger::WARNING, "invalid file failed %d (%s:%d)", sec, __FILE__, __LINE__);
        CloseFile(idx);
        return NULL;
    }

    //read data version
    ret = ReadFromFD(fd, reinterpret_cast<char*>(&sec), sizeof(sec));
    if(!ret) {
        DoLog(Logger::WARNING, "read BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return NULL;
    }
    data_ver = sec;

    ret = ReadFromFD(fd, reinterpret_cast<char*>(&sec), sizeof(sec));
    if(!ret) {
        DoLog(Logger::WARNING, "read BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
        CloseFile(idx);
        return NULL;
    }

    for (int i = 0; i < sec; i++) {
        uint64_t seed;
        ret = ReadFromFD(fd, reinterpret_cast<char*>(&seed), sizeof(seed));
        if(!ret) {
            DoLog(Logger::WARNING, "read BloomFilter file failed %d (%s:%d)", errno, __FILE__, __LINE__);
            CloseFile(idx);
            return NULL;
        }
        hash_seeds.push_back(seed);
    }
    BitSet* bitset = BitSet::Load(fd);
    CloseFile(idx);
    if(bitset == NULL) {
        DoLog(Logger::WARNING, "create bitset failed (%s:%d)", __FILE__, __LINE__);
        return NULL;
    }
    return new(std::nothrow) BloomFilter(data_ver, hash_seeds, bitset);
}
