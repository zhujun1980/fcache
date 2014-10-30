--TEST--
Check for bloom filter
--SKIPIF--
<?php if (!extension_loaded("fcache")) print "skip"; ?>
--FILE--
<?php
ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);

$data = array (
    2316867724,
    4068980303,
    2781348013,
    611697685,
    3459378032,
    703853544,
    2499439889,
    3979295859,
    66851218,
    1721452780,
);

$data_not_exists = array(
    354446825,
    3736975502,
    604894758,
    2394360897,
    1283694294,
    3441614561,
    209260319,
    4235429463,
    240632569,
    2533304094,    
);

function T($data, $dataVers, $data_not_exists) {
    $N = count($data);
    $dir = dirname(__FILE__);

    ini_set('fcache.data_dir', $dir);
    $setid = "temp";
    $txt = $dir . DIRECTORY_SEPARATOR . $setid . ".txt";
    $idx = $dir . DIRECTORY_SEPARATOR . $setid . "." . FCacheBloomFilter::$EXT_NAME;
    file_put_contents($txt, implode("\n", $data));

    $ret = FCacheBloomFilter::createIdx($txt, $idx, $N, $dataVers);
    var_dump($ret);
    
    $stat = FCacheBloomFilter::stat($idx);
    var_dump($stat["version"] == $dataVers);
    var_dump($stat["hash_cnt"] == 8);

    foreach($data as $id) {
        $ret = FCacheBloomFilter::isPresent($setid, intval($id));
        assert($ret);
    }
    foreach($data as $id) {
        $ret = FCacheBloomFilter::isPresent($setid, strval($id));
        assert($ret);
    }
    if(is_array($data_not_exists)) {
        foreach($data_not_exists as $id) {
            $ret = FCacheBloomFilter::isPresent($setid, $id);
            assert(!$ret);
        }
    }
    unlink($txt);
    unlink($idx);
}

$dataVers = 0x100;
T($data, $dataVers, $data_not_exists);

$data_rand = array();
for($i = 0; $i < 10000; $i++)
    $data_rand[] = rand();
$dataVers = 200;
T($data_rand, $dataVers, NULL);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
