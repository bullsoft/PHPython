/**
 *  Python.h
 *
 *  Class that is exported to PHP space
 */

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
#include <iostream>
#include <sstream>

namespace py = pybind11;

/**
 *  Class definition
 */
class Master : public Php::Base
{
private:
  py::module _main;
  py::module _json;
  py::module _sys;
  py::dict *_scope;
  py::dict *_local;
  std::map<std::string, py::module> _modules;

public:
  /**
   *  Constructor
   */
  Master()
  {
    //PyEval_AcquireLock();
    //PyThreadState_Swap(tstate);
    _main = py::module::import("__main__");
    _sys  = py::module::import("sys");
    _json = py::module::import("json");
    _scope = new py::dict(_main.attr("__dict__"));
    _local = _scope;

    _modules["sys"] = _sys;
    _modules["json"] = _json;
  }

  /**
   *  Destructor
   */
  ~Master() {
    delete _scope;
    //delete _local;
    //PyEval_ReleaseLock();
  }

  /**
   *  php "constructor"
   *  @param  params
   */
  void __construct(Php::Parameters &params)
  {
    if (!params.empty()) {
      Php::Value argv = params[0];
      _sys.attr("argv") = phpVal2PyObj(argv);
    }
  }

  void evalFile(Php::Parameters &params) {
    std::string filePath = params[0];
    try {
      py::eval_file(filePath, *_scope);
    } catch(py::error_already_set& ex) {
      throw Php::Exception(ex.what());
    }
  }

  void eval(Php::Parameters &params) {
    std::string state = params[0];
    try {
      py::eval<py::eval_statements>(state, *_scope);
    } catch (py::error_already_set& ex) {
      throw Php::Exception(ex.what());
    }
  }

  void loadModule(Php::Parameters &params) {
    const char* name = params[0];
    if(_modules.find(name) == _modules.end()) {
      _modules[name] = py::module::import(name);
    }
  }

  void assign(Php::Parameters &params) {
    std::string name = params[0];
    (*_local)[py::str(name)] = phpVal2PyObj(params[1]);
  }

  Php::Value call(Php::Parameters &params) {
    // Python函数执行结果
    py::object pyRet;
    // 第1个参数为被调用的函数名，形式为：
    // 1) "print"
    // 2) ["os", "rename"]
    if (params[0].isString())  {
      std::string callFunc = params[0];
      pyRet = py::eval(callFunc, *_scope);
      if (params.size() > 1 && params[1].isString()) {
        std::string pyVar = params[1];
        (*_local)[py::str(pyVar)] = pyRet;
        return nullptr;
      }
    } else if(params[0].isArray()){
      Php::Value array = params[0];
      auto moduleFunc = array.mapValue();
      const char* mName = moduleFunc["0"];
      const char* fName = moduleFunc["1"];
      // 模块是否已加载
      if(_modules.find(mName) == _modules.end()) {
        _modules[mName] = py::module::import(mName);
      }

      // 构建Python参数
      py::tuple pyParams;
      if (params.size() > 1 && params[1].isArray()) {
        auto pyArgs = params[1];
        pyParams = py::tuple(pyArgs.length());
        for (auto &iter : pyArgs) {
          bool isPyLocalArg = false;
          if (iter.second.isString()) {
            std::string pyLocalArgStr = iter.second;
            if(pyLocalArgStr.rfind("py::", 0) == 0) {
              std::string pyLocalArgName = pyLocalArgStr.substr(4);
              py::object pyLocalArg = (*_local)[py::str(pyLocalArgName)];
              pyParams[iter.first.numericValue()] = pyLocalArg;
              isPyLocalArg = true;
            }
          }
          // 在作用域中未找到相关变量，当作字符串处理
          if (isPyLocalArg == false) {
            pyParams[iter.first.numericValue()] = phpVal2PyObj(iter.second);
          }
        }
      } else {
        pyParams = py::tuple(0);
      }
      auto func = _modules[mName].attr(fName);
      pyRet = func(*pyParams);

      if(params.size() > 2 && params[2].isString()) {
        std::string pyVar = params[2];
        (*_local)[py::str(pyVar)] = pyRet;
        return nullptr;
      }
    }
    return pyObj2PhpVal(pyRet);
  }

  py::object phpVal2PyObj(Php::Value val) {
    auto json_decode = this->_json.attr("loads");
    std::string phpStr = Php::call("json_encode", val);
    return json_decode(py::str(phpStr));
  }

  Php::Value pyObj2PhpVal(py::object obj) {
    auto json_encode = this->_json.attr("dumps");
    std::string pyStr = json_encode(obj).cast<std::string>();
    return Php::call("json_decode", pyStr, true);
  }

  void def(Php::Parameters &params) {
    const char *name = params[0];
    Php::Value func = params[1];
    _main.def(name, [func, this](py::args args, py::kwargs kwargs) {
                      auto json_encode = this->_json.attr("dumps");
                      auto json_decode = this->_json.attr("loads");
                      std::string a = json_encode(args).cast<std::string>();
                      Php::Value  b = Php::call("json_decode", a, true);
                      std::string ret = Php::call("json_encode", func(b));
                      return json_decode(py::str(ret));
                    });
  }

  Php::Value extract(Php::Parameters &params) {
    if (params.size() != 1) throw Php::Exception("Invalid number of parameters supplied");
    std::string name = params[0];
    //auto json_encode = _json.attr("dumps");
    py::object val = (*_local)[py::str(name)];
    //std::string a = json_encode(val).cast<std::string>();
    //return Php::call("json_decode", a, true);
    return pyObj2PhpVal(val);
  }

  void print(Php::Parameters &params) {
    if (params.size() != 1) throw Php::Exception("Invalid number of parameters supplied");
    std::string v = params[0];
    py::print((*_local)[py::str(v)]);
  }

  /**
   *  Cast to a string
   *  @return const char *
   */
  const char *__toString() const
  {
    return "This is the PHPython main class";
  }
};

