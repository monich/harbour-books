/*
 * Copyright (C) 2004-2010 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef __SHARED_PTR_H__
#define __SHARED_PTR_H__

#ifndef FBREADER_USE_GNUC_SYNC_BUILTINS
#  ifdef FBREADER_DISABLE_GNUC_SYNC_BUILTINS
#    define FBREADER_DISABLE_GNUC_SYNC_BUILTINS 0
#  endif
#  if FBREADER_DISABLE_GNUC_SYNC_BUILTINS
#    define FBREADER_USE_GNUC_SYNC_BUILTINS 0
#  else
#    ifdef __GNUC__
#      define FBREADER_USE_GNUC_SYNC_BUILTINS 1
#    else
#      define FBREADER_USE_GNUC_SYNC_BUILTINS 0
#    endif
#  endif
#endif

template<class T> class shared_ptr_storage {
	private:
		unsigned int myCounter;
		unsigned int myWeakCounter;
#if FBREADER_USE_GNUC_SYNC_BUILTINS
		unsigned int myTotalCount;
#endif
		T* myPointer;

	public:
		shared_ptr_storage(T *pointer);
		~shared_ptr_storage();

		T* pointer() const;
		T& content() const;

		void addReference();
		void addWeakReference();
		unsigned int removeReference();
		unsigned int removeWeakReference();
		unsigned int counter() const;
};

template<class T> class weak_ptr;

template<class T> class shared_ptr {
	friend class weak_ptr<T>;

	private:
		shared_ptr_storage<T> *myStorage;

		shared_ptr_storage<T> *newStorage(T *t);
		void attachStorage(shared_ptr_storage<T> *storage);
		void detachStorage();

	public:
		shared_ptr();
		shared_ptr(T *t);
		shared_ptr(const shared_ptr<T> &t);
		shared_ptr(const weak_ptr<T> &t);
		~shared_ptr();

		const shared_ptr<T> &operator = (T *t);
		const shared_ptr<T> &operator = (const shared_ptr<T> &t);
		const shared_ptr<T> &operator = (const weak_ptr<T> &t);

		T* operator -> () const;
		T& operator * () const;
		bool isNull() const;
		void reset();
		bool operator == (const weak_ptr<T> &t) const;
		bool operator != (const weak_ptr<T> &t) const;
		bool operator < (const weak_ptr<T> &t) const;
		bool operator > (const weak_ptr<T> &t) const;
		bool operator <= (const weak_ptr<T> &t) const;
		bool operator >= (const weak_ptr<T> &t) const;
		bool operator == (const shared_ptr<T> &t) const;
		bool operator != (const shared_ptr<T> &t) const;
		bool operator < (const shared_ptr<T> &t) const;
		bool operator > (const shared_ptr<T> &t) const;
		bool operator <= (const shared_ptr<T> &t) const;
		bool operator >= (const shared_ptr<T> &t) const;
};

template<class T> class weak_ptr {
	friend class shared_ptr<T>;
	private:
		shared_ptr_storage<T> *myStorage;

		void attachStorage(shared_ptr_storage<T> *storage);
		void detachStorage();

	public:
		weak_ptr();
		weak_ptr(const shared_ptr<T> &t);
		weak_ptr(const weak_ptr<T> &t);
		~weak_ptr();

		const weak_ptr<T> &operator = (const weak_ptr<T> &t);
		const weak_ptr<T> &operator = (const shared_ptr<T> &t);

		T* operator -> () const;
		T& operator * () const;
		bool isNull() const;
		void reset();

		bool operator == (const weak_ptr<T> &t) const;
		bool operator != (const weak_ptr<T> &t) const;
		bool operator < (const weak_ptr<T> &t) const;
		bool operator > (const weak_ptr<T> &t) const;
		bool operator <= (const weak_ptr<T> &t) const;
		bool operator >= (const weak_ptr<T> &t) const;
		bool operator == (const shared_ptr<T> &t) const;
		bool operator != (const shared_ptr<T> &t) const;
		bool operator < (const shared_ptr<T> &t) const;
		bool operator > (const shared_ptr<T> &t) const;
		bool operator <= (const shared_ptr<T> &t) const;
		bool operator >= (const shared_ptr<T> &t) const;
};

template<class T>
inline shared_ptr_storage<T>::shared_ptr_storage(T *pointer) {
	myPointer = pointer;
	myCounter = 0;
	myWeakCounter = 0;
#if FBREADER_USE_GNUC_SYNC_BUILTINS
	myTotalCount = 0;
#endif
}
template<class T>
inline shared_ptr_storage<T>::~shared_ptr_storage() {
}
template<class T>
inline T* shared_ptr_storage<T>::pointer() const {
	return myPointer;
}
template<class T>
inline T& shared_ptr_storage<T>::content() const {
	return *myPointer;
}
template<class T>
inline void shared_ptr_storage<T>::addReference() {
#if FBREADER_USE_GNUC_SYNC_BUILTINS
	__sync_fetch_and_add(&myCounter, 1);
	__sync_fetch_and_add(&myTotalCount, 1);
#else
	++myCounter;
#endif
}
template<class T>
inline unsigned int shared_ptr_storage<T>::removeReference() {
#if FBREADER_USE_GNUC_SYNC_BUILTINS
	if (!__sync_sub_and_fetch(&myCounter, 1)) {
		T* ptr = myPointer;
		myPointer = 0;
		delete ptr;
	}
	return __sync_sub_and_fetch(&myTotalCount, 1);
#else
	--myCounter;
	if (myCounter == 0) {
		T* ptr = myPointer;
		myPointer = 0;
		delete ptr;
	}
	return counter();
#endif
}
template<class T>
inline void shared_ptr_storage<T>::addWeakReference() {
#if FBREADER_USE_GNUC_SYNC_BUILTINS
	__sync_fetch_and_add(&myWeakCounter, 1);
	__sync_fetch_and_add(&myTotalCount, 1);
#else
	++myWeakCounter;
#endif
}
template<class T>
inline unsigned int shared_ptr_storage<T>::removeWeakReference() {
#if FBREADER_USE_GNUC_SYNC_BUILTINS
	__sync_fetch_and_sub(&myWeakCounter, 1);
	return __sync_add_and_fetch(&myTotalCount, 1);
#else
	--myWeakCounter;
	return counter();
#endif
}
template<class T>
inline unsigned int shared_ptr_storage<T>::counter() const {
#if FBREADER_USE_GNUC_SYNC_BUILTINS
	return myTotalCount;
#else
	return myCounter + myWeakCounter;
#endif
}

template<class T>
inline shared_ptr_storage<T> *shared_ptr<T>::newStorage(T *t) {
	return (t == 0) ? 0 : new shared_ptr_storage<T>(t);
}
template<class T>
inline void shared_ptr<T>::attachStorage(shared_ptr_storage<T> *storage) {
	myStorage = storage;
	if (myStorage != 0) {
		myStorage->addReference();
	}
}
template<class T>
inline void shared_ptr<T>::detachStorage() {
	if (myStorage != 0) {
		if (myStorage->removeReference() == 0) {
			delete myStorage;
		}
	}
}

template<class T>
inline shared_ptr<T>::shared_ptr() {
	myStorage = 0;
}
template<class T>
inline shared_ptr<T>::shared_ptr(T *t) {
	attachStorage(newStorage(t));
}
template<class T>
inline shared_ptr<T>::shared_ptr(const shared_ptr<T> &t) {
	attachStorage(t.myStorage);
}
template<class T>
inline shared_ptr<T>::shared_ptr(const weak_ptr<T> &t) {
	if (!t.isNull()) {
		attachStorage(t.myStorage);
	} else {
		attachStorage(0);
	}
}
template<class T>
inline shared_ptr<T>::~shared_ptr() {
	detachStorage();
}
template<class T>
inline const shared_ptr<T> &shared_ptr<T>::operator = (T *t) {
	detachStorage();
	attachStorage(newStorage(t));
	return *this;
}
template<class T>
inline const shared_ptr<T> &shared_ptr<T>::operator = (const shared_ptr<T> &t) {
	if (&t != this) {
		detachStorage();
		attachStorage(t.myStorage);
	}
	return *this;
}
template<class T>
inline const shared_ptr<T> &shared_ptr<T>::operator = (const weak_ptr<T> &t) {
	detachStorage();
	if (!t.isNull()) {
		attachStorage(t.myStorage);
	} else {
		attachStorage(0);
	}
	return *this;
}

template<class T>
inline T* shared_ptr<T>::operator -> () const {
	return (myStorage == 0) ? 0 : myStorage->pointer();
}
template<class T>
inline T& shared_ptr<T>::operator * () const {
	return myStorage->content();
}
template<class T>
inline bool shared_ptr<T>::isNull() const {
	return myStorage == 0;
}
template<class T>
inline void shared_ptr<T>::reset() {
	detachStorage();
	attachStorage(0);
}
template<class T>
inline bool shared_ptr<T>::operator == (const weak_ptr<T> &t) const {
	return operator -> () == t.operator -> ();
}
template<class T>
inline bool shared_ptr<T>::operator != (const weak_ptr<T> &t) const {
	return !operator == (t);
}
template<class T>
inline bool shared_ptr<T>::operator < (const weak_ptr<T> &t) const {
	return operator -> () < t.operator -> ();
}
template<class T>
inline bool shared_ptr<T>::operator > (const weak_ptr<T> &t) const {
	return t.operator < (*this);
}
template<class T>
inline bool shared_ptr<T>::operator <= (const weak_ptr<T> &t) const {
	return !t.operator < (*this);
}
template<class T>
inline bool shared_ptr<T>::operator >= (const weak_ptr<T> &t) const {
	return !operator < (t);
}
template<class T>
inline bool shared_ptr<T>::operator == (const shared_ptr<T> &t) const {
	return operator -> () == t.operator -> ();
}
template<class T>
inline bool shared_ptr<T>::operator != (const shared_ptr<T> &t) const {
	return !operator == (t);
}
template<class T>
inline bool shared_ptr<T>::operator < (const shared_ptr<T> &t) const {
	return operator -> () < t.operator -> ();
}
template<class T>
inline bool shared_ptr<T>::operator > (const shared_ptr<T> &t) const {
	return t.operator < (*this);
}
template<class T>
inline bool shared_ptr<T>::operator <= (const shared_ptr<T> &t) const {
	return !t.operator < (*this);
}
template<class T>
inline bool shared_ptr<T>::operator >= (const shared_ptr<T> &t) const {
	return !operator < (t);
}

template<class T>
inline void weak_ptr<T>::attachStorage(shared_ptr_storage<T> *storage) {
	myStorage = storage;
	if (myStorage != 0) {
		myStorage->addWeakReference();
	}
}
template<class T>
inline void weak_ptr<T>::detachStorage() {
	if (myStorage != 0) {
		if (myStorage->removeWeakReference() == 0) {
			delete myStorage;
		}
	}
}

template<class T>
inline weak_ptr<T>::weak_ptr() {
	myStorage = 0;
}
template<class T>
inline weak_ptr<T>::weak_ptr(const shared_ptr<T> &t) {
	attachStorage(t.myStorage);
}
template<class T>
inline weak_ptr<T>::weak_ptr(const weak_ptr<T> &t) {
	if (!t.isNull()) {
		attachStorage(t.myStorage);
	} else {
		attachStorage(0);
	}
}
template<class T>
inline weak_ptr<T>::~weak_ptr() {
	detachStorage();
}

template<class T>
inline const weak_ptr<T> &weak_ptr<T>::operator = (const weak_ptr<T> &t) {
	if (&t != this) {
		detachStorage();
		if (!t.isNull()) {
			attachStorage(t.myStorage);
		} else {
			attachStorage(0);
		}
	}
	return *this;
}
template<class T>
inline const weak_ptr<T> &weak_ptr<T>::operator = (const shared_ptr<T> &t) {
	detachStorage();
	attachStorage(t.myStorage);
	return *this;
}

template<class T>
inline T* weak_ptr<T>::operator -> () const {
	return (myStorage == 0) ? 0 : myStorage->pointer();
}
template<class T>
inline T& weak_ptr<T>::operator * () const {
	return myStorage->content();
}
template<class T>
inline bool weak_ptr<T>::isNull() const {
	return (myStorage == 0) || (myStorage->pointer() == 0);
}
template<class T>
inline void weak_ptr<T>::reset() {
	detachStorage();
	attachStorage(0);
}
template<class T>
inline bool weak_ptr<T>::operator == (const weak_ptr<T> &t) const {
	return operator -> () == t.operator -> ();
}
template<class T>
inline bool weak_ptr<T>::operator != (const weak_ptr<T> &t) const {
	return !operator == (t);
}
template<class T>
inline bool weak_ptr<T>::operator < (const weak_ptr<T> &t) const {
	return operator -> () < t.operator -> ();
}
template<class T>
inline bool weak_ptr<T>::operator > (const weak_ptr<T> &t) const {
	return t.operator < (*this);
}
template<class T>
inline bool weak_ptr<T>::operator <= (const weak_ptr<T> &t) const {
	return !t.operator < (*this);
}
template<class T>
inline bool weak_ptr<T>::operator >= (const weak_ptr<T> &t) const {
	return !operator < (t);
}
template<class T>
inline bool weak_ptr<T>::operator == (const shared_ptr<T> &t) const {
	return operator -> () == t.operator -> ();
}
template<class T>
inline bool weak_ptr<T>::operator != (const shared_ptr<T> &t) const {
	return !operator == (t);
}
template<class T>
inline bool weak_ptr<T>::operator < (const shared_ptr<T> &t) const {
	return operator -> () < t.operator -> ();
}
template<class T>
inline bool weak_ptr<T>::operator > (const shared_ptr<T> &t) const {
	return t.operator < (*this);
}
template<class T>
inline bool weak_ptr<T>::operator <= (const shared_ptr<T> &t) const {
	return !t.operator < (*this);
}
template<class T>
inline bool weak_ptr<T>::operator >= (const shared_ptr<T> &t) const {
	return !operator < (t);
}

#endif /* __SHARED_PTR_H__ */
