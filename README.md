FCache Extensions (version 1.0.3)
=================================

A php extension which can dump memory data to a binary file and load the file into shared memory to accelerate accessing.


Table of Contents
=================

* [Install](#install)
* [BloomFilter](#BloomFilter)
* [FixedArray](#FixedArray)
* [Changelog](#Changelog)

# Install

~~~
phpize
./configure
make clean && make
make test
make install
~~~

# php.ini 配置

~~~
[fcache]
extension=fcache.so

; data_dir 是索引文件目录
fcache.data_dir="/usr/home/zhujun5/data"

; 检查周期, 每 purge_period 秒进行一次清理检查操作
fcache.purge_period=43200

; 每次清理检查时，在 unused_threshold 秒内没有被访问过的数据将被 unmap 出去
fcache.unused_threshold=86400
~~~

# API

<a name="BloomFilter"></a>

## BloomFilter

### Introduction

BLoomFilter 是一种高效的数据结构，用于判断一个元素是否在集合中，它的复杂度是$$O\left(1\right)$$。而且由于 BloomFilter 使用 Bitmap 保存数据，所以它也非常的节省内存。但是 BloomFilter 会有一定的 `false positive`，即对于部分不在集合中的元素会认为在集合中。这个错误概率和两个变量有关，一个是平均每个元素占用的 bits 数`bitsPerElement`，另一个是使用的 hash function 的个数 `hashCnt` ，在目前使用的配置中 `hashCnt = 8` 和 `bitsPerElement = 16` 时，这个错误概率大约是 `0.000574`。具体证明可参见 [Bloom Filters - the math](http://pages.cs.wisc.edu/~cao/papers/summary-cache/node8.html)

### Class

~~~
class FCacheBloomFilter {
	/* Constants */
	static const string $EXT_NAME = "bf";

	/* Methods */
	static public bool isPresent( string $setid, string $key [, string $db = NULL] );
	static public bool createIdx( string $txtFile, string $idxFile, long $N, int $dataVers [, int $hashCnt = 8, int $bitsPerElement = 16] );
	static public array stat( string $idxFile );
}
~~~

### isPresent
----------
_**Description**_: 查询`$key`是否在集合`$setid`中

#### *Parameters*

*setid*: `fcache.data_dir` 目录下的数据文件名，不包含扩展名

*key*: 待查询的键值

*db*: `fcache.data_dir` 下的子目录，optional

#### *Return value*

*BOOL*: `TRUE` 在集合中, `FALSE` 不在集合中.

#### *Example*

~~~
$ret = FCacheBloomFilter::isPresent("big", "3099999890");
~~~

### createIdx
----------
_**Description**_: 从文本文件生成 BloomFilter 数据文件

___注意___：这个方法会阻塞在文件 IO 上，所以不能在前台使用，只能在后台使用。

#### *Parameters*

*txtFile*: 输入集合文件全路径，每个key占一行

*idxFile*: 输出数据文件全路径，带扩展名（即 FCacheBloomFilter::$EXT_NAME ）

*N*: 输入文件中 key 的数量

*dataVers*: 数据版本

*hashCnt*: 生成 hash 值个数，optional

*bitsPerElement*: 每个位占用的 bits ，optional

#### *Return value*

*BOOL*: `TRUE` 成功, `FALSE` 失败.

#### *Example*

~~~
$ret = FCacheBloomFilter::createIdx($txtFile, $idxFile, 10000, 1);
~~~

### stat
----------
_**Description**_: 读取数据文件的摘要信息

___注意___：对于大文件，这个方法会阻塞在文件 IO 上，所以建议不要在前台使用。

#### *Parameters*

*idxFile*: 输出数据文件全路径，带扩展名（即 FCacheBloomFilter::$EXT_NAME ）

#### *Return value*

*array*: 

~~~
array(4) {
	'bit_size' => int(16024) //位集大小
	'byte_size' => int(2003) //占用字节数
	'version' => int(1)      //数据版本
  	'hash_cnt' => int(8)     //使用的hash function 数
}
~~~

#### *Example*

~~~
$statinfo = FCacheBloomFilter::stat($idxFile);
~~~

使用的例子可见 fcache.php 和 fcache_writer.php

### BloomFilter 性能测试

fcache 使用 `mmap` 把数据文件映射到进程的虚拟地址空间（以 `MAP_SHARED` 方式），通常 mmap 不会把整个文件拷贝到内存中，而是采用缓式加载机制，即只有用到的 __*内存页*__ 才会被加载到物理内存中。当进行 BloomFilter 查找时，如果相关的内存页没有加载到内存，就会产生一个 __*缺页中断(pagefault)*__，这时发生的 pagefault 错误称为 __*大错误(Major)*__ ：即物理内存中没有这页，需要从存储上加载。Major 的成本比较高，因为它产生磁盘IO；如果一个内存页已经加载到了内存但是在当前进程的内存管理单元中没有标记，这时发生的 pagefault 错误称为 __*小错误(Minor)*__ 这种情况一般多出现在使用共享内存的情况下，Minor 成本就较低。下面是经过测试得出的查询延迟：

* 发生1次 Major 时的查询延迟：0.00774 秒	(7 ms)
* 发生1次 Minor 时的查询延迟：0.00013112 秒 (131 us)
* 没有任何缺页错误时的查询延迟：0.00005412 秒 (54 us)

以上的时间是较粗略的统计，其中还包括了一些（少量）系统调用的消耗，但是还是能够说明总体的情况，在数据完全加载到内存后延迟是微秒级，在没有加载到内存时延迟是毫秒级别。

使用 ps 可以查看进程的 pagefault 情况，MAJFLT就是 major，MINFLT 是 minor

~~~
ps -o majflt,minflt -p <pid>

MAJFLT MINFLT
    32   1034
~~~

### BloomFilter 错误率测试

####测试方法

生成两个包含随机数的集合 A 和 B，A、B 的元素数量均为 24854815，且 A 和 B 交集为空。生成 A 和 B 对应的数据文件：A.bf 和 B.bf，交叉检查集合中每个元素是否出现在对方集合中：即在 A.bf 中查找 B 的每个元素，在 B.bf 中查找 A 的每个元素

####测试结果
当使用`hashCnt = 8` 和 `bitsPerElement = 16` 生成数据时，得到如下的结果：

* 在 A.bf 中存在的 B 中的元素数量是： 14493 个
* 在 B.bf 中存在的 A 中的元素数量是： 14491 个

两个错误率均约等于 0.000583，与上述的错误率（0.000574）基本一致。

当使用`hashCnt = 8` 和 `bitsPerElement = 32` 生成数据时，得到如下的结果：

* 在 A.bf 中存在的 B 中的元素数量是： 137 个
* 在 B.bf 中存在的 A 中的元素数量是： 159 个

两个错误率约等于 0.0000055 和 0.0000064，与标准的错误率（5.73e-06）基本一致。

上述的第二种配置误判率更低，但是生成的数据文件也更大（`95MB`），是第一个数据文件的两倍大小（`47MB`）。

<a name="FixedArray"></a>

## FixedArray

### Introduction

FixedArray 能把PHP数组保存到文件，然后加载到共享内存，提供只读的访问，加载的过程不需要反序列化，没有数据拷贝。目前不支持嵌套的数组，只能使用string/long/double这种简单类型作为值，可用作配置文件的存储

### Class

~~~
class FixedArray implements Iterator, ArrayAccess, Countable {

    public static FixedArray fromArray (array $array);
    public static FixedArray fromFile (string $filepath);

    private __construct();

    public bool toFile($filepath);
    public array toArray( void );

    public int count ( void );

    public mixed current ( void );
    public string key ( void );
    public void next ( void );
    public void rewind ( void );
    public boolean valid ( void );

    public boolean offsetExists ( mixed $offset );
    public mixed offsetGet ( mixed $offset );
    public void offsetSet ( mixed $offset , mixed $value ); //not implement
    public void offsetUnset ( mixed $offset );//not implement

}

~~~

### 用法

#### 把数组保存到文件
~~~
//目前数组不支持嵌套，只能是 long/double/string 类型
$data = array (
    "abcdefghi",
    "1",
    78,
    "abc" => "cba",
    1,
    4,
    5,
    NULL,
    "中文",
);

$fa = FixedArray::fromArray($data);
$ret = $fa->toFile($filepath);

~~~

#### 加载文件到共享内存

~~~

//FixedArray 文件以 pa 为后缀，加载的路径和 BloomFilter 一样，需要在 ini 中配置目录
$fa = FixedArray::fromFile("filename", "setting");

echo $fa->offsetGet("abc"); //输出: cba
echo $fa[1]; //输出:  1

echo "for loop:\n";
for(; $fa->valid(); $fa->next()) {
	echo  "Key:\t" . $fa->key() . " = " . $fa->current() . "\n";
}

echo "foreach loop2:\n";
foreach($fa as $k => $v) {
	echo  "Key:\t" . $k . " = " . $v . "\n";
}

//不支持写操作
$fa[5] = 1; //将报错

~~~

# 数据更新

* 当数据内容被更新，扩展会把数据重新映射到内存中。
* 当数据文件被删除，isPresent 返回 false，同时引发一个 php warning，提示获取文件信息失败。
* 当数据长度被清空，isPresent 返回 false，同时引发一个 php warning，提示文件长度为 0。

# 执行 googletest

~~~
cd fcache/gtest
make clean
make
~~~

<a name="Changelog"></a>

# Changelog

## v1.0.3
* 修改代码组织结构，便于增加数据结构
* 增加 php module 宏定义，使调用更加清晰
* php测试脚本移入 tools 目录
* 增加 FixedArray

## v1.0.2
* 修复bug

## v1.0.1
* 修复 bug
* 使用 googletest 做为单元测试框架
* 增加测试用例
* 修改前台接口 isPresent 错误级别为 WARNING
* 优化性能，不必每次检查文件路径有效性

## v1.0.0

* 支持数据结构 bloomfilter，并支持导出到文件系统
* 支持前端对 bloomfilter 的只读访问，快速确定元素是否在大集合中
* 支持把一个文件映射到共享内存中，并支持当文件更新时，重新加载数据
* 支持定期清理数据，把长时间未访问的数据清理出内存
