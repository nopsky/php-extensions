dnl $Id$
dnl config.m4 for extension lumen_ClassLoader

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(lumen_ClassLoader, for lumen_ClassLoader support,
Make sure that the comment is aligned:
[  --with-lumen_ClassLoader             Include lumen_ClassLoader support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(lumen_ClassLoader, whether to enable lumen_ClassLoader support,
Make sure that the comment is aligned:
[  --enable-lumen_ClassLoader           Enable lumen_ClassLoader support])

if test "$PHP_LUMEN_CLASSLOADER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-lumen_ClassLoader -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/lumen_ClassLoader.h"  # you most likely want to change this
  dnl if test -r $PHP_LUMEN_CLASSLOADER/$SEARCH_FOR; then # path given as parameter
  dnl   LUMEN_CLASSLOADER_DIR=$PHP_LUMEN_CLASSLOADER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for lumen_ClassLoader files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       LUMEN_CLASSLOADER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$LUMEN_CLASSLOADER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the lumen_ClassLoader distribution])
  dnl fi

  dnl # --with-lumen_ClassLoader -> add include path
  dnl PHP_ADD_INCLUDE($LUMEN_CLASSLOADER_DIR/include)

  dnl # --with-lumen_ClassLoader -> check for lib and symbol presence
  dnl LIBNAME=lumen_ClassLoader # you may want to change this
  dnl LIBSYMBOL=lumen_ClassLoader # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LUMEN_CLASSLOADER_DIR/$PHP_LIBDIR, LUMEN_CLASSLOADER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_LUMEN_CLASSLOADERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong lumen_ClassLoader lib version or lib not found])
  dnl ],[
  dnl   -L$LUMEN_CLASSLOADER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  PHP_SUBST(LUMEN_CLASSLOADER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(lumen_ClassLoader, lumen_ClassLoader.c, $ext_shared)
fi
