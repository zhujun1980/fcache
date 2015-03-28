/* 
 * File:   fcache_dict.cc
 * Author: zhujun
 *
 * Created on 2014年9月15日, 上午10:18
 */

#ifdef	__cplusplus
extern "C" {
#endif

#include <time.h>
#include "php.h"
#include "Zend/zend.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_variables.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#ifdef	__cplusplus
}
#endif

#include <string>
#include "php_fcache.h"
#include "php_logger.h"
#include "fcache_dict.h"
#include "fixed_array.h"
#include "fcache/fc.h"
#include "fcache/consts.h"

zend_class_entry *fcache_fixed_array_ce;

static zend_object_handlers fcache_fixed_array_handlers;

typedef struct _fixed_array_object {
    zend_object         std;
    const FixedArray    *array;
    const Bucket        *current;
} fixed_array_object;

PHP_METHOD(fcache_fixed_array, fromArray) {
    zval *array;
    fixed_array_object *intern;
    const FixedArray *fa;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &array) == FAILURE || Z_TYPE_P(array) != IS_ARRAY) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "param error");
		RETURN_FALSE;
    }

    fa = FixedArray::Load(array);
    if(!fa) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "create object from array failed");
		RETURN_FALSE;
    }

    object_init_ex(return_value, fcache_fixed_array_ce);
    Z_TYPE_P(return_value) = IS_OBJECT;
    Z_SET_REFCOUNT_P(return_value, 1);
    Z_UNSET_ISREF_P(return_value);

    intern = (fixed_array_object*)zend_object_store_get_object(return_value TSRMLS_CC);
    intern->array = fa;
    intern->current = fa->Head();
}

PHP_METHOD(fcache_fixed_array, fromFile) {
    char *id = NULL;
    char *db = NULL;
    long id_len = 0;
    long db_len = 0;
    fixed_array_object *intern;
    const FixedArray *fa;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &id, &id_len, &db, &db_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "param error");
		RETURN_FALSE;
    }

    std::string base(FCACHE_G(data_dir));
    std::string dbname(db, db_len);
    std::string filename(id, id_len);
    std::string extname(FixedArray::EXT_NAME);

    FCache<FixedArray> caches;
    fa = caches.GetMappingObject(base, dbname, filename, extname);
    if(!fa) {
    	php_error_docref(NULL TSRMLS_CC, E_WARNING, "create object from file failed");
        RETURN_FALSE;
    }

    object_init_ex(return_value, fcache_fixed_array_ce);
    Z_TYPE_P(return_value) = IS_OBJECT;
    Z_SET_REFCOUNT_P(return_value, 1);
    Z_UNSET_ISREF_P(return_value);

    intern = (fixed_array_object*)zend_object_store_get_object(return_value TSRMLS_CC);
    intern->array = fa;
    intern->current = fa->Head();
}

PHP_METHOD(fcache_fixed_array, __construct) {
}

PHP_METHOD(fcache_fixed_array, toFile) {
    char *filepath = NULL;
    long path_len = 0;
    fixed_array_object *intern;
    const FixedArray *fa;
    bool ret;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filepath, &path_len) == FAILURE || path_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "param error");
		RETURN_FALSE;
    }

    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    fa = intern->array;

    std::string file(filepath, path_len);
    ret = fa->Save(file);
    RETURN_BOOL(ret);
}

PHP_METHOD(fcache_fixed_array, toArray) {
}

PHP_METHOD(fcache_fixed_array, count) {
    fixed_array_object *intern;
    const FixedArray *fa;

    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    fa = intern->array;

    RETURN_LONG(fa->Count());
}

PHP_METHOD(fcache_fixed_array, current) {
    bool ret;
    zval *val;
    fixed_array_object *intern;
    const FixedArray *fa;

    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    fa = intern->array;
    ret = fa->Current(intern->current, &val);
    if(!ret) RETURN_NULL();
    RETURN_ZVAL(val, 0, 0);
}

PHP_METHOD(fcache_fixed_array, key) {
    bool ret;
    zval *val;

    fixed_array_object *intern;
    const FixedArray *fa;

    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    fa = intern->array;
    ret = fa->Key(intern->current, &val);
    if(!ret) RETURN_NULL();
    RETURN_ZVAL(val, 0, 0);
}

PHP_METHOD(fcache_fixed_array, next) {
    fixed_array_object *intern;
    const FixedArray *fa;

    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    if(intern->current == NULL) {
        return;
    }

    fa = intern->array;
    intern->current = fa->NextBucket(intern->current);
}

PHP_METHOD(fcache_fixed_array, rewind) {
    fixed_array_object *intern;
    const FixedArray *fa;

    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    fa = intern->array;
    intern->current = fa->Head();
}

PHP_METHOD(fcache_fixed_array, valid) {
    fixed_array_object *intern;
    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    RETURN_BOOL(intern->current != NULL);
}

