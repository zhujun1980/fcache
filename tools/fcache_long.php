<?php
//php fcache_long.php big
ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);
ini_set('log_errors', 0);

function bloomfilter($setid, $db, $d) {
    $t1 = microtime(true);
    if(is_array($d)) {
        foreach($d as $s) {
            $ret = FCacheBloomFilter::isPresent($setid, $s, $db);
            //assert($ret);
            var_dump($ret);
        }
    }
    else {
        $ret = FCacheBloomFilter::isPresent($setid, $d, $db);
        var_dump($ret);
    }
    $t2 = microtime(true);
    echo "Time: " . ($t2 - $t1) . "\n";
}

$setid 	= $argv[1];
$db 	= $argv[2];

$a = array (
    2147499710,
    2147499727,
);

while(1) {
    bloomfilter($setid, $db, $a);
    usleep(100);
}
