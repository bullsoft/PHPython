<?php
dl('python.so');

$code = <<<EOD
tmp = {'a': 'b', 'c': 'd'}

EOD;

$python = new Python();
$python->eval($code);
var_dump($python->extract("tmp"));