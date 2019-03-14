#include <phpcpp.h>
#include "master.h"

PyThreadState *mainThreadState;
PyThreadState *tstate;

/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C" {

  /**
   *  Function that is called by PHP right after the PHP process
   *  has started, and that returns an address of an internal PHP
   *  strucure with all the details and features of your extension
   *
   *  @return void*   a pointer to an address that is understood by PHP
   */
  PHPCPP_EXPORT void *get_module()
  {
    // static(!) Php::Extension object that should stay in memory
    // for the entire duration of the process (that's why it's static)
    static Php::Extension extension("php-python++", "1.0");

    // we have to class - master
    Php::Class<Master> master("python");

    master.method<&Master::__construct>
      ("__construct",
       {
        Php::ByRef("argv", Php::Type::Array, false)
       });

    master.method<&Master::print>
      ("print",
       {
        Php::ByVal("name", Php::Type::String, true)
       });

    master.method<&Master::dump>
      ("dump", {});

    master.method<&Master::eval>
      ("eval",
       {
        Php::ByVal("statements", Php::Type::String, true)
       });

    master.method<&Master::evalFile>
      ("evalFile",
       {
        Php::ByVal("filePath", Php::Type::String, true)
       });

    master.method<&Master::extract>
      ("extract",
       {
        Php::ByVal("name", Php::Type::String, true)
       });

    master.method<&Master::assign>
      ("assign",
       {
        Php::ByVal("name", Php::Type::String, true),
        Php::ByVal("value", Php::Type::Null, true),
       });

    master.method<&Master::def>
      ("def",
       {
        Php::ByVal("name", Php::Type::String, true),
        Php::ByVal("func", Php::Type::Callable, true),
       });

    master.method<&Master::call>
      ("call",
       {
        Php::ByVal("func", Php::Type::Null, true),
        Php::ByVal("params", Php::Type::Null, false),
        Php::ByVal("result", Php::Type::String, false)
       });

    // add all classes to the extension
    extension.add(master);

    extension.onStartup([]() {
                          Py_Initialize();
                          PyEval_InitThreads(); // Will Lock Interpreter
                          mainThreadState = PyThreadState_Get();
                          PyEval_ReleaseLock(); // Will Release Lock
                        });

    extension.onShutdown([]() {
                           PyEval_AcquireLock();
                           Py_Finalize() ;
                         });

    extension.onRequest([]() {
                          // Creating a New Thread of Execution
                          PyEval_AcquireLock();
                          PyInterpreterState * mainInterpreterState = mainThreadState->interp;
                          tstate = PyThreadState_New(mainInterpreterState);
                          PyEval_ReleaseLock();

                          if (!tstate) {
                            throw Php::Exception("Python: Failed to create new interpreter");
                          }

                          // Prepare to execute PythonCode
                          PyEval_AcquireLock();
                          PyThreadState_Swap(tstate);
                          // Thread Ready!!!
                        });

    extension.onIdle([]() {
                       //PyEval_ReleaseLock();
                       //PyEval_AcquireLock();
                       // Thread Completed!!!

                       // Swith to main thread
                       PyThreadState_Swap(mainThreadState);
                       // Clear thread
                       PyThreadState_Clear(tstate);
                       PyEval_ReleaseLock();
                       // Delete ...
                       PyThreadState_Delete(tstate);
                     });

    // return the extension
    return extension;
  }
}
