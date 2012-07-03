#ifndef _CERTUS_PY_UTIL__H_
#define _CERTUS_PY_UTIL__H_

#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/exception_translator.hpp>
#include "sys.h"


namespace bp = boost::python;

#define CERTUS_THROW(ExcType, Msg) { \
        std::stringstream strm;                        	\
        strm << Msg;                               		\
        PyErr_SetString(ExcType, strm.str().c_str());   \
        boost::python::throw_error_already_set();      	\
}


namespace certus {

	typedef bp::return_value_policy<bp::return_by_value> bp_ret_val;


	// helper for pickling of string-constructed objects
	template<typename T>
    struct _str_pickle : boost::python::pickle_suite
    {
		static bp::tuple getinitargs(const T& t) {
			return bp::make_tuple(boost::lexical_cast<std::string>(t));
		}
    };


    // http://stackoverflow.com/questions/9620268/boost-python-custom-exception-class
    PyObject* createExceptionClass(const char* name, PyObject* baseTypeObj);


    // exception binding
	template<typename T>
	struct exception_bind
	{
		exception_bind(const char* name)
		{
			m_excType = createExceptionClass(name, exception_bind<certus_error>::m_excType);
			bp::register_exception_translator<T>(&translate);
		}

		static void translate(const T& e)
		{
			PyErr_SetString(m_excType, e.what());
		}

		static PyObject* m_excType;
	};

	template<>
	exception_bind<certus_error>::exception_bind(const char* name);

	template<typename T>
	PyObject* exception_bind<T>::m_excType(NULL);

}

#endif



