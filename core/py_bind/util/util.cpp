#include "util.h"


namespace certus {

PyObject* createExceptionClass(const char* name, PyObject* baseTypeObj)
{
    std::string scopeName = bp::extract<std::string>(bp::scope().attr("__name__"));
    std::string qualifiedName0 = scopeName + "." + name;
    char* qualifiedName1 = const_cast<char*>(qualifiedName0.c_str());

    PyObject* typeObj = PyErr_NewException(qualifiedName1, baseTypeObj, 0);
    if(!typeObj) bp::throw_error_already_set();
    bp::scope().attr(name) = bp::handle<>(bp::borrowed(typeObj));
    return typeObj;
}


template<>
exception_bind<certus_error>::exception_bind(const char* name)
{
	m_excType = createExceptionClass(name, PyExc_Exception);
	bp::register_exception_translator<certus_error>(&translate);
}

}
