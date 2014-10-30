<?php
//php fcache.php half1
//php fcache.php small
//php fcache.php big
ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);
ini_set('log_errors', 0);

function bloomfilter($setid, $db, $d) {
    $t1 = microtime(true);
    if(is_array($d)) {
        foreach($d as $s) {
            $ret = FCacheBloomFilter::isPresent($setid, $s, $db);
            assert($ret);
        }
    }
    else {
        $ret = FCacheBloomFilter::isPresent($setid, $d, $db);
        assert($ret);
    }
    $t2 = microtime(true);
    echo "Time: " . ($t2 - $t1) . "\n";
}

$setid = $argv[1];
$db = "";
$key = $argv[2];

$a = array("1100730460",
            "3588190085",
            "3869802378",
            "3725196184",
            "3099999890",
            1574713088,
            3312357142,
            253984121,
            245594748,
            3153882355,
            513484685,
            45435489,
            445593241,
            2947864180,
            1742261858,
            1520831132,
            4088548844,
            2000520278,
            1118885358,
            2958833893,
);

if(!empty($key))
    bloomfilter($setid, $db, $key);
bloomfilter($setid, $db, "2457658764");
bloomfilter($setid, $db, $a);
