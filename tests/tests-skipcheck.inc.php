<?php
extension_loaded('timecop') or die('skip timecop module not available');
if (isset($required_version)) {
    if (version_compare(PHP_VERSION, $required_version, "<")) {
        die("skip PHP ${required_version}+ required for this test");
    }
}
if (isset($required_func)) {
    foreach ($required_func as $func_name) {
        if (!function_exists($func_name)) {
            die("skip $func_name() function is not available.");
        }
    }
}
if (isset($required_class)) {
    foreach ($required_class as $class_name) {
        if (!class_exists($class_name)) {
            die("skip $class_name class is not available.");
        }
    }
}
if (isset($required_method)) {
    foreach ($required_method as $v) {
        list($class_name, $method_name) = $v;
        if (!method_exists($class_name, $method_name)) {
            die("skip $class_name::$method_name() method is not available.");
        }
    }
}
