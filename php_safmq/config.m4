PHP_ARG_ENABLE(safmq, whether to enable SAFMQ support,
[  --disable-safmq   Disable SAFMQ support])


GPP_VER=`g++ -dumpversion`
GPP_MAG_VER=`g++ -dumpversion | cut -d "." -f 1,2`
LIB_DIR="../lib/bin.$GPP_VER"

SAFMQ_INCLUDE=..
SAFMQ_LIB_INCLUDE=../lib
SAFMQ_SHARED_INCLUDE=../shared

if test "$PHP_SAFMQ" != "no"; then
  PHP_REQUIRE_CXX()
  PHP_ADD_INCLUDE($SAFMQ_INCLUDE)
  PHP_ADD_INCLUDE($SAFMQ_LIB_INCLUDE)
  PHP_ADD_INCLUDE($SAFMQ_SHARED_INCLUDE)

  PHP_ADD_LIBRARY(stdc++, 1, SAFMQ_SHARED_LIBADD)
  PHP_ADD_LIBRARY(supc++, 1, SAFMQ_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(safmqclient, $LIB_DIR, SAFMQ_SHARED_LIBADD)
  PHP_ADD_LIBRARY(ssl, 1, SAFMQ_SHARED_LIBADD)
  PHP_ADD_LIBRARY(crypto, 1, SAFMQ_SHARED_LIBADD)

  CFLAGS+=" -Wno-write-strings"
  CXXFLAGS+=" -Wno-write-strings"

  AC_DEFINE(HAVE_SAFMQ, 1, [Whether you have SAFMQ])
  PHP_NEW_EXTENSION(safmq, safmq.cpp , $ext_shared)
  PHP_SUBST(SAFMQ_SHARED_LIBADD)

fi

