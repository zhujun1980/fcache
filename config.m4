PHP_ARG_ENABLE(fcache, whether to enable fcache support,
[  --enable-fcache           Enable fcache support])

if test "$PHP_FCACHE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-fcache -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/fcache.h"  # you most likely want to change this
  dnl if test -r $PHP_FCACHE/$SEARCH_FOR; then # path given as parameter
  dnl   FCACHE_DIR=$PHP_FCACHE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for fcache files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       FCACHE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$FCACHE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the fcache distribution])
  dnl fi

  dnl # --with-fcache -> add include path
  dnl PHP_ADD_INCLUDE($FCACHE_DIR/include)

  dnl # --with-fcache -> check for lib and symbol presence
  dnl LIBNAME=fcache # you may want to change this
  dnl LIBSYMBOL=fcache # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FCACHE_DIR/lib, FCACHE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_FCACHELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong fcache lib version or lib not found])
  dnl ],[
  dnl   -L$FCACHE_DIR/lib -lm
  dnl ])
  dnl

  PHP_REQUIRE_CXX()
  PHP_SUBST(FCACHE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, 1, FCACHE_SHARED_LIBADD)
  PHP_NEW_EXTENSION(fcache, fcache.cc                       \
                            fcache_php_class.cc             \
                            fcache_set.cc                   \
                            fcache_dict.cc                  \
                            fixed_array.cc                  \
                            deep_copy.cc                    \
                            fcache/murmur_hash3.cc          \
                            fcache/bit_set.cc               \
                            fcache/file_utils.cc            \
                            fcache/rand_utils.cc            \
                            fcache/time_utils.cc            \
                            fcache/shmm_posix.cc            \
                            fcache/bloom_filter.cc,         $ext_shared)
fi
