#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "editor/editor.hpp"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace Editor;

class Test: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Test);
	CPPUNIT_TEST( test_simple    );
  CPPUNIT_TEST( test_icase     );
	CPPUNIT_TEST( test_unknown   );
	CPPUNIT_TEST( test_double    );
  CPPUNIT_TEST( test_empty     );
  CPPUNIT_TEST( test_nosuffix  );
  CPPUNIT_TEST_SUITE_END();

public:

  void test_simple(){
	  std::string suffix;
	  TYPE type = classify_name("path:filename.txt", suffix);
    CPPUNIT_ASSERT_EQUAL(TYPE_PATH, type);
    CPPUNIT_ASSERT_EQUAL(std::string("filename.txt"), suffix);
  }

	void test_icase(){
	  std::string suffix;
	  TYPE type = classify_name("pATh:fiLEname.txt", suffix);
    CPPUNIT_ASSERT_EQUAL(TYPE_PATH, type);
    CPPUNIT_ASSERT_EQUAL(std::string("fiLEname.txt"), suffix);
	}

	void test_unknown(){
		std::string input = "_non_existing_:filename.txt";
	  std::string suffix;
	  TYPE type = classify_name(input, suffix);
    CPPUNIT_ASSERT_EQUAL(TYPE_UNKNOWN, type);
    CPPUNIT_ASSERT_EQUAL(input, suffix);
	}

	void test_double(){
		std::string suffix;
		TYPE type = classify_name("path:a:b", suffix);
		CPPUNIT_ASSERT_EQUAL(TYPE_PATH, type);
		CPPUNIT_ASSERT_EQUAL(std::string("a:b"), suffix);
	}

	void test_empty(){
	  std::string suffix;
	  TYPE type = classify_name("", suffix);
    CPPUNIT_ASSERT_EQUAL(TYPE_UNKNOWN, type);
    CPPUNIT_ASSERT_EQUAL(std::string(""), suffix);
	}

	void test_nosuffix(){
	  std::string suffix;
	  TYPE type = classify_name("float", suffix);
    CPPUNIT_ASSERT_EQUAL(TYPE_FLOAT, type);
    CPPUNIT_ASSERT_EQUAL(std::string(""), suffix);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

int main(int argc, const char* argv[]){
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;

  runner.addTest( suite );
  runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr ));

  return runner.run() ? 0 : 1;
}
