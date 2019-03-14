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

    _main.attr("json") = _json;
    _main.attr("sys") = _sys;
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

  void assign(Php::Parameters &params) {
    std::string name = params[0];
    (*_local)[py::str(name)] = phpVal2PyObj(params[1]);
  }

  Php::Value call(Php::Parameters &params) {
    // Python函数执行结果
    py::object pyRet;
    py::object func;
    // 第1个参数为被调用的函数名，形式为：
    // 1) "print"
    // 2) ["os", "rename"]
    if (params[0].isString())  {
      const char* funcStr = params[0];
      if ((*_scope).contains(funcStr) && !(*_scope)[funcStr].is_none()) {
        func = _main.attr(funcStr);
      } else {
        pyRet = py::eval(funcStr, *_scope);
        return pyObj2PhpVal(pyRet);
      }
    } else if(params[0].isArray()){
      Php::Value array = params[0];
      auto moduleFunc = array.mapValue();
      const char* mName = moduleFunc["0"];
      const char* fName = moduleFunc["1"];
      // 模块是否已加载
      if ((*_scope).contains(mName) && !(*_scope)[mName].is_none()) {
        // nothing
        // std::cout << mName << " alread loaded in _main" << std::endl;
      } else {
        (*_scope)[mName] = py::module::import(mName);
      }
      func = (*_local)[mName].attr(fName);
    }
    // 构建Python参数
    py::tuple pyParams;
    if (params.size() > 1) {
      pyParams = makeArgs(params[1]);
    } else {
      pyParams = py::tuple(0);
    }
    pyRet = func(*pyParams);

    if(params.size() > 2 && params[2].isString()) {
      std::string pyVar = params[2];
      (*_local)[py::str(pyVar)] = pyRet;
      return nullptr;
    }

    return pyObj2PhpVal(pyRet);
  }

  Php::Value __call(const char *name, Php::Parameters &params)
  {
    std::string funcName = std::string(name);
    size_t index = 0;
    py::tuple pyArgs = py::tuple(params.size());

    for (auto iter : params) {
      if (iter.isString()) {
        std::string pyLocalArgStr = iter;
        if(pyLocalArgStr.rfind("py::", 0) == 0) {
          std::string pyLocalArgName = pyLocalArgStr.substr(4);
          py::object pyLocalArg = (*_local)[py::str(pyLocalArgName)];
          pyArgs[index++] = pyLocalArg;
        } else {
          pyArgs[index++] = py::str(pyLocalArgStr);
        }
      } else {
        pyArgs[index++] = phpVal2PyObj(iter);
      }
    }
    auto func = (*_local)[py::str(funcName)];
    return pyObj2PhpVal(func(*pyArgs));
  }

  py::tuple makeArgs(Php::Value phpArgs) {
    size_t index = 0;
    if(phpArgs.isArray()) {
      auto pyParams = py::tuple(phpArgs.size());
      for (auto &iter : phpArgs) {
        if (iter.second.isString()) {
          std::string pyLocalArgStr = iter.second;
          if(pyLocalArgStr.rfind("py::", 0) == 0) {
            std::string pyLocalArgName = pyLocalArgStr.substr(4);
            py::object pyLocalArg = (*_local)[py::str(pyLocalArgName)];
            pyParams[index++] = pyLocalArg;
          } else {
            pyParams[index++] = py::str(pyLocalArgStr);
          }
        } else {
          pyParams[index++] = phpVal2PyObj(iter.second);
        }
      }
      return pyParams;
    } else if(phpArgs.isString()) {
      auto pyParams = py::tuple(1);
      std::string pyLocalArgStr = phpArgs;
      if(pyLocalArgStr.rfind("py::", 0) == 0) {
        std::string pyLocalArgName = pyLocalArgStr.substr(4);
        py::object pyLocalArg = (*_local)[py::str(pyLocalArgName)];
        pyParams[index++] = pyLocalArg;
      }  else {
        pyParams[index++] = py::str(pyLocalArgStr);
      }
      return pyParams;
    }
    return py::tuple(0);
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
                      return phpVal2PyObj(func(pyObj2PhpVal(args)));
                    });
  }

  Php::Value extract(Php::Parameters &params) {
    if (params.size() != 1) throw Php::Exception("Invalid number of parameters supplied");
    std::string name = params[0];
    py::object val = (*_local)[py::str(name)];
    return pyObj2PhpVal(val);
  }

  void dump(Php::Parameters &params) {
    py::print(*_local);
  }

  void print(Php::Parameters &params) {
    if(params.size() == 1) {
      std::string v = params[0];
      py::print((*_local)[py::str(v)]);
    } else {
      auto pyParams = py::tuple(params.size());
      size_t index = 0;
      for (auto iter : params) {
        std::string v = iter;
        pyParams[index++] = (*_local)[py::str(v)];
      }
      py::print(pyParams);
    }
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

