<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('timecop')) {
	dl('timecop.' . PHP_SHLIB_SUFFIX);
}
$module = 'timecop';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = 'timecop_time';
if (extension_loaded($module)) {
    echo timecop_time()."$br\n";
    $_SERVER['REQUEST_TIME'] = PHP_INT_MAX;
    echo timecop_time()."$br\n";
} else {
    echo "Module $module is not compiled into PHP";
}

?>
