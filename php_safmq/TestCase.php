<?php
include_once 'Assert.php';

class TestCase {
	public static function Test($func) {
		try {
			printf("Test: %s", $func);
			$func();
			printf(" OK!\n");		
		} catch (Assertion $ex) {
			printf(" Failed: %s\n", $ex->get_message());			
		}
	}
}
?>
