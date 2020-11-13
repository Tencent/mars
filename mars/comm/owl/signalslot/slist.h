/*
 ============================================================================
 Name		: slist.h
 Author		: peterfan
 Version	: 1.0
 Created on	: 2010-4-28
 Copyright	: Copyright (C) 1998 - 2009 TENCENT Inc. All Right Reserved
 Description:
 ============================================================================
 */

#ifndef _SIGNALSLOT_SLIST_H_
#define _SIGNALSLOT_SLIST_H_

namespace owl {

struct slist_node_base {
    slist_node_base():next(0) {}
    ~slist_node_base() {}
    
    slist_node_base* next;
};


template <typename T>
struct slist_node: public slist_node_base {
    slist_node(const T& v):value(v) {}
    ~slist_node() {}
    
    T value;
};


template <typename T>
class slist_iterator {
public:
    typedef slist_node<T> node_type;
    
    slist_iterator(slist_node_base* node):node_(node) {}
    ~slist_iterator() {}
    
    bool has_next() const {
        return node_ != 0 && node_->next != 0;
    }
    
    T& next() {
        node_ = node_->next;
        return static_cast<node_type*>(node_)->value;
    }
    
private:
    slist_node_base* node_;
};


template <typename T>
class slist {
private:
	slist(const slist&);
	slist& operator=(const slist&);

public:
	typedef slist_node<T> node_type;
	typedef slist_iterator<T> iterator_type;

	slist() {}
	~slist() { destory_slist(head_.next); }

	bool empty() const { return head_.next == 0; }
	int size() const { return slist_size(head_.next); }
	iterator_type iterator() { return iterator_type(&head_); }

	void prepend(const T& value) {
		node_type* node = new node_type(value);
		make_link(&head_, node);
	}

	void remove(const T& value) {
		slist_node_base* node = &head_;
		while (node != 0 && node->next != 0) {
			if (static_cast<node_type*>(node->next)->value == value)
				erase_after(node);
			else
				node = node->next;
		}
	}

	template <typename Predicate>
	void remove_if(Predicate predicate) {
		slist_node_base* node = &head_;
		while (node != 0 && node->next != 0) {
			if (predicate(static_cast<node_type*>(node->next)->value))
				erase_after(node);
			else
				node = node->next;
		}
	}

	bool exists(const T& value) {
		slist_node_base* node = &head_;
		while (node != 0 && node->next != 0) {
			if (static_cast<node_type*>(node->next)->value == value)
				return true;
			node = node->next;
		}
		return false;
	}

private:
	static void make_link(slist_node_base* prev_node, slist_node_base* new_node) {
		new_node->next = prev_node->next;
		prev_node->next = new_node;
	}

	static int slist_size(slist_node_base* node) {
		int result = 0;
		for (; node != 0; node = node->next)
			++result;
		return result;
	}

	static void destory_slist(slist_node_base* node) {
		while (node != 0) {
			slist_node_base* temp = node;
			node = node->next;
			delete static_cast<node_type*>(temp);
		}
	}

	static void erase_after(slist_node_base* node) {
		slist_node_base* next = node->next;
		node->next = next->next;
		delete static_cast<node_type*>(next);
	}

private:
	slist_node_base head_;
};

} //namespace owl

#endif /* _SIGNALSLOT_SLIST_H_ */
