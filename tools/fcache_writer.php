<?php
ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);
ini_set('log_errors', 0);

//php fcache_writer.php /usr/home/zhujun5/data/small.txt /usr/home/zhujun5/data/small.bf 1000 1 8 32

$txtFile = $argv[1];
$idxFile = $argv[2];
$N = $argv[3];
$dataVers = $argv[4];
$hashCnt = $argv[5];
$bitsPerElement = $argv[6];

$ret = FCacheBloomFilter::createIdx($txtFile, $idxFile, $N, $dataVers, $hashCnt, $bitsPerElement);
assert($ret);

$statinfo = FCacheBloomFilter::stat($idxFile);
var_dump($statinfo);
return;
