--TEST--
Check for fcache presence
--SKIPIF--
<?php if (!extension_loaded("fcache")) print "skip"; ?>
--FILE--
<?php
ini_set('error_reporting', E_ALL & ~E_NOTICE);
echo "fcache extension is available";
/*
	you can add regression tests for your extension here

  the output of your test code has to be equal to the
  text in the --EXPECT-- section below for the tests
  to pass, differences between the output and the
  expected text are interpreted as failure

	see php5/README.TESTING for further information on
  writing regression tests
*/
?>
--EXPECT--
fcache extension is available
