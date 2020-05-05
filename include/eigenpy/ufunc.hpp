//
// Copyright (c) 2020 INRIA
//

#ifndef __eigenpy_ufunc_hpp__
#define __eigenpy_ufunc_hpp__

#include "eigenpy/fwd.hpp"
#include "eigenpy/numpy-type.hpp"
#include "eigenpy/exception.hpp"

#include <algorithm>
#include <map>
#include <typeinfo>
#include <string>

namespace eigenpy
{
  namespace internal
  {
  
#define EIGENPY_REGISTER_BINARY_OPERATOR(name,op) \
    template<typename T1, typename T2, typename R> \
    void binary_op_##name(char** args, npy_intp * dimensions, npy_intp * steps, void * /*data*/) \
    { \
      npy_intp is0 = steps[0], is1 = steps[1], \
      os = steps[2], n = *dimensions; \
      char * i0 = args[0], *i1 = args[1], *o = args[2]; \
      int k; \
      for (k = 0; k < n; k++) \
      { \
        T1 & x = *static_cast<T1*>(static_cast<void*>(i0)); \
        T2 & y = *static_cast<T2*>(static_cast<void*>(i1)); \
        R & res = *static_cast<R*>(static_cast<void*>(o)); \
        res = x op y; \
        i0 += is0; i1 += is1; o += os; \
      } \
    } \
    \
    template<typename T> \
    void binary_op_##name(char** args, npy_intp * dimensions, npy_intp * steps, void * data) \
    { \
      binary_op_##name<T,T,T>(args,dimensions,steps,data); \
    }
  
    EIGENPY_REGISTER_BINARY_OPERATOR(add,+)
    EIGENPY_REGISTER_BINARY_OPERATOR(subtract,-)
    EIGENPY_REGISTER_BINARY_OPERATOR(multiply,*)
    EIGENPY_REGISTER_BINARY_OPERATOR(divide,/)
    EIGENPY_REGISTER_BINARY_OPERATOR(equal,==)
    EIGENPY_REGISTER_BINARY_OPERATOR(not_equal,!=)
    EIGENPY_REGISTER_BINARY_OPERATOR(less,<)
    EIGENPY_REGISTER_BINARY_OPERATOR(greater,>)
    EIGENPY_REGISTER_BINARY_OPERATOR(less_equal,<=)
    EIGENPY_REGISTER_BINARY_OPERATOR(greater_equal,>=)
  
  } // namespace internal
  
#define REGISTER_BINARY_UFUNC(name,T1,T2,R) { \
   PyUFuncObject* ufunc = \
       (PyUFuncObject*)PyObject_GetAttrString(numpy, #name); \
   int _types[3] = { Register::getTypeCode<T1>(), Register::getTypeCode<T2>(), Register::getTypeCode<R>()}; \
   if (!ufunc) { \
       /*goto fail; \*/ \
   } \
   if (sizeof(_types)/sizeof(int)!=ufunc->nargs) { \
       PyErr_Format(PyExc_AssertionError, \
                    "ufunc %s takes %d arguments, our loop takes %lu", \
                    #name, ufunc->nargs, (unsigned long) \
                    (sizeof(_types)/sizeof(int))); \
       Py_DECREF(ufunc); \
   } \
   if (PyUFunc_RegisterLoopForType((PyUFuncObject*)ufunc, code, \
        internal::binary_op_##name<T1,T2,R>, _types, 0) < 0) { \
       /*Py_DECREF(ufunc);*/ \
       /*goto fail; \*/ \
   } \
   Py_DECREF(ufunc); \
}

  template<typename Scalar>
  void registerCommonUfunc()
  {
    PyObject* numpy_str;
#if PY_MAJOR_VERSION >= 3
    numpy_str = PyUnicode_FromString("numpy");
#else
    numpy_str = PyString_FromString("numpy");
#endif
    PyObject* numpy;
    numpy = PyImport_Import(numpy_str);
    Py_DECREF(numpy_str);
    
    // load numpy
    import_ufunc();

    REGISTER_BINARY_UFUNC(add,Scalar,Scalar,Scalar);
    REGISTER_BINARY_UFUNC(subtract,Scalar,Scalar,Scalar);
    REGISTER_BINARY_UFUNC(multiply,Scalar,Scalar,Scalar);
    REGISTER_BINARY_UFUNC(divide,Scalar,Scalar,Scalar);
  
    // Comparison operators
    REGISTER_BINARY_UFUNC(equal,Scalar,Scalar,bool);
    REGISTER_BINARY_UFUNC(not_equal,Scalar,Scalar,bool);
    REGISTER_BINARY_UFUNC(greater,Scalar,Scalar,bool);
    REGISTER_BINARY_UFUNC(less,Scalar,Scalar,bool);
    REGISTER_BINARY_UFUNC(greater_equal,Scalar,Scalar,bool);
    REGISTER_BINARY_UFUNC(less_equal,Scalar,Scalar,bool);

    Py_DECREF(numpy);
  }
  
} // namespace eigenpy

#endif // __eigenpy_ufunc_hpp__
