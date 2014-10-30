<?php
//php fcache_checker.php m 1 < /Users/zhujun/source/BloomFilter/data/m.txt
//php fcache_checker.php small 0 < /Users/zhujun/source/BloomFilter/data/small2.txt

ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);
ini_set('log_errors', 0);

$f = fopen("php://stdin", "r");
$cnt = 0;
$setid = $argv[1];
$ccc = $argv[2];
$db = $argv[3];
$err = 0;

while(1) {
    $s = trim(fgets($f));
    if($s == '')
        break;
    $ret = FCacheBloomFilter::isPresent($setid, $s, $db);
	
    if($ccc) {
        assert($ret);
    }
    else {
        if($ret) {
            $err++;
            echo $s . "\n";
        }
        //assert(!$ret);
    }
    $cnt++;
    if(feof($f))
        break;
}
echo "Succ: " . $cnt . "\n";
echo "Err: " . $err . "\n";
