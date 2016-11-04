#ifndef _COM_STOCHASTIC_GLUTPP_TESTER_H_
#define _COM_STOCHASTIC_GLUTPP_TESTER_H_ 1

#include <glutpp.h>

namespace glutpp {

    class Tester {
    protected:
	static int s_argc;
	/*
	 * Some tests will want to make a copy of argv.
	 */
	static char** s_argv;
    private:
	static Tester* s_first;
	Tester* _next;
	const char* _name;
    public:
	static void runAll(int argc, char* argv[]);
	explicit Tester(const char* name);
	virtual void test() = 0;
    };

    class TestException : public Exception {
    public:
	TestException(const char* what) : Exception(what) {}
    };
};

#endif // _COM_STOCHASTIC_GLUTPP_TESTER_H_
