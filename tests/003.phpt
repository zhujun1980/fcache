--TEST--
Check for modify data
--SKIPIF--
<?php if (!extension_loaded("fcache")) print "skip"; ?>
--FILE--
<?php
ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);

class C {
    public $txt;
    public $idx;
    public $setid;
    public $data;
    public $var;

    function __construct($data, $var) {
        $dir = dirname(__FILE__);
        ini_set('fcache.data_dir', $dir);
        $this->setid = "temp";
        $this->txt = $dir . DIRECTORY_SEPARATOR . $this->setid . ".txt";
        $this->idx = $dir . DIRECTORY_SEPARATOR . $this->setid . "." . FCacheBloomFilter::$EXT_NAME;
        $this->renew($data, $var);
    }
    
    function renew($data, $var) {
        $this->clean();
        $this->data = $data;
        $this->var  = $var;
        $N = count($this->data);
        file_put_contents($this->txt, implode("\n", $this->data));
        $ret = FCacheBloomFilter::createIdx($this->txt, $this->idx, $N, $var);
    }

    function __destruct() {
        $this->clean();
    }

    function clean() {
        if(file_exists($this->txt))
            unlink($this->txt);
        if(file_exists($this->idx))
            unlink($this->idx);
    }
}

function check($setid, $data, $exists) {
    foreach($data as $id) {
        $ret = FCacheBloomFilter::isPresent($setid, $id);
        if($exists)
            assert($ret);
        else
            assert(!$ret);
    }
}

$dataVers = 0x100;
$data_rand = array();
for($i = 0; $i < 1000; $i++)
    $data_rand[] = $i + 1;

$dataVers2 = 0x200;
$big_data_rand = array();
for($i = 0; $i < 10000; $i++)
    $big_data_rand[] = $i + 10000;

$dataVers3 = 0x300;
$small_data_rand = array();
for($i = 0; $i < 100; $i++)
    $small_data_rand[] = $i + 100000;

$c = new C($data_rand, $dataVers);
echo "TXT SUCC: " . file_exists($c->txt) . "\n";
echo "IDX SUCC: " . file_exists($c->idx) . "\n";
check($c->setid, $data_rand, true);

$c->renew($big_data_rand, $dataVers2);
check($c->setid, $big_data_rand, true);

$c->renew($small_data_rand, $dataVers3);
check($c->setid, $small_data_rand, true);
?>
--EXPECT--
TXT SUCC: 1
IDX SUCC: 1
