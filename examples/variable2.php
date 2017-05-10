<?php
dl('python.so');

$a = ["a" => "b", "c" => "d"];

$code = <<<EOD

print(b)

EOD;

$python = new Python();
$python->assign("b", $a);
$python->eval($code);
