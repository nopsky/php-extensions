<?php
//define("HHVM_VERSION", "1.0");
//use Composer\Autoload\ClassLoader as ClassLoader;

$obj = new ClassLoader();

// $obj->add("abc", "/usr/local/libs", true);
// $obj->add("acd", "/usr/local/libs/nginx", true);
// $obj->add("acd", "/usr/local/libs/nginx2", false);
// var_dump($obj->getPrefixes());

// $a =  array();
// $a['a'] = array('abc'=>array("/usr/local/libs"), 'acd'=>array("/usr/local/libs/nginx"));
// $b = array("/usr/local/libs/nginx2");

// //$a['a']['acd'] = array_merge($b, $a['a']['acd']);
// $a['a']['acd'] = array_merge($a['a']['acd'], $b);
// var_dump($a['a']);
// 
// $obj->addPsr4("abc\\", "/usr/local/php", false);
// $obj->addPsr4("abc\\", "/usr/local/lua", true);
// $a = $obj->getPrefixesPsr4();
// $b = $obj->prefixLengthsPsr4;
// var_dump($a);
// var_dump($b);
// $obj->setPsr4("abc\\", "/usr/local/php");
// $a = $obj->getPrefixesPsr4();
// $b = $obj->prefixLengthsPsr4;
// var_dump($a);
// var_dump($b);
// 
// $obj->setClassMapAuthoritative(false);
// var_dump($obj->classMapAuthoritative);
// 
// $obj->register(true);
// $obj->unregister();

$obj->addPsr4("", "/data/php-5.6.9/ext/lumen_ClassLoader");
$obj->loadClass("test");
?>
