<?php
dl("fcache.so");
ini_set('fcache.data_dir', '/Users/zhujun/source/sina/BloomFilter/data');
ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);
ini_set('log_errors', 0);

$setid = $argv[1];
$db = $argv[2];
$key = $argv[3];

$data = array (
    "abcdefghi",
    "1",
    78,
    array (
        "a" => "x",
        "b" => "y",
        "c" => "z",
    ),
    "abc" => "cba",
    1,
    "lala" => array(1, 2, 3),
    4,5,
    NULL,
    "浅粉红",
);
//$data[] = $data1;

$fa = NULL;
$filepath = trim($argv[1]);

var_dump($data);

//$fa = FixedArray::fromArray($data);
//t($fa, $filepath);

$id = 1;
$db = "pa";
$fa = FixedArray::fromFile($id, $db);
t($fa, "/Users/zhujun/source/sina/BloomFilter/data/pa/2.pa");

function t($fa, $filepath) {
    if($filepath != NULL) {
        $ret = $fa->toFile($filepath);
        echo "Save:\t";
        var_dump($ret);
    }

    $ret = $fa->count();
    echo "Count:\t";
    var_dump($ret);
    
    $ret = $fa->offsetGet(8);
    echo "Get:\t";
    var_dump($ret);

    echo "DGet:\t";
    var_dump($fa[1]);
    
    $ret = $fa->offsetExists("lala");
    echo "Exists:\t";
    var_dump($ret);
    
    $ret = $fa->offsetExists("abc");
    echo "Exists:\t";
    var_dump($ret);

    $ret = $fa->offsetExists("abcasdfs");
    echo "Exists:\t";
    var_dump($ret);

    //$fa[1] = 2;

    //$fa->offsetUnset(NULL);
    //$fa->offsetSet(NULL);

    $fa->rewind();
    echo "for loop:\n";
    for(; $fa->valid(); $fa->next()) {
        echo  "Key:\t" . $fa->key() . " = " . ($fa->current()) . "\n";
    }
    
    echo "foreach loop1:\n";
    foreach($fa as $v) {
        echo  $v . "\n";
    }
    
    echo "foreach loop2:\n";
    foreach($fa as $k => $v) {
        echo  "Key:\t" . $k . " = " . $v . "\n";
    }

    echo "Out of Range1:\n";
    var_dump($fa->key());
    var_dump($fa->current());
    echo "Out of Range2:\n";
    $fa->next();
    echo "Out of Range3:\n";
    $fa->next();
    var_dump($fa->key());
    var_dump($fa->current());
}

//$a = new FixedArray();
//var_dump($a);

//$id = 1;
//$db = "pa";
//$r = FixedArray::fromFile($id, $db);
//var_dump($r);

/*

class FixedArray implements Iterator, ArrayAccess, Countable {

    public static FixedArray fromArray (array $array);
    public static FixedArray fromFile (string $filepath);

    public __construct();

    public bool toFile($filepath);
    public array toArray( void )

    public int count ( void );

    public mixed current ( void )
    public string key ( void )
    public void next ( void )
    public void rewind ( void )
    public boolean valid ( void )

    public boolean offsetExists ( mixed $offset )
    public mixed offsetGet ( mixed $offset )
    public void offsetSet ( mixed $offset , mixed $value )
    public void offsetUnset ( mixed $offset )

}
*/
