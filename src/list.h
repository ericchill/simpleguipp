#ifndef _COM_STOCHASTIC_GLUTPP_LIST_H_
#define _COM_STOCHASTIC_GLUTPP_LIST_H_ 1

namespace glutpp {

    template<typename T> class List;

    template<typename T>
    class ListNode {
	ListNode<T>* _next;
	ListNode<T>* _prev;
	T _val;
	friend class List<T>;
    public:
	ListNode(T val) : _next(nullptr), _prev(nullptr),  _val(val) {}
	ListNode* next() { return _next; }
	ListNode* prev() { return _prev; }
	T value() { return _val; }
    };

    /* A list structure */
    template<typename T>
    class List {
	ListNode<T>* _first;
	ListNode<T>* _last;
    public:
    List() : _first(nullptr), _last(nullptr) {}
	ListNode<T>* first() { return _first; }
	ListNode<T>* last() { return _last; }
	void append(ListNode<T>* node) {
	    if (_last) {
		ListNode<T> *ln = _last;
		ln->_next = node;
		node->_prev = ln;
	    } else {
		node->_prev = nullptr;
		_first = node;
	    }
	    node->_next = nullptr;
	    _last = node;
	}
	ListNode<T>* pop() {
	    ListNode<T>* result = _last;
	    remove(_last);
	    return result;
	}
	void remove(ListNode<T> *node) {
	    if (node->_next) {
		node->_next->_prev = node->_prev;
	    }
	    if (node->_prev) {
		node->_prev->_next = node->_next;
	    }
	    if (_first == node) {
		_first = node->_next;
	    }
	    if (_last == node) {
		_last = node->_prev;
	    }
	}
	int length() {
	    int length = 0;
	    for (ListNode<T>* node = _first; node; node = node->_next) {
		++length;
	    }
	    return length;
	}
	void insert(ListNode<T> *next, ListNode<T> *node) {
	    ListNode<T> *prev;
	    node->_next = next;
	    if (next) {
		prev = next->_prev;
		next->_prev = node;
	    } else {
		prev = _last;
		_last = node;
	    }
	    node->_prev = prev;
	    if (prev) {
		prev->_next = node;
	    } else {
		_first = node;
	    }
	}
    };

    template<typename T, typename Timpl>
    class IteratorImpl : public Iterator<T> {
    private:
	ListNode<Timpl>* _next;
    public:
	IteratorImpl(List<Timpl>& list) {
	    _next = list.first();
	}
	bool hasMore() {
	    return nullptr != _next;
	}
	T next() {
	    T result = _next->value();
	    _next = _next->next();
	    return result;
	}
    };
};


#endif // _COM_STOCHASTIC_GLUTPP_LIST_H
