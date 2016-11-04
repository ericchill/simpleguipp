#include "tester.h"

//#include <pthread.h>
#include <stdio.h>

//static pthread_key_t key;

using namespace glutpp;

int Tester::s_argc;
char** Tester::s_argv;

Tester* Tester::s_first = nullptr;

void Tester::runAll(int argc, char* argv[])
{
    s_argc = argc;
    s_argv = argv;
    for (Tester* t = s_first; t; t = t->_next) {
	try {
	    fprintf(stdout, "Testing %s\n", t->_name);
	    t->test();
	} catch (TestException e) {
	    fprintf(stdout, "Test \"%s\" failed: %s\n", t->_name, e.what());
	}
    }
}

Tester::Tester(const char* name)
    : _name(name)
{
    _next = s_first;
    s_first = this;
}

int main(int argc, char *argv[])
{
    //pthread_key_create(&key, nullptr);
    fprintf(stderr, "Tester\n");
    Tester::runAll(argc, argv);
}
