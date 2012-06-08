#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "version.h"
#include "version_range.h"
#include "multi_version_range.h"
#include "ver_token_alphanumeric.h"

using namespace certus;

typedef ver::ver_token_alphanumeric<unsigned int>	tok_type;
typedef ver::version<tok_type> 						ver_type;
typedef ver::version_range<tok_type> 				ver_range_type;
typedef ver::multi_version_range<tok_type>			multi_ver_range_type;

#define ASSERT_(X) \
	if(!(X)) { std::cerr << "assertion failed: " << #X << std::endl; assert(0); exit(1); }


template<typename T>
void test_repr(const T& t, const std::string& s, const std::string& sExpect)
{
	std::ostringstream strm;
	strm << t;
	if(strm.str() != sExpect)
	{
		std::cerr << "object created from \"" << s <<
			"\" has unexpected string representation: \'" << strm.str() << "\',"
			" expected \'" << sExpect << "\'" << std::endl;
		assert(0);
		exit(1);
	}
}

const std::string g_nomatch("x");

template<typename T>
void test_object(const std::string& objname, const std::string& s, bool success, 
	const std::string& expected_repr = g_nomatch)
{
    try
    {
        T v(s);
        if(!success)
        {
            std::cerr << "The " << objname << " string \'" << s << 
            "\' should have been invalid, but has been parsed successfully. " << 
			"Resulting " << objname << " is \'" << v << "\'" << std::endl;
            exit(1);            
        }
		
		test_repr(v, s, ((expected_repr==g_nomatch)? s : expected_repr));		
		std::cout << objname << ": \'" << v << "\' from string \'" << s << "\'" << std::endl;
    }
    catch (const ver::invalid_version_error& e)
    {
        if(success)
        {
            std::cerr << "The " << objname << " string \'" << s << 
            "\' should have been valid, but has failed parsing: " << e.what() << std::endl;
            exit(1); 
        }
		
		std::cout << objname << ": " << e.what() << std::endl;
    }
}


multi_ver_range_type g_intresult;

bool intersect(ver_range_type& v1, const ver_range_type& v2, const ver_range_type** result)
{
	*result = &v1;
	return v1.intersect_with(v2);
}

bool intersect(const multi_ver_range_type& v1, const multi_ver_range_type& v2, 
	const multi_ver_range_type** result)
{
	*result = &g_intresult;
	return v1.intersect(v2, g_intresult);
}


template<typename T>
void test_intersection_(const std::string& objname, const std::string& s1, 
	const std::string& s2, bool overlap, const std::string& result = std::string(""))
{
	T t1(s1);
	T t2(s2);

	std::string s_int;
	{
		std::ostringstream strm;
		strm << "\'" << s1 << "\' & \'" << s2 << "\'";
		s_int = strm.str();
	}
	
	const T* t3;
	if(intersect(t1, t2, &t3))
	{
		if(overlap)
		{
			test_repr(*t3, s_int, result);		
			std::cout << objname << ": " << s_int <<
				" = \'" << *t3 << "\'" << std::endl;
		}
		else
		{
			std::cerr << objname << ": " << s_int <<
				" should have been null, but is \'" << *t3 << "\'" << std::endl;
			exit(1);
		}
	}
	else
	{
		if(overlap)
		{
			std::cerr << objname << ": " << s_int <<
				" should be \'" << result << "\', but is empty" << std::endl;
			exit(1);
		}
		else
			std::cout << objname << ": " << s_int << " is empty" << std::endl;
	}
}


template<typename T>
void test_intersection(const std::string& objname, const std::string& s1, 
	const std::string& s2, bool overlap, const std::string& result = std::string(""))
{
	test_intersection_<T>(objname, s1, s2, overlap, result);
	if(s1 != s2)
		test_intersection_<T>(objname, s2, s1, overlap, result);	
}


void test_union_(const std::string& s1, const std::string& s2, const std::string& result)
{
	multi_ver_range_type v1(s1);
	multi_ver_range_type v2(s2);
	v1.union_with(v2);
	
	std::string s_union;
	{
		std::ostringstream strm;
		strm << "\'" << s1 << "\' | \'" << s2 << "\'";
		s_union = strm.str();
	}
	
	test_repr(v1, s_union, result);
	std::cout << "multi_version_range: " << s_union << " = \'" << result << "\'" << std::endl;
}


void test_union(const std::string& s1, const std::string& s2, const std::string& result)
{
	test_union_(s1, s2, result);
	if(s1 != s2)
		test_union_(s2, s1, result);
}


void test_inverse_(const std::string& s, const std::string& result)
{
	multi_ver_range_type v(s);
	multi_ver_range_type v2;
	v.inverse(v2);
	
	std::string s_inv;
	{
		std::ostringstream strm;
		strm << "inv(\'" << s << "\')";
		s_inv = strm.str();
	}
	
	test_repr(v2, s_inv, result);
	std::cout << "multi_version_range: " << s_inv << " = \'" << result << "\'" << std::endl;
}


void test_inverse(const std::string& s, const std::string& result)
{
	test_inverse_(s, result);
	test_inverse_(result, s);
}


// main
int main(int arvc, char** argv)
{
	std::vector<std::string> good_vers;
	good_vers.push_back("");
	good_vers.push_back("0");
	good_vers.push_back("1");
	good_vers.push_back("1.2");
	good_vers.push_back("5.0.1");
	good_vers.push_back("5.3.0.20.9.654.1");
	
	std::vector<std::string> bad_vers;
	bad_vers.push_back(" ");
	bad_vers.push_back("..");
	bad_vers.push_back("01");
	bad_vers.push_back(" 1.2");
	bad_vers.push_back("1.2 ");
	bad_vers.push_back(".10");
	bad_vers.push_back(".1.2");
	bad_vers.push_back("1.2..");
	bad_vers.push_back("4..5");
	bad_vers.push_back("1.-2");
	
	std::vector<std::string> good_ranges;
	good_ranges.push_back("");
	good_ranges.push_back(".");
	good_ranges.push_back("1+");
	good_ranges.push_back("1.2+");
	good_ranges.push_back("3.5.");
	good_ranges.push_back("1+<10");
	good_ranges.push_back("+<1");
	good_ranges.push_back("1.0.0+<5.6");
	good_ranges.push_back("2.3.0+<2.3.2");
	
	std::vector<std::string> bad_ranges;
	bad_ranges.push_back("+1");
	bad_ranges.push_back("<1");
	bad_ranges.push_back("5+<2");
	bad_ranges.push_back("1+<");
	bad_ranges.push_back("1.5+<");
	bad_ranges.push_back("1+<2+");
	bad_ranges.push_back("1+<2+<3");
	bad_ranges.push_back("1<");
	bad_ranges.push_back("1<5.6");
	bad_ranges.push_back("0+<0");
	bad_ranges.push_back(".+<2");
	bad_ranges.push_back("1.3+<1.3");
	bad_ranges.push_back("2.3+<1");
	bad_ranges.push_back("1.5+<2.");
	bad_ranges.push_back("2+<.");
	bad_ranges.push_back("1.+<2");
	
	std::vector<const std::string*> intersections;
	std::string int1[3] = {"", 			"", 			""};
	std::string int2[3] = {"7", 		"7", 			"7"};
	std::string int3[3] = {"", 			"1.5", 			"1.5"};
	std::string int4[3] = {"3.0+", 		"", 			"3.0+"};
	std::string int5[3] = {"1", 		"1.5", 			"1.5"};
	std::string int6[3] = {"1.4.5", 	"1.4", 			"1.4.5"};
	std::string int7[3] = {"5+", 		"1+<8", 		"5+<8"};
	std::string int8[3] = {"4.5+<5",	"4.8.1+<6.2", 	"4.8.1+<5"};
	intersections.push_back(int1);
	intersections.push_back(int2);
	intersections.push_back(int3);
	intersections.push_back(int4);
	intersections.push_back(int5);
	intersections.push_back(int6);
	intersections.push_back(int7);
	intersections.push_back(int8);
	
	std::vector<const std::string*> null_intersections;
	std::string null_int1[2] = {"1", 		"5"};
	std::string null_int2[2] = {"2", 		"3"};
	std::string null_int3[2] = {"3.0.0", 	"3.0.1"};		
	std::string null_int4[2] = {"2+<2.0", 	"2.0"};
	null_intersections.push_back(null_int1);
	null_intersections.push_back(null_int2);
	null_intersections.push_back(null_int3);
	null_intersections.push_back(null_int4);
	
	
	// testing all types with basic version string contruction
	for(std::size_t i=0; i<good_vers.size(); ++i)
	{
		test_object<ver_type>("version", good_vers[i], true);
		test_object<ver_range_type>("version_range", good_vers[i], true);
		test_object<multi_ver_range_type>("multi_version_range", good_vers[i], true);
	}

	for(std::size_t i=0; i<bad_vers.size(); ++i)
	{
		test_object<ver_type>("version", bad_vers[i], false);
		test_object<ver_range_type>("version_range", bad_vers[i], false);
		test_object<multi_ver_range_type>("multi_version_range", bad_vers[i], false);
	}

	
	// testing version
	ASSERT_(ver_type("4") < ver_type("5"));
	ASSERT_(ver_type("4") < ver_type("4.0"));
	ASSERT_(ver_type("7.9.2") < ver_type("20"));
	ASSERT_(ver_type("_") < ver_type("0"));	
	ASSERT_(ver_type("1.b") < ver_type("1.0"));	
	ASSERT_(ver_type("a1") < ver_type("1a"));
	ASSERT_(ver_type("foo22hey") < ver_type("foo24zzzzz"));	
	ASSERT_(ver_type("foo22hey") < ver_type("fool3zzzzz"));	
	ASSERT_(ver_type("5.4.hey.7a") < ver_type("5.4.ho"));


	// testing version and version_range
	test_object<ver_range_type>("version_range", "+", 		true, "");
	test_object<ver_range_type>("version_range", "+<_", 	true, ".");
	
	for(std::size_t i=0; i<good_vers.size(); ++i)
	{
		ver_type v(good_vers[i]);
		ver_type vnearest = v.get_nearest();
		ver_type vnext = v.get_next();
		ver_type vnnext = vnext.get_next();
		
		ASSERT_(v < vnearest);
		ASSERT_(v < vnext);
		ASSERT_(v < vnnext);
		ASSERT_(vnext < vnnext);
		
		{
			std::ostringstream strm;
			strm << v << "+<" << vnearest;
			test_object<ver_range_type>("version_range", strm.str(), true, good_vers[i]+".");
		}
		
		if(vnext != vnearest)
		{
			std::ostringstream strm;
			strm << v << "+<" << vnext;
			test_object<ver_range_type>("version_range", strm.str(), true, good_vers[i]);
		}
		
		{
			std::ostringstream strm;
			strm << v << "+<" << vnnext;
			test_object<ver_range_type>("version_range", strm.str(), true);
		}
	}
	
	ASSERT_(ver_range_type("4") < ver_range_type("5"));
	ASSERT_(ver_range_type("4") < ver_range_type("4.0"));
	ASSERT_(ver_range_type("7.9.2") < ver_range_type("20"));

	test_object<multi_ver_range_type>("multi_version_range", ".", true);

	// testing version_range and multi_version_range
	for(std::size_t i=0; i<good_ranges.size(); ++i)
	{
		test_object<ver_range_type>("version_range", good_ranges[i], true);
		test_object<multi_ver_range_type>("multi_version_range", good_ranges[i], true);
	}
	
	for(std::size_t i=0; i<bad_ranges.size(); ++i)
	{
		test_object<ver_range_type>("version_range", bad_ranges[i], false);
		test_object<multi_ver_range_type>("multi_version_range", bad_ranges[i], false);
	}
	
	for(std::size_t i=0; i<intersections.size(); ++i)
	{
		test_intersection<ver_range_type>("version_range", 
			intersections[i][0], intersections[i][1], true, intersections[i][2]);

		test_intersection<multi_ver_range_type>("multi_version_range", 
			intersections[i][0], intersections[i][1], true, intersections[i][2]);
	}

	for(std::size_t i=0; i<null_intersections.size(); ++i)
	{
		test_intersection<ver_range_type>("version_range", 
			null_intersections[i][0], null_intersections[i][1], false);

		test_intersection<multi_ver_range_type>("multi_version_range", 
			null_intersections[i][0], null_intersections[i][1], false);
	}
	
	
	// testing multi_version_range
	for(std::size_t i=0; i<good_ranges.size(); ++i)
	{
		test_union(good_ranges[i], good_ranges[i], good_ranges[i]);
		test_union(good_ranges[i], "", "");
		
		test_intersection<multi_ver_range_type>("multi_version_range",
			good_ranges[i], good_ranges[i], true, good_ranges[i]);
		test_intersection<multi_ver_range_type>("multi_version_range",
			good_ranges[i], "", true, good_ranges[i]);
	}
	
	test_object<multi_ver_range_type>("multi_version_range", "4|4.5", 			true, "4");
	test_object<multi_ver_range_type>("multi_version_range", "300|301", 		true, "300+<302");
	test_object<multi_ver_range_type>("multi_version_range", "8|6|4", 			true, "4|6|8");	
	test_object<multi_ver_range_type>("multi_version_range", "5.0.3|5.0.4", 	true, "5.0.3+<5.0.5");	
	test_object<multi_ver_range_type>("multi_version_range", "1|3|1.0.9", 		true, "1|3");		
	test_object<multi_ver_range_type>("multi_version_range", "1|3|1.8+<3.1", 	true, "1+<4");		
	
	test_union("4+<7.5",		"8",			"4+<7.5|8");
	test_union("4+<7.5",		"6.3.1",		"4+<7.5");
	test_union("1.2+<2",		"1.5+",			"1.2+");
	test_union("3",				"4",			"3+<5");
	test_union("6+<6.0",		"6.0",			"6+<6.1");
	test_union("7",				"2",			"2|7");
	test_union("2.5+<5.1",		"2|5",			"2+<6");
	test_union("4|7|22|10",		"5.1|13|9",		"4|5.1|7|9+<11|13|22");
	test_union("2.5|3.6+<4.1",	"2.5.3+<3.9",	"2.5+<4.1");
	
	test_intersection<multi_ver_range_type>("multi_version_range", "3.0|7|9",		"2+<3.0.5",			true, "3.0+<3.0.5");
	test_intersection<multi_ver_range_type>("multi_version_range", "4.5|5",			"4.5.2+<5.1.0",		true, "4.5.2+<4.6|5+<5.1.0");
	test_intersection<multi_ver_range_type>("multi_version_range", "1.1|3.2|7",		"7.9+",				true, "7.9+<8");
	test_intersection<multi_ver_range_type>("multi_version_range", "1|3|5|7",		"",					true, "1|3|5|7");
	
	test_inverse("", 			"(empty)");
	test_inverse(".", 			"_+");
	test_inverse("+<5.1",		"5.1+");
	test_inverse("1",			"+<1|2+");
	test_inverse("_+<10",		".|10+");
	test_inverse("1.2.3.",		"+<1.2.3|1.2.3._+");
	test_inverse("2|4|8",		"+<2|3|5+<8|9+");
	
	// done
    std::cout << "\nVersion tests passed successfully!" << std::endl;
	return 0;
}














