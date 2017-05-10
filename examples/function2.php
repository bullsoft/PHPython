<?php
dl('python.so');
$a = ["a" => "b", "c" => "d"];

$code = <<<EOD

def dofunc(arg1):
   print("Python Output")
   print(arg1)
   return {"Python": {"a": "b"}}

after = "abcd"
EOD;

$python = new Python();
$python->eval($code);
$python->assign("tmp", $a);

var_dump($python->call('dofunc(tmp)'));

$python->print("after");