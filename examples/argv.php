<?php
//dl("python.so");

$code = <<<EOD
import sys
print(sys.argv)

EOD;

var_dump($argv);

$python = new Python($argv);
$python->eval($code);
