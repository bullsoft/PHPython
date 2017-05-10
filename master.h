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
      _json = py::module::import("json");
      _scope = new py::dict(_main.attr("__dict__"));
      _local = _scope;//new py::dict();
    }

    /**
     *  Destructor
     */
    ~Master() {
      //delete _local;
      delete _scope;
      //PyEval_ReleaseLock();
    }

    /* void evalFile(Php::Parameters &params) { */
    /*   std::string filePath = params[0]; */
    /*   py::eval_file(filePath, _scope, _local); */
    /* } */

    void eval(Php::Parameters &params) {
      std::string state = params[0];
      py::eval<py::eval_statements>(state, *_scope);
    }

    void assign(Php::Parameters &params) {
      std::string name = params[0];
      std::string value = Php::call("json_encode", params[1]);
      auto loads = _json.attr("loads");
      (*_local)[py::str(name)] = loads(py::str(value));
    }

    Php::Value call(Php::Parameters &params) {
      std::string callFunc = params[0];
      py::object ret = py::eval(callFunc, *_scope);
      auto dumps = _json.attr("dumps");
      std::string a = dumps(ret).cast<std::string>();
      return Php::call("json_decode", a, true);
    }

    void def(Php::Parameters &params) {
      const char *name = params[0];
      Php::Value func = params[1];
      _main.def(name, [func, this](py::args args, py::kwargs kwargs) {
          auto dumps = this->_json.attr("dumps");
          auto loads = this->_json.attr("loads");
          std::string a = dumps(args).cast<std::string>();
          Php::Value  b = Php::call("json_decode", a, true);
          std::string ret = Php::call("json_encode", func(b));
          return loads(py::str(ret));
      });
    }

    Php::Value extract(Php::Parameters &params) {
      if (params.size() != 1) throw Php::Exception("Invalid number of parameters supplied");
      std::string name = params[0];
      auto dumps = _json.attr("dumps");
      py::object val = (*_local)[py::str(name)];
      std::string a = dumps(val).cast<std::string>();
      return Php::call("json_decode", a, true);
      //return (Php::Value) (*_local)[py::str(name)].attr("__str__")().cast<std::string>();
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
        return "this is the master";
    }
};