PHP_METHOD(fcache_fixed_array, offsetExists) {
    bool ret;
    zval *val;
    zval *offset;
    const FixedArray *fa;
    fixed_array_object *intern;

    val = NULL;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &offset) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "param error");
		RETURN_FALSE;
    }

    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    fa = intern->array;

    if(Z_TYPE_P(offset) == IS_LONG) {
        ret = fa->Get(Z_LVAL_P(offset), &val);
    }
    else if(Z_TYPE_P(offset) == IS_STRING){
        std::string key(Z_STRVAL_P(offset), Z_STRLEN_P(offset) + 1);
        ret = fa->Get(key, &val);
    }
    else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "key type error");
		RETURN_FALSE;
    }
    if(ret || val != NULL)
        zval_ptr_dtor(&val);
    RETURN_BOOL(ret);
}

PHP_METHOD(fcache_fixed_array, offsetGet) {
    bool ret;
    zval *val;
    zval *offset;
    const FixedArray *fa;
    fixed_array_object *intern;

    val = NULL;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &offset) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "param error");
		RETURN_FALSE;
    }

    intern = (fixed_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    fa = intern->array;

    if(Z_TYPE_P(offset) == IS_LONG) {
        ret = fa->Get(Z_LVAL_P(offset), &val);
    }
    else if(Z_TYPE_P(offset) == IS_STRING){
        std::string key(Z_STRVAL_P(offset), Z_STRLEN_P(offset) + 1);
        ret = fa->Get(key, &val);
    }
    else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "key type error");
		RETURN_FALSE;
    }
    if(!ret) {
    	RETURN_NULL();
    }
    RETURN_ZVAL(val, 0, 0);
}

PHP_METHOD(fcache_fixed_array, offsetSet) {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "FixedArray object is read-only");
}

PHP_METHOD(fcache_fixed_array, offsetUnset) {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "FixedArray object is read-only");
}

ZEND_BEGIN_ARG_INFO_EX(from_array_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(from_file_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, db)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(__construct_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(toFile_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, filepath)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(toArray_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(count_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(current_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(key_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(next_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(rewind_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(valid_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(offsetExists_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(offsetGet_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(offsetSet_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(offsetUnset_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

static zend_function_entry fcache_fixed_array_methods[] = {
    PHP_ME(fcache_fixed_array, fromArray, from_array_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, fromFile, from_file_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)

    PHP_ME(fcache_fixed_array, __construct, __construct_arginfo, ZEND_ACC_PRIVATE)

    PHP_ME(fcache_fixed_array, toFile, toFile_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, toArray, toArray_arginfo, ZEND_ACC_PUBLIC)

    PHP_ME(fcache_fixed_array, count, count_arginfo, ZEND_ACC_PUBLIC)
    
    PHP_ME(fcache_fixed_array, current, current_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, key, key_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, next, next_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, rewind, rewind_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, valid, valid_arginfo, ZEND_ACC_PUBLIC)
    
    PHP_ME(fcache_fixed_array, offsetExists, offsetExists_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, offsetGet, offsetGet_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, offsetSet, offsetSet_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(fcache_fixed_array, offsetUnset, offsetUnset_arginfo, ZEND_ACC_PUBLIC)

    {NULL, NULL, NULL}
};

static void fixed_array_object_free_storage(void *object TSRMLS_DC) {
    fixed_array_object  *intern;

    intern = reinterpret_cast<fixed_array_object*>(object);

    zend_object_std_dtor(&intern->std TSRMLS_CC);

    if(intern->array->DataLoc() == DATA_LOC_HEAP) {
        delete intern->array;
        intern->array = NULL;
    }

    efree(object);
}

static zend_object_value fixed_array_new(zend_class_entry *class_type TSRMLS_DC) {
    fixed_array_object  *intern;
    zend_object_value   retval;

    intern = reinterpret_cast<fixed_array_object*>(ecalloc(1, sizeof(fixed_array_object)));
    memset(intern, 0, sizeof(fixed_array_object));

    zend_object_std_init(&intern->std, class_type TSRMLS_CC);
    object_properties_init(&intern->std, class_type);

    retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)fixed_array_object_free_storage, NULL TSRMLS_CC);
    retval.handlers = &fcache_fixed_array_handlers;

    return retval;
}

FCACHE_STARTUP_D(dict) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, FIXED_ARRAY_N, fcache_fixed_array_methods);
    ce.create_object = fixed_array_new;
    fcache_fixed_array_ce = zend_register_internal_class_ex(&ce ,NULL, NULL TSRMLS_CC);
    memcpy(&fcache_fixed_array_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    //fcache_fixed_array_handlers.

    zend_class_implements(fcache_fixed_array_ce TSRMLS_CC, 1, zend_ce_iterator);
    zend_class_implements(fcache_fixed_array_ce TSRMLS_CC, 1, zend_ce_arrayaccess);
    //zend_class_implements(fcache_fixed_array TSRMLS_CC, 1, zend_ce_traversable);

    return SUCCESS;
}

FCACHE_SHUTDOWN_D(dict) {
    FCache<FixedArray>::Purge(FCACHE_G(unused_threshold));
    return SUCCESS;
}

FCACHE_DEACTIVATE_D(dict) {
    FCache<FixedArray>::Clear();
    return SUCCESS;
}

FCACHE_INFO_D(dict) {
}
