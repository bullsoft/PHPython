# PHPPython
An extension to eval python codes in PHP

## Requirements
   - pybind11 V2.1.1
   - PHP-CPP-LEGACY V1.5.7 / PHP-CPP V2.x
   - PHP 5 / PHP 7
   - Python 3
   - C++ 11


## Example

### Variables defined in Python
```php
$code = <<<EOD
a = [1, 2, 3]
EOD;
$python = new Python();
$python->eval($code);
var_export($python->extract("a"));
```
... you can use `extract` method in php to get that python variable, codes above output:
```
array (
  0 => 1,
  1 => 2,
  2 => 3,
)
```

### Variables defined in PHP
```php
$a = ["a" => "b", "c" => "d"];
$code = <<<EOD
print(a)
EOD;

$python = new Python();
$python->assign("a", $a);
$python->eval($code);
```
... you can use `assign` method in php to let python know that php variable, codes above output:
```
{'a': 'b', 'c': 'd'}
```

### Functions defined in Python
```php
$a = ["a" => "b", "c" => "d"];
$code = <<<EOD
def dofunc(arg1, arg2):
   print("Python Output")
   print(arg1)
   print(arg2)
   return {"Python": {"a": arg1, "b": arg2}}

after = "abcd"

EOD;

$python = new Python();
$python->eval($code);
$python->assign("tmp", $a);
var_dump("PHP Here...", $python->dofunc($a, "py::tmp"));
```
... you can call functions defined in Python as-is-a `$python->method()`, codes above output:

Python Output
```
{'a': 'b', 'c': 'd'}
{'a': 'b', 'c': 'd'}
string(11) "PHP Here..."
array(1) {
  'Python' =>
  array(2) {
    'a' =>
    array(2) {
      'a' =>
      string(1) "b"
      'c' =>
      string(1) "d"
    }
    'b' =>
    array(2) {
      'a' =>
      string(1) "b"
      'c' =>
      string(1) "d"
    }
  }
}
```
Here, We can also use these ways to call functions defined in Python:
```php
var_dump("PHP Here...", $python->call("dofunc", [$a, "py::tmp"]));
var_dump("PHP Here...", $python->call("dofunc(after, tmp)"));
```
this will output:
```
Python Output
{'a': 'b', 'c': 'd'} // $a in php
{'a': 'b', 'c': 'd'} // tmp in python which assigned by php
string(11) "PHP Here..."
array(1) {
  'Python' =>
  array(2) {
    'a' =>
    array(2) {
      'a' =>
      string(1) "b"
      'c' =>
      string(1) "d"
    }
    'b' =>
    array(2) {
      'a' =>
      string(1) "b"
      'c' =>
      string(1) "d"
    }
  }
}
Python Output
abcd // after in python
{'a': 'b', 'c': 'd'} // tmp in python which assigned by php
string(11) "PHP Here..."
array(1) {
  'Python' =>
  array(2) {
    'a' =>
    string(4) "abcd"
    'b' =>
    array(2) {
      'a' =>
      string(1) "b"
      'c' =>
      string(1) "d"
    }
  }
}
```

### Functions defined in PHP
```php
$code = <<<EOD
tmp = {'a': 'b', 'c': 'd'}
print("Python begin")
print(dofunc(tmp))
print("Python end")
EOD;

$python = new Python();
$python->def("dofunc", function($param) {
    echo __function__ . " in PHP: Get params from Python :" . PHP_EOL;
    echo var_export($param, true) . PHP_EOL;
    return [
        "php" => $param
    ];
});
$python->eval($code);
```
... you can call php funciton in python as-is-a real python function, codes above output:
```
Python begin
{closure} in PHP: Get params from Python :
array (
  0 =>
  array (
    'a' => 'b',
    'c' => 'd',
  ),
)
{'php': [{'a': 'b', 'c': 'd'}]}
Python end
```
if you like, you can also call that function using the php way, like this:
```php
var_dump($python->dofunc("py::tmp", "phpString"));
```
this will output:
```
{closure} in PHP: Get params from Python :
array (
  0 =>
  array (
    'a' => 'b',
    'c' => 'd',
  ),
  1 => 'phpString',
)
array(1) {
  'php' =>
  array(2) {
    [0] =>
    array(2) {
      'a' =>
      string(1) "b"
      'c' =>
      string(1) "d"
    }
    [1] =>
    string(9) "phpString"
  }
}
```

### PHP Call Python Function

![PHP Call Python Function](https://raw.githubusercontent.com/bullsoft/PHPPython/master/examples/php_call_python_func.png)

### Python Call PHP Function

![Python Call PHP Function](https://raw.githubusercontent.com/bullsoft/PHPPython/master/examples/python_call_php_func.png)

