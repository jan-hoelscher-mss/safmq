<?php
class Assertion extends Exception
{
	public $message = "";
	
	public function __construct($message) {
		$this->message = $message;		
	}

	public function get_message() {
		return $this->message;
	}
}

class Assert
{
	public static function _True($message, $test) {
		if ($test !== TRUE) {
			throw new Assertion($message);
		}	
	}
}


?>
