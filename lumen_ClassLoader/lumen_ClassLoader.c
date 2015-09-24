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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#include "php_lumen_ClassLoader.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_filestat.h"
#include "zend_interfaces.h"


/* If you declare any globals in php_lumen_ClassLoader.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(lumen_ClassLoader)
*/

zend_class_entry *ClassLoader_ce;


ZEND_BEGIN_ARG_INFO(addClassMap_arginfo, 0)
    ZEND_ARG_ARRAY_INFO(0, classMap, 0)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(add_arginfo, 0)
	ZEND_ARG_INFO(0, prefix)
	ZEND_ARG_INFO(0, paths)
	ZEND_ARG_INFO(0, prepend)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(addPsr4_arginfo, 0)
	ZEND_ARG_INFO(0, prefix)
	ZEND_ARG_INFO(0, paths)
	ZEND_ARG_INFO(0, prepend)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(set_arginfo, 0)
	ZEND_ARG_INFO(0, prefix)
	ZEND_ARG_INFO(0, paths)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(setPsr4_arginfo, 0)
	ZEND_ARG_INFO(0, prefix)
	ZEND_ARG_INFO(0, paths)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(setUseIncludePath_arginfo, 0)
	ZEND_ARG_INFO(0, useIncludePath)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(setClassMapAuthoritative_arginfo, 0)
	ZEND_ARG_INFO(0, classMapAuthoritative)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(register_arginfo, 0)
	ZEND_ARG_INFO(0, prepend)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(loadClass_arginfo, 0)
	ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(findFile_arginfo, 0)
	ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(findFileWithExtension_arginfo, 0)
	ZEND_ARG_INFO(0, class)
	ZEND_ARG_INFO(0, ext)
ZEND_END_ARG_INFO()

/* True global resources - no need for thread safety here */
static int le_lumen_ClassLoader;





/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("lumen_ClassLoader.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_lumen_ClassLoader_globals, lumen_ClassLoader_globals)
    STD_PHP_INI_ENTRY("lumen_ClassLoader.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_lumen_ClassLoader_globals, lumen_ClassLoader_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_lumen_ClassLoader_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(includeFile)
{
	zval *fileName = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|", &fileName) == FAILURE) {
		return;
	}
	//php_printf("arg:%s\n", Z_STRVAL_P(fileName));

	zend_op_array *op_array;

	op_array = compile_filename(ZEND_INCLUDE, fileName TSRMLS_CC);

	if(op_array) {

		zval *result = NULL;

		zval ** __old_return_value_pp   = EG(return_value_ptr_ptr);
		zend_op ** __old_opline_ptr  	= EG(opline_ptr);
		zend_op_array * __old_op_array  = EG(active_op_array);
		
		//EX(function_state).function = (zend_function *) op_array;


		EG(return_value_ptr_ptr) = &result;
		EG(active_op_array) 	 = op_array;

		if (!EG(active_symbol_table)) {
			zend_rebuild_symbol_table(TSRMLS_C);
		}

		zend_execute(op_array TSRMLS_CC);

		destroy_op_array(op_array TSRMLS_CC);

		efree(op_array);
		if (!EG(exception)) {
			if (EG(return_value_ptr_ptr) && *EG(return_value_ptr_ptr)) {
				zval_ptr_dtor(EG(return_value_ptr_ptr));
			}
		}

		//EX(function_state).function = (zend_function *) EX(op_array);
		EG(return_value_ptr_ptr) = __old_return_value_pp;
		EG(opline_ptr) = __old_opline_ptr;
		EG(active_op_array) = __old_op_array;
	}

}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_lumen_ClassLoader_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_lumen_ClassLoader_init_globals(zend_lumen_ClassLoader_globals *lumen_ClassLoader_globals)
{
	lumen_ClassLoader_globals->global_value = 0;
	lumen_ClassLoader_globals->global_string = NULL;
}
*/
/* }}} */

//定义析构方法
PHP_METHOD(ClassLoader, __construct) {
	//定义数组
	zval *prefixLengthsPsr4Value, *prefixDirsPsr4Value, *fallbackDirsPsr4Value, *prefixesPsr0Value, *fallbackDirsPsr0Value, *classMapValue;
	//this指针
	zval *pThis;
	pThis = getThis();
	//$this->prefixLengthsPsr4 = array("hello");
	MAKE_STD_ZVAL(prefixLengthsPsr4Value);
	array_init(prefixLengthsPsr4Value);

	MAKE_STD_ZVAL(prefixDirsPsr4Value);
	array_init(prefixDirsPsr4Value);

	MAKE_STD_ZVAL(fallbackDirsPsr4Value);
	array_init(fallbackDirsPsr4Value);

	MAKE_STD_ZVAL(prefixesPsr0Value);
	array_init(prefixesPsr0Value);

	MAKE_STD_ZVAL(fallbackDirsPsr0Value);
	array_init(fallbackDirsPsr0Value);

	MAKE_STD_ZVAL(classMapValue);
	array_init(classMapValue);

	//add_property_zval_ex(pThis, ZEND_STRS("prefixLengthsPsr4"), prefixLengthsPsr4Value);
	zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixLengthsPsr4"), prefixLengthsPsr4Value TSRMLS_CC);
	zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixDirsPsr4"), prefixDirsPsr4Value TSRMLS_CC);
	zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr4"), fallbackDirsPsr4Value TSRMLS_CC);
	zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixesPsr0"), prefixesPsr0Value TSRMLS_CC);
	zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr0"), fallbackDirsPsr0Value TSRMLS_CC);
	zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("classMap"), classMapValue TSRMLS_CC);
}

