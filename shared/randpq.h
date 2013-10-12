/*
 Copyright 2004-2008 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.




This software implements a platform independent C++ random access priority queue.
*/
#ifndef _RANDPQ_H_
#define _RANDPQ_H_

#include <list>

/*
template<class ITEM>
unsigned long operator-(const std::list<ITEM,std::allocator<ITEM> >::iterator& lhs, const std::list<ITEM,std::allocator<ITEM> >::iterator& rhs)
{
	std::list<ITEM>::iterator	tmp = rhs;
	unsigned long 				distance;
	for(distance=0; tmp != rhs; distance++)
		tmp++
	return distance
}

template<class ITEM>
std::list<ITEM>::iterator operator+(std::list<ITEM,std::allocator<ITEM> >::iterator& lhs, int offset)
{
	std::list<ITEM>::iterator ret = lhs;
	if (offset > 0)
		for( ;offset>0;offset--)
			ret++;
	else
		for( ;offset<0;offset++)
			ret--;
	return ret;
}
*/

template<class ITEM >
class ra_priority_queue
{

public: // declaration
	typedef	std::list<ITEM>	storage;

	struct position {
		position() {
			m_refcount = 0;
		}

		position(const typename storage::iterator& pos) {
			m_pos = pos;
			m_refcount = 0;
			m_invalid = false;
		}

		position(const position& src) {
			operator=(src);
		}

		const position& operator=(const position& src) {
			m_pos = src.m_pos;
			m_refcount = src.m_refcount;
			m_invalid = src.m_invalid;
			return *this;
		}

		void incRef() {
			++m_refcount;
		}

		void decRef() {
			--m_refcount;
		}

		bool isDisposable() const {
			return m_refcount <= 0;
		}

		bool isSingle() const {
			return m_refcount == 1;
		}

		void invalidate() {
			m_invalid = true;
		}

		bool isValid() {
			return !m_invalid;
		}

		void setOffset(const typename storage::iterator& pos) {
			m_pos = pos;
			m_invalid = false;
		}

		bool						m_invalid;
		typename storage::iterator	m_pos;
		int							m_refcount;	
	};


	typedef std::list<position>				positionlist;
	typedef typename positionlist::iterator	position_ptr;

	class cursor {
	protected:
	public: // implementation
		cursor() { 
			init();
		}
		cursor(const cursor& src) {
			init();
			copy(src);
		}
		~cursor() {
			release();
		}
		const cursor& operator=(const cursor& src) {
			copy(src);
			return *this;
		}

		ITEM& operator*() {
			return *(m_ppos->m_pos);
		}
		const ITEM& operator*() const {
			return *(m_ppos->m_pos);
		}

		ITEM* operator->() {
			return &(*(m_ppos->m_pos));
		}
		const ITEM* operator->() const {
			return &(*(m_ppos->m_pos));
		}

		operator bool() const {
			return m_data_source != NULL && m_ppos->isValid();
		}

		bool operator==(const cursor& src) const {
			return m_data_source == src.m_data_source && (m_data_source == NULL || m_ppos->m_pos == src.m_ppos->m_pos);
		}

		bool operator!=(const cursor& src) const {
			return !operator==(src);
		}

		cursor& operator++() {
			m_data_source->makeWritable(m_ppos);
			++(m_ppos->m_pos);
			return *this;
		}

		cursor operator++(int) {
			cursor tmp = *this;
			operator++();
			return tmp;
		}

	protected: // implementation
		explicit cursor(ra_priority_queue<ITEM> *data_source, typename storage::iterator pos) {
			m_data_source	= data_source;
			m_ppos = data_source->getPosition(pos);
		}

		void copy(const cursor& src) {
			release();

			m_data_source	= src.m_data_source;
			if (m_data_source) {
				m_ppos = m_data_source->getCopy(src.m_ppos);
			}
		}
		
		void init() {
			m_data_source = NULL;
		}

		void release() {
			if (m_data_source) {
				m_data_source->release(m_ppos);
			}
			m_data_source = NULL;
		}

		typename storage::iterator& getOffset() const {
			return m_ppos->m_pos;
		}

		void setOffset(const typename storage::iterator& pos) {
			m_ppos->setOffset(pos);
		}

	protected: // storage
		position_ptr			m_ppos;
		ra_priority_queue<ITEM>	*m_data_source;

		friend class ra_priority_queue<ITEM>;
	};

public: // implementation
	ra_priority_queue() {
		init();
	}

	ITEM& front() {
		return *m_storage.begin();
	}

	void dequeue() {
		invalidate(m_storage.erase(m_storage.begin()));
	}

	cursor enqueue(const ITEM& item)	{
		typename storage::iterator	i = lower_bound(item);
		i = m_storage.insert(i,item);
		return cursor(this, i);
	}

	bool empty()	{ return m_storage.begin() == m_storage.end(); }

	cursor& begin() {
		m_begin.setOffset(m_storage.begin()); // incase it has been moved
		return m_begin;
	}

	cursor& end() {
		m_end.setOffset(m_storage.end());
		return m_end;
	}

	cursor erase(const cursor& cur) {
		if (validate(cur)) {
 			typename storage::iterator i = cur.getOffset();
			invalidate(i); // erase this item.
			i = m_storage.erase(i);
			return cursor(this, i);
		}
		return end();
	}

	bool validate(const cursor& cur) const {
		return (bool)cur;
	}

	size_t capacity() { 
		return m_storage.capacity();
	}

	size_t size() {
		return m_storage.size();
	}

	size_t nel() {
		return m_storage.end() - m_storage.begin();
	}

protected: // implementation
	void init() {
		m_begin = cursor(this,m_storage.begin());
		m_end = cursor(this,m_storage.end());
 	}

	typename storage::iterator lower_bound(const ITEM& item) {
		typename storage::reverse_iterator	i = m_storage.rbegin();
		while ( i != m_storage.rend() && item < *i)
			++i;
		return i.base();
	}
	
	void invalidate(typename storage::iterator pos) {
		for (typename positionlist::iterator i = m_positions.begin(); i != m_positions.end(); i++) {
			if (i->isValid() && i->m_pos == pos) {
				// Invalidate any cursors positions pointing to this position in the queue
				i->invalidate();
			}
		}
	}
	
	void makeWritable(position_ptr& ppos) {
		position& pos = *ppos;
		if (pos.isSingle())
			return;
		
		// create a duplicate, writable position
		typename storage::iterator savPos = pos.m_pos;
		release(ppos); // release the multiply referenced position.
		ppos = getPosition(savPos);
	}

	position_ptr getCopy(const position_ptr& ppos) {
		position_ptr ret = ppos;
		ret->incRef();
		return ret;
	}

	position_ptr getPosition(const typename storage::iterator& ipos) {
		position	pos(ipos);
		pos.incRef();
		m_positions.push_front(pos);
		return m_positions.begin();
	}

	void release(position_ptr& ppos) {
		position& pos = *ppos;
		pos.decRef();
		if (pos.isDisposable())
			m_positions.erase(ppos);
	}
protected: // storage
	storage			m_storage;
	positionlist	m_positions;
	friend			class cursor;

private:
	cursor			m_begin, m_end;
};

#endif
