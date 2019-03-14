<?php
$a = ["a" => "b", "c" => "d"];
$code = <<<EOD
def dofunc(arg1, arg2):
   print("Python Output")
   print(arg1)
   print(arg2)
   return {"Python": {"a": "b"}}

after = "abcd"
a = 1

EOD;

$python = new Python();
$python->eval($code);
$python->assign("tmp", $a);
var_dump($python->dofunc($a, "py::tmp"));

//var_dump($python->call("dofunc", ["py::tmp", "py::tmp"]));

//var_dump($python->call(["json", "loads"], ["[1,2,3]"]));

$ret = $python->call(["decimal", "Decimal"], [3.14159], "dec");
var_dump($ret);
var_dump($python->call(["math", "ceil"], ["py::dec"]));

$python->print("after");
$a = $python->extract('a');
var_dump($a);