PHP_METHOD(ClassLoader, getPrefixes) {
	zval *pThis, *prefixesPsr0Value, *retval;
 
	MAKE_STD_ZVAL(retval);
	array_init(retval);

	pThis = getThis();
 
	prefixesPsr0Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixesPsr0"), 0 TSRMLS_CC);

	int result;

	result = (zend_hash_num_elements(Z_ARRVAL_P(prefixesPsr0Value))) ? 1 : 0;

	if(result) {
		zval *func, *funcname;
		MAKE_STD_ZVAL(func);
		ZVAL_STRING(func, "array_merge", 1);

		MAKE_STD_ZVAL(funcname);
		ZVAL_STRING(funcname, "call_user_func_array", 0);

		zval **params = (zval**)malloc(sizeof(zval));  
        params[0] = func;
        params[1] = prefixesPsr0Value;
        if(call_user_function(CG(function_table),NULL, funcname, retval, 2, params TSRMLS_CC)==FAILURE){  
                zend_error(E_ERROR,"call function failed");  
        } 
        zval_ptr_dtor(&func);
        //efree(funcname);
        zval_ptr_dtor(params);

		//php_printf("prefixesPsr0Value不为空\n");
	} else {
		//php_printf("prefixesPsr0Value为空\n");
	}

	RETURN_ZVAL(retval, 0, 1);
   

}

PHP_METHOD(ClassLoader, getPrefixesPsr4) {
	zval *pThis, *prefixDirsPsr4Value;

	pThis = getThis();

	prefixDirsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixDirsPsr4"), 0 TSRMLS_CC);

	RETURN_ZVAL(prefixDirsPsr4Value, 1, 0);
	//php_printf("这是getPrefixesPsr4\n");
}

PHP_METHOD(ClassLoader, getFallbackDirs) {
	zval *pThis, *fallbackDirsPsr0Value;

	pThis = getThis();

	fallbackDirsPsr0Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr0"), 0 TSRMLS_CC);
	//php_printf("这是getFallbackDirs\n");
	RETURN_ZVAL(fallbackDirsPsr0Value, 1, 0);
}

PHP_METHOD(ClassLoader, getFallbackDirsPsr4){
	zval *pThis, *fallbackDirsPsr4Value;

	pThis = getThis();

	fallbackDirsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr4"), 0 TSRMLS_CC);
	//php_printf("这是getFallbackDirsPsr4\n");
	RETURN_ZVAL(fallbackDirsPsr4Value, 1, 0);
}

PHP_METHOD(ClassLoader, getClassMap) {
	zval *pThis, *classMapValue;

	pThis = getThis();

	classMapValue = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("classMap"), 0 TSRMLS_CC);

	RETURN_ZVAL(classMapValue, 1, 0);
	//php_printf("这是getFallbackDirs\n");

}

/*{{{ proto addClassMap(array $classMap)*/
PHP_METHOD(ClassLoader, addClassMap) {
	zval *pThis, *classMapValue, *_classMapValue;

	pThis = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &_classMapValue) == FAILURE ) {
		return;
	} else {
		classMapValue = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("classMap"), 0 TSRMLS_CC);

		int isEmpty;

		isEmpty = (zend_hash_num_elements(Z_ARRVAL_P(classMapValue))) ? 1 : 0;
		if(isEmpty) {
			//array_merge
			//php_printf("classMap不为空\n");
			php_array_merge(Z_ARRVAL_P(classMapValue), Z_ARRVAL_P(_classMapValue), 0 TSRMLS_CC);
		} else {
			//php_printf("classMap为空\n");
			zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("classMap"), _classMapValue TSRMLS_CC);
		}
		zval_ptr_dtor(&_classMapValue);
	}

}	
/*}}}*/

