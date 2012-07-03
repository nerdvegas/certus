#ifndef _CERTUS_EXCEPTIONS_BINDINGS__H_
#define _CERTUS_EXCEPTIONS_BINDINGS__H_ 

/*
#include "util.h"
#include "sys.h"
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/exception_translator.hpp>


namespace certus {

	PyObject* createExceptionClass(const char* name, PyObject* baseTypeObj);

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
*/

#endif
