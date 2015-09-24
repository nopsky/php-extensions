/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_LUMEN_CLASSLOADER_H
#define PHP_LUMEN_CLASSLOADER_H

extern zend_module_entry lumen_ClassLoader_module_entry;
#define phpext_lumen_ClassLoader_ptr &lumen_ClassLoader_module_entry

#define PHP_LUMEN_CLASSLOADER_VERSION "0.1.0" /* Replace with version number for your extension */

PHP_METHOD(ClassLoader, __construct);
PHP_METHOD(ClassLoader, getPrefixes);
PHP_METHOD(ClassLoader, getPrefixesPsr4);
PHP_METHOD(ClassLoader, getFallbackDirs);
PHP_METHOD(ClassLoader, getFallbackDirsPsr4);
PHP_METHOD(ClassLoader, getClassMap);
PHP_METHOD(ClassLoader, addClassMap);
PHP_METHOD(ClassLoader, add);
PHP_METHOD(ClassLoader, addPsr4);
PHP_METHOD(ClassLoader, set);
PHP_METHOD(ClassLoader, setPsr4);
PHP_METHOD(ClassLoader, setUseIncludePath);
PHP_METHOD(ClassLoader, getUseIncludePath);
PHP_METHOD(ClassLoader, setClassMapAuthoritative);
PHP_METHOD(ClassLoader, isClassMapAuthoritative);
PHP_METHOD(ClassLoader, register);
PHP_METHOD(ClassLoader, unregister);
PHP_METHOD(ClassLoader, loadClass);
PHP_METHOD(ClassLoader, findFile);
PHP_METHOD(ClassLoader, findFileWithExtension);

#ifdef PHP_WIN32
#	define PHP_LUMEN_CLASSLOADER_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_LUMEN_CLASSLOADER_API __attribute__ ((visibility("default")))
#else
#	define PHP_LUMEN_CLASSLOADER_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(lumen_ClassLoader)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(lumen_ClassLoader)
*/

/* In every utility function you add that needs to use variables 
   in php_lumen_ClassLoader_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as LUMEN_CLASSLOADER_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define LUMEN_CLASSLOADER_G(v) TSRMG(lumen_ClassLoader_globals_id, zend_lumen_ClassLoader_globals *, v)
#else
#define LUMEN_CLASSLOADER_G(v) (lumen_ClassLoader_globals.v)
#endif

#endif	/* PHP_LUMEN_CLASSLOADER_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