PHP_METHOD(ClassLoader, add) {
	zval *pThis, *fallbackDirsPsr0Value, *prefixesPsr0Value;
	char *prefix;
	uint prefixLen;
	char *first;
	zval *arg_paths;
	zval *paths;
	zend_bool prepend = 0;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|b", &prefix, &prefixLen, &arg_paths, &prepend) == FAILURE) {
		return;
	}

	convert_to_array(arg_paths);

	MAKE_STD_ZVAL(paths);
	array_init(paths);
	ZVAL_ZVAL(paths, arg_paths, 1, 0);
	
	pThis = getThis();

	fallbackDirsPsr0Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr0"), 0 TSRMLS_CC);

	//php_printf("prepend : %d prefixLen:%d\n is_array:%d\n", prepend, prefixLen, Z_TYPE_P(paths) == IS_ARRAY);
	//判断字符串是否为空
	if(!prefixLen) {
		if(prepend) {
			php_array_merge(Z_ARRVAL_P(paths), Z_ARRVAL_P(fallbackDirsPsr0Value), 0 TSRMLS_CC);
			zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr0"), paths TSRMLS_CC);
			//php_printf("追加.....\n");
		} else {
			php_array_merge(Z_ARRVAL_P(fallbackDirsPsr0Value), Z_ARRVAL_P(paths), 0 TSRMLS_CC);
			//php_printf("不追加.....\n");
			zval_ptr_dtor(&paths);
		}
		return;
	}
	first = estrndup(prefix, 1);
	//php_printf("first:%s\n", first);
	zval *arrPrefix;
	MAKE_STD_ZVAL(arrPrefix);
	array_init(arrPrefix);
	zval **tmp;


	prefixesPsr0Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixesPsr0"), 0 TSRMLS_CC);
	//(!isset($prefixesPsr0[$first]))
	if(zend_hash_find(Z_ARRVAL_P(prefixesPsr0Value), first, 2, (void **) &tmp) == FAILURE) {
		//$arr[$prefix] = $paths;
		//
		// zval *test;
		// MAKE_STD_ZVAL(test);
		// array_init(test);
  		// add_next_index_string(test, "hello", 1); 
		//php_printf("没有找到了首字母\n");
		add_assoc_zval(arrPrefix, prefix, paths);
		//$prefixesPsr0[$prefix[0]][$prefix] = $paths;
		add_assoc_zval(prefixesPsr0Value, first, arrPrefix);
		return;
	} else {
		//php_printf("找到了首字母\n");
		arrPrefix = *tmp;
		if(zend_hash_find(Z_ARRVAL_P(arrPrefix), prefix, strlen(prefix) + 1, (void **) &tmp) == FAILURE) {
			add_assoc_zval(arrPrefix, prefix, paths);
			return;
		}
		arrPrefix = *tmp;
		if(prepend) {
			php_array_merge(Z_ARRVAL_P(paths), Z_ARRVAL_P(arrPrefix), 0 TSRMLS_CC);
			ZVAL_ZVAL(arrPrefix, paths, 1, 0);
		} else {
			php_array_merge(Z_ARRVAL_P(arrPrefix), Z_ARRVAL_P(paths), 0 TSRMLS_CC);
		}
		zval_ptr_dtor(&paths);
		return;
	}
}

PHP_METHOD(ClassLoader, addPsr4) {
	zval *pThis, *fallbackDirsPsr4Value, *prefixDirsPsr4Value, *prefixLengthsPsr4Value;
	char *prefix;
	uint prefixLen;
	char *first;
	zval *arg_paths;
	zval *paths;
	zend_bool prepend = 0;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|b", &prefix, &prefixLen, &arg_paths, &prepend) == FAILURE) {
		return;
	}

	convert_to_array(arg_paths);

	MAKE_STD_ZVAL(paths);
	array_init(paths);
	ZVAL_ZVAL(paths, arg_paths, 1, 0);

	zval **tmp;

	pThis = getThis();

	zval *arrPrefix;
	MAKE_STD_ZVAL(arrPrefix);
	array_init(arrPrefix);

	prefixDirsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixDirsPsr4"), 0 TSRMLS_CC);
	//php_printf("prefixLen:%d\n", prefixLen);
	if(!prefixLen) {

		fallbackDirsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr4"), 0 TSRMLS_CC);
		
		if(prepend) {
			php_array_merge(Z_ARRVAL_P(paths), Z_ARRVAL_P(fallbackDirsPsr4Value), 0 TSRMLS_CC);
			zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr4"), paths TSRMLS_CC);
		} else {
			php_array_merge(Z_ARRVAL_P(fallbackDirsPsr4Value), Z_ARRVAL_P(paths), 0 TSRMLS_CC);
			//php_printf("不追加.....\n");
			zval_ptr_dtor(&paths);
		}
	} else if(zend_hash_find(Z_ARRVAL_P(prefixDirsPsr4Value), prefix, strlen(prefix) + 1, (void **) &tmp) == FAILURE) {
		//php_printf("没有找到preifx\n");
  		int length = strlen(prefix);

  		if(prefix[length - 1] != '\\') {
  			zend_throw_exception(ClassLoader_ce, " A non-empty PSR-4 prefix must end with a namespace separator.", 0 TSRMLS_DC);
  		}

  		prefixLengthsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixLengthsPsr4"), 0 TSRMLS_CC);

		add_assoc_long(arrPrefix, prefix, length);
		add_assoc_zval(prefixLengthsPsr4Value, estrndup(prefix, 1), arrPrefix);
  		//$this->prefixLengthsPsr4[$prefix[0]][$prefix] = $length;
       	add_assoc_zval(prefixDirsPsr4Value, prefix, paths);
        //$this->prefixDirsPsr4[$prefix] = (array) $paths;
       
	} else {
		//php_printf("找到了preifx\n");
		arrPrefix = *tmp;
		if(prepend) {
			php_array_merge(Z_ARRVAL_P(paths), Z_ARRVAL_P(arrPrefix), 0 TSRMLS_CC);
			ZVAL_ZVAL(arrPrefix, paths, 1, 0);
			//add_assoc_zval(prefixDirsPsr4Value, estrndup(prefix, strlen(prefix)), paths);
			//zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixesPsr4"), paths TSRMLS_CC);
		} else {
			php_array_merge(Z_ARRVAL_P(arrPrefix), Z_ARRVAL_P(paths), 0 TSRMLS_CC);
			//add_assoc_zval(prefixDirsPsr4Value, estrndup(prefix, strlen(prefix)), arrPrefix);
			//zval_ptr_dtor(&paths);
		}
	}
}

