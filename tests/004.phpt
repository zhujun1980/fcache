--TEST--
Check for sub dir
--SKIPIF--
<?php if (!extension_loaded("fcache")) print "skip"; ?>
--FILE--
<?php
ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);

class C {
    public $txt;
    public $idx;
    public $dd;
    public $db;

    public $setid;
    public $data;

    function __construct($data, $db) {
        $dir = dirname(__FILE__);
        ini_set('fcache.data_dir', $dir);
        $this->setid = "temp";
        $this->dd = $dir . DIRECTORY_SEPARATOR . $db;
        mkdir($this->dd);
        $this->txt = $this->dd . DIRECTORY_SEPARATOR . $this->setid . ".txt";
        $this->idx = $this->dd . DIRECTORY_SEPARATOR . $this->setid . "." . FCacheBloomFilter::$EXT_NAME;
        $this->renew($data);
    }

    function renew($data) {
        //$this->clean();
        $this->data = $data;
        $this->var  = 1;
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
        if(file_exists($this->dd))
            rmdir($this->dd);
    }
}

function check($setid, $data, $exists, $db) {
    foreach($data as $id) {
        $ret = FCacheBloomFilter::isPresent($setid, $id, $db);
        if($exists)
            assert($ret);
        else
            assert(!$ret);
    }
}

$db = 't';
$data_rand = array();
for($i = 0; $i < 1000; $i++)
    $data_rand[] = $i + 1;

$c = new C($data_rand, $db);
echo "TXT SUCC: " . file_exists($c->txt) . "\n";
echo "IDX SUCC: " . file_exists($c->idx) . "\n";
echo "DIR SUCC: " . file_exists($c->dd) . "\n";
check($c->setid, $data_rand, true, $db);

//check($c->setid, $data_rand, false, "t2");
//check($c->setid, $data_rand, false, "..");
?>
--EXPECT--
TXT SUCC: 1
IDX SUCC: 1
DIR SUCC: 1

