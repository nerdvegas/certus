#ifndef _CERTUS_RAW_CONSTRUCTOR_HPP
#define _CERTUS_RAW_CONSTRUCTOR_HPP

/*
 * From http://wiki.python.org/moin/boost.python/HowTo#A.22Raw.22_constructor
 */

#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/raw_function.hpp>


namespace boost { namespace python {

	namespace detail {

	  template <class F>
	  struct raw_constructor_dispatcher
	  {
		  raw_constructor_dispatcher(F f)
		 : f(make_constructor(f)) {}

		  PyObject* operator()(PyObject* args, PyObject* keywords)
		  {
			  borrowed_reference_t* ra = borrowed_reference(args);
			  object a(ra);
			  return incref(
				  object(
					  f(
						  object(a[0])
						, object(a.slice(1, len(a)))
						, keywords ? dict(borrowed_reference(keywords)) : dict()
					  )
				  ).ptr()
			  );
		  }

	   private:
		  object f;
	  };

	} // namespace detail

	template <class F>
	object raw_constructor(F f, std::size_t min_args = 0)
	{
		return detail::make_raw_function(
			objects::py_function(
				detail::raw_constructor_dispatcher<F>(f)
			  , mpl::vector2<void, object>()
			  , min_args+1
			  , (std::numeric_limits<unsigned>::max)()
			)
		);
	}

} } // namespace boost::python

#endif // RAW_CONSTRUCTOR_HPP