PHP_METHOD(ClassLoader, set) {
	zval *pThis, *prefixesPsr0Value;
	char *prefix;
	uint prefixLen;
	char *first;
	zval *arg_paths;
	zval *paths;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|", &prefix, &prefixLen, &arg_paths) == FAILURE) {
		return;
	}
	convert_to_array(arg_paths);

	MAKE_STD_ZVAL(paths);
	array_init(paths);
	ZVAL_ZVAL(paths, arg_paths, 1, 0);

	zval **tmp;

	pThis = getThis();

	zval *arrPrefix;
	MAKE_STD_ZVAL(arrPrefix);
	array_init(arrPrefix);

	if(!prefixLen) {
		zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr0"), paths TSRMLS_CC);
	} else {
		add_assoc_zval(arrPrefix, prefix, paths);
		//$this->prefixesPsr0;
		prefixesPsr0Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixesPsr0"), 0 TSRMLS_CC);
		add_assoc_zval(prefixesPsr0Value, estrndup(prefix, 1), arrPrefix);
	}

}

PHP_METHOD(ClassLoader, setPsr4) {
	zval *pThis, *prefixDirsPsr4Value, *prefixLengthsPsr4Value;
	char *prefix;
	uint prefixLen;
	char *first;
	zval *arg_paths;
	zval *paths;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|", &prefix, &prefixLen, &arg_paths) == FAILURE) {
		return;
	}
	convert_to_array(arg_paths);

	MAKE_STD_ZVAL(paths);
	array_init(paths);
	ZVAL_ZVAL(paths, arg_paths, 1, 0);

	zval **tmp;

	pThis = getThis();

	zval *arrPrefix;
	MAKE_STD_ZVAL(arrPrefix);
	array_init(arrPrefix);

	if(!prefixLen) {
		zend_update_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr4"), paths TSRMLS_CC);
	} else {
  		int length = strlen(prefix);

  		if(prefix[length - 1] != '\\') {
  			zend_throw_exception(ClassLoader_ce, " A non-empty PSR-4 prefix must end with a namespace separator.", 0 TSRMLS_DC);
  		}
  		prefixDirsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixDirsPsr4"), 0 TSRMLS_CC);
    	prefixLengthsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixLengthsPsr4"), 0 TSRMLS_CC);

		add_assoc_long(arrPrefix, prefix, length);
		add_assoc_zval(prefixLengthsPsr4Value, estrndup(prefix, 1), arrPrefix);
  		//$this->prefixLengthsPsr4[$prefix[0]][$prefix] = $length;
       	add_assoc_zval(prefixDirsPsr4Value, prefix, paths);
        //$this->prefixDirsPsr4[$prefix] = (array) $paths;
	}
}

PHP_METHOD(ClassLoader, setUseIncludePath) {
	zval *pThis;
	zend_bool useIncludePath;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b|", &useIncludePath) == FAILURE) {
		return;
	}
	pThis = getThis();
	zend_update_property_bool(Z_OBJCE_P(pThis), pThis, ZEND_STRL("useIncludePath"), useIncludePath TSRMLS_CC);
}

PHP_METHOD(ClassLoader, getUseIncludePath){
	zval *pThis, *useIncludePathValue;

	pThis = getThis();

	useIncludePathValue = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("useIncludePath"), 0 TSRMLS_CC);

	RETURN_ZVAL(useIncludePathValue, 1, 0);
}

PHP_METHOD(ClassLoader, setClassMapAuthoritative){
	zval *pThis;
	zend_bool classMapAuthoritative;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b|", &classMapAuthoritative) == FAILURE) {
		return;
	}
	pThis = getThis();
	zend_update_property_bool(Z_OBJCE_P(pThis), pThis, ZEND_STRL("classMapAuthoritative"), classMapAuthoritative TSRMLS_CC);
}

PHP_METHOD(ClassLoader, isClassMapAuthoritative){
	zval *pThis, *classMapAuthoritativeValue;

	pThis = getThis();

	classMapAuthoritativeValue = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("classMapAuthoritative"), 0 TSRMLS_CC);

	RETURN_ZVAL(classMapAuthoritativeValue, 1, 0);
}

