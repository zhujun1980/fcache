/* 
 * File:   fcache_php_class.cc
 * Author: zhujun
 *
 * Created on 2014年8月5日, 上午10:18
 */

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdarg.h>    
#include <time.h>
#include "php.h"
#include "Zend/zend.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_filestat.h"
#include "ext/standard/php_string.h"

#ifdef	__cplusplus
}
#endif

#include <string>
#include "php_fcache.h"
#include "fcache_php_class.h"
#include "fcache/log.h"
#include "fcache/bloom_filter.h"
#include "fcache/fc.h"

zend_class_entry *fcache_bloom_filter_cls;

static time_t last_purge_time;

Logger* logger_global = NULL;

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

PHP_METHOD(fcache_bloom_filter, isPresent) {
    char* id = NULL;
    char* key = NULL;
    char* db = NULL;
    long id_len = 0;
    long key_len = 0;
    long db_len = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|s", &id, &id_len, &key, &key_len, &db, &db_len) == FAILURE || id_len == 0 || key_len == 0) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "setid and key must not be empty");
	RETURN_FALSE;
    }

    std::string base(FCACHE_G(data_dir));
    std::string dbname(db, db_len);
    std::string filename(id, id_len);
    std::string extname(BloomFilter::EXT_NAME);

    FCache<BloomFilter> caches;
    const BloomFilter* filter = caches.GetMappingObject(base, dbname, filename, extname);
    if(!filter) {
        RETURN_FALSE;
    }
    std::string k(key, key_len);
    RETURN_BOOL(filter->IsPresent(k));
}

PHP_METHOD(fcache_bloom_filter, createIdx) {
    char* txtFile = NULL;
    char* idxFile = NULL;
    long txt_len = 0;
    long idx_len = 0;
    long numElement = 0;
    long dataVers = 0;
    long hashCnt = 8;
    long bitsPerElement = 16;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssll|ll", &txtFile, &txt_len, &idxFile, &idx_len, &numElement, &dataVers, &hashCnt, &bitsPerElement) == FAILURE || txt_len == 0 || idx_len == 0) {
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "txtFile and idxFile must not be empty");
	return;
    }
    if(numElement == 0) {
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "number of Element must be greater than zero.");
	return;
    }

    BloomFilter filter(hashCnt, numElement, bitsPerElement, dataVers);
    char buf[129];
    FILE *in = fopen(txtFile, "r");
    if(in) {
        while(1) {
            fgets(buf, sizeof(char) * 128, in);
            if(feof(in)) {
                std::string uid(buf);
                filter.Add(uid);
                break;
            }
            std::string uid(buf, strlen(buf) - 1); //exclude last newline
            filter.Add(uid);
        }
        fclose(in);
    }
    else {
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "open txtFile failed.");
	return;
    }
    std::string idxPath(idxFile, idx_len);
    int ret = filter.Save(idxPath);
    RETURN_BOOL(ret);
}

PHP_METHOD(fcache_bloom_filter, stat) {
    char* idxFile = NULL;
    long idx_len = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &idxFile, &idx_len) == FAILURE || idx_len == 0) {
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "idxFile must not be empty");
	return;
    }

    std::string idxPath(idxFile, idx_len);
    BloomFilter* filter = BloomFilter::Load(idxPath);
    if(filter == NULL) {
        RETURN_FALSE;
    }
    BloomFilter::Stats stat = filter->Stat();
    delete filter;

    zval *qs;
    MAKE_STD_ZVAL(qs);
    array_init(qs);
    add_assoc_long_ex(qs, BLOOM_FILTER_STAT_BITS, sizeof(BLOOM_FILTER_STAT_BITS), stat.bit_size);
    add_assoc_long_ex(qs, BLOOM_FILTER_STAT_BYTES, sizeof(BLOOM_FILTER_STAT_BYTES), stat.byte_size);
    add_assoc_long_ex(qs, BLOOM_FILTER_STAT_VER, sizeof(BLOOM_FILTER_STAT_VER), stat.data_vers);
    add_assoc_long_ex(qs, BLOOM_FILTER_STAT_HC, sizeof(BLOOM_FILTER_STAT_HC), stat.hashs_cnt);
    RETURN_ZVAL(qs, 0, 1);
}

ZEND_BEGIN_ARG_INFO_EX(isPresent_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, db)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(createIdx_arginfo, 0, 0, 4)
    ZEND_ARG_INFO(0, txtFile)
    ZEND_ARG_INFO(0, idxFile)
    ZEND_ARG_INFO(0, numElement)
    ZEND_ARG_INFO(0, dataVers)
    ZEND_ARG_INFO(0, hashCnt)
    ZEND_ARG_INFO(0, bitsPerElement)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(stat_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, idxFile)
ZEND_END_ARG_INFO()

static zend_function_entry fcache_bloom_filter_methods[] = {
    PHP_ME(fcache_bloom_filter, isPresent, isPresent_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(fcache_bloom_filter, createIdx, createIdx_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(fcache_bloom_filter, stat, stat_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(fcache_core) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, FCACHE_BLOOM_FILTER_N, fcache_bloom_filter_methods);
    fcache_bloom_filter_cls = zend_register_internal_class_ex(&ce ,NULL, NULL TSRMLS_CC);

    zend_declare_property_string(fcache_bloom_filter_cls, ZEND_STRL("EXT_NAME"), const_cast<char*>(BloomFilter::EXT_NAME), ZEND_ACC_STATIC | ZEND_ACC_PUBLIC | ZEND_ACC_FINAL TSRMLS_CC);

    logger_global = new PHPLogger(TSRMLS_C);
    last_purge_time = time(NULL);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(fcache_core) {
    FCache<BloomFilter>::Clear();
    delete logger_global;
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(fcache_core) {
    time_t cur = time(NULL);
    if (FCACHE_G(purge_period) == 0 || cur - last_purge_time > FCACHE_G(purge_period)) {
        last_purge_time = cur;
        FCache<BloomFilter>::Purge(FCACHE_G(unused_threshold));
    }
    return SUCCESS;
}

PHP_MINFO_FUNCTION(fcache_core) {
}
