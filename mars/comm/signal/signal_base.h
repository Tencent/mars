/*
 ============================================================================
 Name		: signal_base.h
 Author		: peterfan
 Version	: 1.0
 Created on	: 2010-4-28
 Copyright	: Copyright (C) 1998 - 2009 TENCENT Inc. All Right Reserved
 Description:
 ============================================================================
 */

#ifndef _SIGNALSLOT_SIGNAL_BASE_H_
#define _SIGNALSLOT_SIGNAL_BASE_H_

#include "slist.h"

namespace owl {

struct slot_tracker {
	virtual ~slot_tracker() {}
	virtual void disconnect(const void* object) = 0;
};


class trackable {
public:
	trackable():this_(0) {}
	~trackable() { disconnect_all_tracker(); }

	void append_tracker(slot_tracker* tracker) const {
		if (!tracker_list_.exists(tracker))
			tracker_list_.prepend(tracker);
	}

	void remove_tracker(slot_tracker* tracker) const {
		tracker_list_.remove(tracker);
	}

	void set_this(const void* object) const {
		this_ = object;
	}

private:
	void disconnect_all_tracker() {
		slist<slot_tracker*>::iterator_type it = tracker_list_.iterator();
		while (it.has_next()) {
			it.next()->disconnect(this_);
		}
        tracker_list_.clear();
	}

private:
	mutable slist<slot_tracker*> tracker_list_;
	mutable const void* this_;
};


struct slot_function {
	slot_function(const function_holder& holder, const void* trackable):holder_(holder), trackable_(trackable), deleted_(false) {}
	~slot_function() {}

	bool operator==(const slot_function& rhs) const {
		return holder_ == rhs.holder_;
	}

	bool operator!=(const slot_function& rhs) const {
		return !(*this == rhs);
	}

	function_holder holder_;
	const void* trackable_;
	bool deleted_;
};


struct is_same_receiver {
	is_same_receiver(const void* receiver):receiver_(receiver) {}
	~is_same_receiver() {}

	bool operator()(const slot_function& slot) const {
		return receiver_ != 0 && receiver_ == slot.holder_.object;
	}

	const void* receiver_;
};


struct is_deleted_slot {
	bool operator()(const slot_function& slot) const {
		return slot.deleted_;
	}
};


struct slot_list {
	slot_list():signal_destroyed(false) {}
	~slot_list() {}

	slist<slot_function> list;
	bool signal_destroyed;
};


class signal_base: public slot_tracker {
protected:
	typedef slist<slot_function>::iterator_type iterator_type;

public:
	signal_base():slot_list_(0), is_blocked_(false), has_deleted_slot_(false) {
		slot_list_ = new slot_list();
	}

	virtual ~signal_base() {
		disconnect_all();
		if (is_blocked_) {
			slot_list_->signal_destroyed = true;
		}
		else {
			delete slot_list_;
		}
	}

	virtual void disconnect(const void* object) {
		iterator_type it = slot_list_->list.iterator();
		while (it.has_next()) {
			slot_function& s = it.next();
			if (s.holder_.object == object) {
				s.deleted_ = true;
				has_deleted_slot_ = true;
				if (s.trackable_) {
					const trackable* t = static_cast<const trackable*>(s.trackable_);
					t->remove_tracker(this);
				}
			}
		}
	}
    
    void disconnect_all() {
        iterator_type it = slot_list_->list.iterator();
        while (it.has_next()) {
            slot_function& s = it.next();
            s.deleted_ = true;
            if (s.trackable_) {
                const trackable* t = static_cast<const trackable*>(s.trackable_);
                t->remove_tracker(this);
            }
        }
        has_deleted_slot_ = true;
    }

	void block() {
		is_blocked_ = true;
	}

	void unblock() {
		is_blocked_ = false;
	}

	bool is_blocked() const {
		return is_blocked_;
	}

protected:
	void connect_slot(const function_holder& holder, const void* trackable) {
		slot_function slot(holder, trackable);
		slot_list_->list.prepend(slot);
	}

	void disconnect_slot(const function_holder& holder) {
		slot_function slot(holder, 0);
		iterator_type it = slot_list_->list.iterator();
		while (it.has_next()) {
			slot_function& s = it.next();
			if (s == slot) {
				s.deleted_ = true;
				has_deleted_slot_ = true;
				if (s.trackable_) {
					const trackable* t = static_cast<const trackable*>(s.trackable_);
					t->remove_tracker(this);
				}
			}
		}
	}

	void clear_slot_list(slot_list* list) {
		if (list->signal_destroyed) {
			delete list;
		}
		else if (has_deleted_slot_) {
			list->list.remove_if(is_deleted_slot());
			has_deleted_slot_ = false;
		}
	}

private:
	signal_base(const signal_base&);
	signal_base& operator=(const signal_base&);

protected:
	slot_list* slot_list_;
	bool is_blocked_;
	bool has_deleted_slot_;
};

} //namespace owl

#endif /* _SIGNALSLOT_SIGNAL_BASE_H_ */