/*spl_autoload_register(array($this, 'loadClass'), true, $prepend);*/
PHP_METHOD(ClassLoader, register){
	zval *pThis = getThis();
	zval *prepend;
	zval *isThrow;
	zval *function;

	zval *autoload;
	zval *retval;

	MAKE_STD_ZVAL(retval);
	//array_init(retval);

	MAKE_STD_ZVAL(isThrow);
	ZVAL_BOOL(isThrow, 1);

	MAKE_STD_ZVAL(autoload);
	array_init(autoload);


	add_next_index_zval(autoload, pThis);
	add_next_index_string(autoload, "loadClass", 1);

	MAKE_STD_ZVAL(function);
	ZVAL_STRING(function, "spl_autoload_register", 0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &prepend) == FAILURE) {
		return;
	}
	zval **params = (zval**)malloc(sizeof(zval));
	params[0] = autoload;
	params[1] = isThrow;
	params[2] = prepend;

	if(call_user_function(CG(function_table),NULL, function, retval, 3, params TSRMLS_CC)==FAILURE){  
       	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to register autoload function %s", "loadClass");
    }

    zval_ptr_dtor(&autoload);
    zval_ptr_dtor(&isThrow);
    zval_ptr_dtor(&prepend);
    zval_ptr_dtor(params);

    return;
}

PHP_METHOD(ClassLoader, unregister){
	zval *pThis = getThis();

	zval *function;

	zval *autoload;
	zval *retval;

	MAKE_STD_ZVAL(retval);
	array_init(retval);


	MAKE_STD_ZVAL(autoload);
	array_init(autoload);


	add_next_index_zval(autoload, pThis);
	add_next_index_string(autoload, "loadClass", 1);

	MAKE_STD_ZVAL(function);
	ZVAL_STRING(function, "spl_autoload_unregister", 0);


	zval **params = (zval**)malloc(sizeof(zval));
	params[0] = autoload;


	if(call_user_function(CG(function_table),NULL, function, retval, 1, params TSRMLS_CC)==FAILURE){  
       	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to register autoload function %s", "loadClass");
    }
    zval_ptr_dtor(&autoload);
    zval_ptr_dtor(&function);
    zval_ptr_dtor(params);
    if(retval) {
    	zval_ptr_dtor(&retval);
    }

    return ;
}

PHP_METHOD(ClassLoader, loadClass){
	char *className;
	int classNameLength;

	zval *pThis = getThis();
	
	zval *retval, *arg1;

	zval **params = (zval**)malloc(sizeof(zval));

	zval *function;

	MAKE_STD_ZVAL(function);
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|", &className, &classNameLength) == FAILURE) {
		return;
	}
	MAKE_STD_ZVAL(retval);
	MAKE_STD_ZVAL(arg1);

	ZVAL_STRINGL(arg1, className, classNameLength, 1);
	ZVAL_STRINGL(function, "findFile", 8, 1);
	//zend_call_method_with_1_params(&pThis, ClassLoader_ce, NULL, "findFile", &retval, arg1);
	params[0] = arg1;
	call_user_function(&(ClassLoader_ce)->function_table, &pThis, function, retval, 1, params TSRMLS_CC);

	if(Z_TYPE_P(retval) == IS_STRING && Z_STRVAL_P(retval) != NULL){

		ZVAL_STRING(function, "includeFile", 1);
		//php_printf("调用函数:%s\n", Z_STRVAL_P(retval));
		params[0] = retval;
		if(call_user_function(EG(function_table), NULL, function, retval, 1, params TSRMLS_CC)==FAILURE){  
	       	php_error_docref(NULL TSRMLS_CC, E_WARNING, "includeFile is not exists");
	    }
		RETVAL_BOOL(1);
	}

	zval_ptr_dtor(&arg1);
	zval_ptr_dtor(&function);
	zval_ptr_dtor(params);

}

