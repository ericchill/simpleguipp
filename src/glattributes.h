#ifndef _COM_STOCHASTIC_GLUTPP_GLATTRIBUTES_H_
#define _COM_STOCHASTIC_GLUTPP_GLATTRIBUTES_H_

namespace glutpp {

    class GLAttributeList {
    private:
	int _arraySize;
	int _numItems;
	int *_array;
    public:
	GLAttributeList() {
	    _arraySize = 20;
	    _numItems = 0;
	    _array = new int[_arraySize];
	}
	~GLAttributeList() {
	    delete _array;
	}
	void add(int attr) {
	    checkArraySize(1);
	    _array[_numItems++] = attr;
	    _array[_numItems] = None;
	}
	void add(int attr, int value) {
	    checkArraySize(2);
	    _array[_numItems++] = attr;
	    _array[_numItems++] = value;
	    _array[_numItems] = None;
	}
	const int* attributes() const {
	    return _array;
	}
	const int* attribute(int i) const {
	    return &_array[i*2];
	}
	int numAttributes() const {
	    return _numItems / 2;
	}
	void printOn(FILE* out, const char* msg) {
	    fprintf(stderr, "%s: ", msg);
	    for (int i = 0; i < _numItems; i++) {
		fprintf(stderr, "%d ", _array[i]);
	    }
	    fprintf(stderr, "\n");
	}
    private:
	void checkArraySize(int n) {
	    if (_numItems + n + 1 > _arraySize) {
		int newSize = _arraySize + 20;
		int* newArray = new int[newSize];
		for (int i = 0; i < _numItems; i++) {
		    newArray[i] = _array[i];
		}
		delete _array;
		_array = newArray;
		_arraySize = newSize;
	    }
	}
    };

};


#endif // _COM_STOCHASTIC_GLUTPP_GLATTRIBUTES_H_
