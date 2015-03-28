<?php
dl("fcache.so");
ini_set('fcache.data_dir', '/Users/zhujun/source/sina/BloomFilter/data');

ini_set('error_reporting', E_ALL & ~E_NOTICE);
ini_set('display_errors', 1);
ini_set('log_errors', 0);

$data = array (
    "abcdefghi",
    "2",
    78,
    array (
        "a" => "x",
        "b" => "y",
        "c" => "z",
    ),
    "abc" => "cba",
    1,
    "lala" => array(1, 2, 3),
    "abce" => array ("bcd" => 1, "efg" => 0 ),
    4,5,
    NULL,
    "浅粉红",
);

$fa1 = FixedArray::fromArray($data);
$ret = $fa1->toFile('/Users/zhujun/source/sina/BloomFilter/data/pa/data.pa');

$id = 'data';
$db = "pa";
$fa = FixedArray::fromFile($id, $db);

echo $fa[1] . "\n";

$f1 = $fa["lala"];
echo $f1[0];

print_r($fa); //This is not work, because it is a object.

foreach($fa as $k =>$v) {
    echo "$k = $v\n";
}

$fa[1]  = 'abc'; //error, readonly

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
