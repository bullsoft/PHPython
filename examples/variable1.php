<?php
dl('python.so');

$a = ["a" => "b", "c" => "d"];
$json = json_encode($a);

$code = <<<EOD
import json
b = json.loads('$json')
EOD;

$python = new Python();
$python->eval($code);
var_dump($python->extract("b"));