PHP_METHOD(ClassLoader, findFile){
	char *className;
	int classNameLength;

	zval *pThis = getThis();
	zval *classMapValue, *classMapAuthoritativeValue;

	zval **tmp;

	zval *retval, *arg1, *arg2;
	


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|", &className, &classNameLength) == FAILURE) {
		return;
	}

	if(*className == '\\') {
		className = className + 1;
	}

	classMapValue = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("classMap"), 0 TSRMLS_CC);
	if(zend_hash_find(Z_ARRVAL_P(classMapValue), className, strlen(className) + 1, (void **)&tmp) == SUCCESS) {
		RETURN_STRING(Z_STRVAL_PP(tmp), 1);
	}

	classMapAuthoritativeValue = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("classMapAuthoritative"), 0 TSRMLS_CC);

	if(Z_BVAL_P(classMapAuthoritativeValue)) {
		RETURN_BOOL(0);
	}

	zval **params = (zval**)malloc(sizeof(zval));
	zval *function;

	MAKE_STD_ZVAL(function);
	ZVAL_STRINGL(function, "findFileWithExtension", strlen("findFileWithExtension"), 1);
	MAKE_STD_ZVAL(retval);
	MAKE_STD_ZVAL(arg1);
	ZVAL_STRINGL(arg1, className, classNameLength, 1);
	MAKE_STD_ZVAL(arg2);
	ZVAL_STRINGL(arg2, ".php", 4, 1);
	//php_printf("arg1->className:%s, %s\n", Z_STRVAL_P(arg1), className);
	params[0] = arg1;
	params[1] = arg2;

	//zend_call_method_with_2_params(&pThis, ClassLoader_ce, NULL, "findFileWithExtension", &retval, arg1, arg2);
	call_user_function(&(ClassLoader_ce)->function_table, &pThis, function, retval, 2, params TSRMLS_CC);

	if(Z_TYPE_P(retval) == IS_NULL) {

		zval *hhvm;
		zval *ret;
		zval **tmp_params = (zval**)malloc(sizeof(zval));
		zval *tmp_arg;
		MAKE_STD_ZVAL(tmp_arg);
		ZVAL_STRINGL(tmp_arg, "HHVM_VERSION", strlen("HHVM_VERSION"), 0);
		tmp_params[0] = tmp_arg;
		MAKE_STD_ZVAL(ret);
		MAKE_STD_ZVAL(hhvm);
		ZVAL_STRINGL(hhvm, "defined", strlen("defined"), 0);
		if(call_user_function(CG(function_table), NULL, hhvm, ret, 1, tmp_params TSRMLS_CC) ==SUCCESS) {
			//php_printf("z_type:%d\n", Z_TYPE_P(ret));
			if(Z_BVAL_P(ret)) {
				ZVAL_STRINGL(arg2, ".hh", 3, 0);
				//zend_call_method_with_2_params(&pThis, ClassLoader_ce, NULL, "findFileWithExtension", &retval, arg1, arg2);
				call_user_function(&(ClassLoader_ce)->function_table, &pThis, function, retval, 2, params TSRMLS_CC);
			}			
		}
		zval_ptr_dtor(tmp_params);
		zval_ptr_dtor(&hhvm);
		zval_ptr_dtor(&tmp_arg);
	}

	if(Z_TYPE_P(retval) == IS_NULL) {
		zval *value;

		MAKE_STD_ZVAL(value);
		ZVAL_BOOL(value, 0);

		zend_hash_update(Z_ARRVAL_P(classMapValue), className, strlen(className) + 1, (void **)&value, sizeof(zval *), NULL);
		RETURN_BOOL(0);
		zval_ptr_dtor(&value);
	}
	zval_ptr_dtor(&arg1);
	zval_ptr_dtor(&arg2);
	zval_ptr_dtor(&function);
	zval_ptr_dtor(params);
	//zval_ptr_dtor(tmp);
	RETURN_STRING(Z_STRVAL_P(retval), 1);
}

