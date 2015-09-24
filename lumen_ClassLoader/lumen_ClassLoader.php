<?php

$lumenPath = "/data/lumen/vendor/composer";
$loader = new \Composer\Autoload\ClassLoader();

$map = require $lumenPath . '/autoload_namespaces.php';
foreach ($map as $namespace => $path) {
    $loader->set($namespace, $path);
}

$map = require $lumenPath . '/autoload_psr4.php';
foreach ($map as $namespace => $path) {
    $loader->setPsr4($namespace, $path);
}

$classMap = require $lumenPath . '/autoload_classmap.php';
if ($classMap) {
    $loader->addClassMap($classMap);
}

$loader->register(true);

$includeFiles = require $lumenPath . '/autoload_files.php';
foreach ($includeFiles as $file) {
	echo $file."\n";
}

?>
