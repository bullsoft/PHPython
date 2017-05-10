<?php
dl('python.so');

$code = <<<EOD
tmp = {'a': 'b', 'c': 'd'}
print(dofunc(tmp))
EOD;

$python = new Python();
$python->def("dofunc", function($param) {
    echo "Get params from Python :" . PHP_EOL;
    echo var_export($param, true) . PHP_EOL;
    echo "Waiting for Python call" . PHP_EOL;
    return [
        "php" => $param
    ];
});
$python->eval($code);
