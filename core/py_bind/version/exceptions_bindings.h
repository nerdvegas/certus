#ifndef _CERTUS_EXCEPTIONS_BINDINGS__H_
#define _CERTUS_EXCEPTIONS_BINDINGS__H_ 

#include <boost/python.hpp>
#include <boost/python/exception_translator.hpp>
#include <boost/format.hpp>

#include <version.h>
#include <exceptions.h>

namespace certus { namespace ver { 

    struct exception_bind
    {
        exception_bind(const char *name)
        {
            boost::python::class_<invalid_version_error> cl(name, boost::python::init<const std::string &>());
            cl.def("__str__",&invalid_version_error::what);

            boost::python::register_exception_translator<invalid_version_error>(&translate);
            gExeType = cl.ptr();
        }

        static void translate(const invalid_version_error &e)
        {
            boost::python::object pythonExceptionInstance(e);
            PyErr_SetObject(gExeType, pythonExceptionInstance.ptr());
        }

        static PyObject *gExeType;
    };
}}
#endif