PHP_METHOD(ClassLoader, findFileWithExtension){
	zval *pThis, *prefixLengthsPsr4Value, *prefixesPsr0Value, **tmp, *prefixDirsPsr4Value, *useIncludePath;
	zval *fallbackDirsPsr4Value, *fallbackDirsPsr0Value;
	char *file;
	zval exists_flag;
	zval **ppzval;	
	
	char *className, *ext, *logicalPathPsr4, *logicalPathPsr0;

	char *first, default_slash[2];

	int classNameLength, extLength;


	pThis = getThis();

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|", &className, &classNameLength, &ext, &extLength) == FAILURE) {
		return;
	}
	
	sprintf(default_slash, "%c", DEFAULT_SLASH);

	//char *php_strtr(char *str, int len, char *str_from, char *str_to, int trlen)
	

	//php_printf("className:%s\n", className);

	logicalPathPsr4 = estrndup(className, strlen(className)+extLength);

	if(extLength) {
		logicalPathPsr4 = strncat(logicalPathPsr4, ext, extLength);
	}

	php_strtr(logicalPathPsr4, strlen(logicalPathPsr4), "\\", default_slash, 1);

	//php_printf("logicalPathPsr4:%s\n", logicalPathPsr4);

	first = estrndup(className, 1);

	prefixLengthsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixLengthsPsr4"), 0 TSRMLS_CC);

	if(zend_hash_find(Z_ARRVAL_P(prefixLengthsPsr4Value), first, 2, (void **) &tmp) == SUCCESS) {
		HashTable *ht = Z_ARRVAL_P(*tmp);
		char *key;
		uint keyLen = 0;
		ulong idx = 0;
		const char *found = NULL;

		for(zend_hash_internal_pointer_reset(ht);
			zend_hash_get_current_data(ht, (void **) &tmp) == SUCCESS;
			zend_hash_move_forward(ht)) {
			zend_hash_get_current_key_ex(ht, &key, &keyLen, &idx, 0, NULL);
			
			if (zend_hash_get_current_data(ht, (void**)&ppzval) == FAILURE) {
				continue;
			}

			//php_printf("key:%s, keyLen:%d className:%s\n, length:%ld", key, keyLen, className, Z_LVAL_PP(ppzval));
			found = php_memnstr(className,
			                key,
			                keyLen - 1,
			                className + strlen(className));
			if(found) {
				prefixDirsPsr4Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixDirsPsr4"), 0 TSRMLS_CC);
				zval **ptmp;
				if(zend_hash_find(Z_ARRVAL_P(prefixDirsPsr4Value), key, keyLen+1, (void **) &ptmp) == SUCCESS) {
					HashTable *pht = Z_ARRVAL_P(*ptmp);
					zval **pppzval;
					int len;

					for(zend_hash_internal_pointer_reset(pht);
						zend_hash_get_current_data(pht, (void **) &ptmp) == SUCCESS;
						zend_hash_move_forward(pht)) {
						if (zend_hash_get_current_data(pht, (void**)&pppzval) == FAILURE) {
							continue;
						}

						if(strlen(logicalPathPsr4) - Z_LVAL_PP(ppzval) < 0) {
							len = strlen(logicalPathPsr4);
						} else {
							len = Z_LVAL_PP(ppzval);
						}

						spprintf(&file, 0, "%s%c%s", Z_STRVAL_PP(pppzval), DEFAULT_SLASH, logicalPathPsr4+len);
						//php_printf("file:%s\n", file);
						php_stat(file, strlen(file), FS_EXISTS, &exists_flag TSRMLS_CC);
						if (Z_BVAL(exists_flag)) {
							RETURN_STRING(file, 1);
						} 
					}
				}
			}
		}
	}

	// PSR-4 fallback dirs
	fallbackDirsPsr4Value =  zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr4"), 0 TSRMLS_CC);
	for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(fallbackDirsPsr4Value));
	zend_hash_get_current_data(Z_ARRVAL_P(fallbackDirsPsr4Value), (void **) &ppzval) == SUCCESS;
	zend_hash_move_forward(Z_ARRVAL_P(fallbackDirsPsr4Value))) {
		if (zend_hash_get_current_data(Z_ARRVAL_P(fallbackDirsPsr4Value), (void**)&ppzval) == SUCCESS) {
			spprintf(&file, 0, "%s%c%s", Z_STRVAL_PP(ppzval), DEFAULT_SLASH, logicalPathPsr4);
			//php_printf("file:%s\n", file);
			php_stat(file, strlen(file), FS_EXISTS, &exists_flag TSRMLS_CC);
			if (Z_BVAL(exists_flag)) {
				RETURN_STRING(file, 1);
			} 
		}
	}

	// PSR-0 lookup
	char *e = className + classNameLength;
	int pos;
	int found = 0 ;
	while(e >= className) {
		if(*e == '\\') {
			pos = e - className;
			found = 1;
			break;
		}
		e--;
	}

	if(found) {
		char *preStr, *endStr;
		if(strlen(logicalPathPsr4) > (pos + 1)) {
			preStr = estrndup(logicalPathPsr4, pos+1);
			endStr = estrndup(logicalPathPsr4 + pos + 1, strlen(logicalPathPsr4) - pos - 1);
			//php_printf("preStr:%s, endStr:%s\n", preStr, endStr);
			spprintf(&logicalPathPsr0, 0, "%s%s", preStr, php_strtr(endStr , strlen(endStr), "_", default_slash, 1));

			efree(preStr);
			efree(endStr);
			
		}  else {
			preStr = estrndup(logicalPathPsr4, strlen(logicalPathPsr4));
			endStr = NULL;
			logicalPathPsr0 = preStr;
		}
		//php_printf("找到了:%d\n", pos);
	} else {
		//php_printf("没有找到\n");
		logicalPathPsr0 = estrndup(className, strlen(className));
		php_strtr(logicalPathPsr0, strlen(logicalPathPsr0), "_", default_slash, 1);

	}

	//php_printf("logicalPathPsr0:%s\n", logicalPathPsr0);

	prefixesPsr0Value = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("prefixesPsr0"), 0 TSRMLS_CC);

	if(zend_hash_find(Z_ARRVAL_P(prefixesPsr0Value), first, 2, (void **) &tmp) == SUCCESS) {
		HashTable *ht = Z_ARRVAL_P(*tmp);
		char *key;
		uint keyLen = 0;
		ulong idx = 0;
		const char *found = NULL;

		for(zend_hash_internal_pointer_reset(ht);
			zend_hash_get_current_data(ht, (void **) &tmp) == SUCCESS;
			zend_hash_move_forward(ht)) {
			zend_hash_get_current_key_ex(ht, &key, &keyLen, &idx, 0, NULL);
			
			if (zend_hash_get_current_data(ht, (void**)&ppzval) == FAILURE) {
				continue;
			}

			//php_printf("key:%s, keyLen:%d className:%s\n, length:%ld", key, keyLen, className, Z_LVAL_PP(ppzval));
			found = php_memnstr(className,
			                key,
			                keyLen - 1,
			                className + strlen(className));
			if(found) {
				HashTable *pht = Z_ARRVAL_P(*ppzval);
				zval **pppzval, **ptmp;
				int len;

				for(zend_hash_internal_pointer_reset(pht);
					zend_hash_get_current_data(pht, (void **) &ptmp) == SUCCESS;
					zend_hash_move_forward(pht)) {
					if (zend_hash_get_current_data(pht, (void**)&pppzval) == FAILURE) {
						continue;
					}
					spprintf(&file, 0, "%s%c%s", Z_STRVAL_PP(pppzval), DEFAULT_SLASH, logicalPathPsr0);
					//php_printf("file:%s\n", file);
					php_stat(file, strlen(file), FS_EXISTS, &exists_flag TSRMLS_CC);
					if (Z_BVAL(exists_flag)) {
						RETURN_STRING(file, 1);
					} 
				}
			}
		}
	}

	fallbackDirsPsr0Value =  zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("fallbackDirsPsr0"), 0 TSRMLS_CC);
	for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(fallbackDirsPsr0Value));
	zend_hash_get_current_data(Z_ARRVAL_P(fallbackDirsPsr0Value), (void **) &ppzval) == SUCCESS;
	zend_hash_move_forward(Z_ARRVAL_P(fallbackDirsPsr0Value))) {
		if (zend_hash_get_current_data(Z_ARRVAL_P(fallbackDirsPsr0Value), (void**)&ppzval) == SUCCESS) {
			spprintf(&file, 0, "%s%c%s", Z_STRVAL_PP(ppzval), DEFAULT_SLASH, logicalPathPsr4);
			//php_printf("file:%s\n", file);
			php_stat(file, strlen(file), FS_EXISTS, &exists_flag TSRMLS_CC);
			if (Z_BVAL(exists_flag)) {
				RETURN_STRING(file, 1);
			} 
		}
	}

	useIncludePath = zend_read_property(Z_OBJCE_P(pThis), pThis, ZEND_STRL("useIncludePath"), 0 TSRMLS_CC);

	file = zend_resolve_path(logicalPathPsr0, strlen(logicalPathPsr0) TSRMLS_CC);
	if(file != NULL && useIncludePath != NULL) {
		RETURN_STRING(file, 1);	
	}

}

