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
      _scope = new py::dict(py::module::import("__main__").attr("__dict__"));
      _local = new py::dict();
    }

    /**
     *  Destructor
     */
    ~Master() {
      delete _local;
      delete _scope;
      //PyEval_ReleaseLock();
    }

    /* void evalFile(Php::Parameters &params) { */
    /*   std::string filePath = params[0]; */
    /*   py::eval_file(filePath, _scope, _local); */
    /* } */

    void eval(Php::Parameters &params) {
      std::string state = params[0];
      py::eval<py::eval_statements>(state, *_scope, *_local);
    }

    Php::Value var(Php::Parameters &params) {
      if (params.size() != 1) throw Php::Exception("Invalid number of parameters supplied");
      std::string v = params[0];
      return (Php::Value) (*_local)[py::str(v)].attr("__str__")().cast<std::string>();
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

