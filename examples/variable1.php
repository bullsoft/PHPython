<?php
//dl('python.so');

$a = ["a" => "b", "c" => "d"];
$json = json_encode($a);

$code = <<<EOD
b = json.loads('$json')
EOD;

$python = new Python();
$python->eval($code);
var_dump($python->extract("b"));
$python->print("b");
var_dump($python->call('json.dumps(b)'));
var_dump($python->call(['json', 'dumps'], 'py::b'));

var_dump($python->dump());