static zend_function_entry class_method[] = {
	ZEND_ME(ClassLoader,	__construct,	NULL,	ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	ZEND_ME(ClassLoader,	getPrefixes,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	getPrefixesPsr4,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	getFallbackDirs,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	getFallbackDirsPsr4,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	getClassMap,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	addClassMap,	addClassMap_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	add,	add_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	addPsr4,	addPsr4_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	set,	set_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	setPsr4,	setPsr4_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	setUseIncludePath,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	getUseIncludePath,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	setClassMapAuthoritative,	setClassMapAuthoritative_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	isClassMapAuthoritative,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	register,	register_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	unregister,	NULL,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	loadClass,	loadClass_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	findFile,	findFile_arginfo,	ZEND_ACC_PUBLIC)
	ZEND_ME(ClassLoader,	findFileWithExtension,	findFileWithExtension_arginfo,	ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(lumen_ClassLoader)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	//定义并注册一个类
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "ClassLoader", class_method);
	//定义一个包含命名空间的类
	//INIT_NS_CLASS_ENTRY(ce, "Composer\\Autoload", "ClassLoader", class_method);

	ClassLoader_ce = zend_register_internal_class(&ce TSRMLS_CC);
	
	//定义属性
	// PSR-4
    // private $prefixLengthsPsr4 = array();
    // private $prefixDirsPsr4 = array();
    // private $fallbackDirsPsr4 = array();

    // // PSR-0
    // private $prefixesPsr0 = array();
    // private $fallbackDirsPsr0 = array();

    // private $useIncludePath = false;
    // private $classMap = array();

    // private $classMapAuthoritative = false;
	//zend_declare_property_string(ClassLoader_ce, ZEND_STRL("prefixLengthsPsr4"), "nopsky", ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(ClassLoader_ce, ZEND_STRL("prefixLengthsPsr4"), ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(ClassLoader_ce, ZEND_STRL("prefixDirsPsr4"), ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(ClassLoader_ce, ZEND_STRL("fallbackDirsPsr4"), ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(ClassLoader_ce, ZEND_STRL("prefixesPsr0"), ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(ClassLoader_ce, ZEND_STRL("fallbackDirsPsr0"), ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_bool(ClassLoader_ce, ZEND_STRL("useIncludePath"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(ClassLoader_ce, ZEND_STRL("classMap"), ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_bool(ClassLoader_ce, ZEND_STRL("classMapAuthoritative"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(lumen_ClassLoader)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(lumen_ClassLoader)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(lumen_ClassLoader)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(lumen_ClassLoader)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "lumen_ClassLoader support", "enabled");
	php_info_print_table_header(2, "lumen_ClassLoader version", "1.0");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ lumen_ClassLoader_functions[]
 *
 * Every user visible function must have an entry in lumen_ClassLoader_functions[].
 */
const zend_function_entry lumen_ClassLoader_functions[] = {
	PHP_FE(includeFile,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in lumen_ClassLoader_functions[] */
};
/* }}} */

/* {{{ lumen_ClassLoader_module_entry
 */
zend_module_entry lumen_ClassLoader_module_entry = {
	STANDARD_MODULE_HEADER,
	"lumen_ClassLoader",
	lumen_ClassLoader_functions,
	PHP_MINIT(lumen_ClassLoader),
	PHP_MSHUTDOWN(lumen_ClassLoader),
	PHP_RINIT(lumen_ClassLoader),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(lumen_ClassLoader),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(lumen_ClassLoader),
	PHP_LUMEN_CLASSLOADER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LUMEN_CLASSLOADER
ZEND_GET_MODULE(lumen_ClassLoader)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
