// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "dynamic_bitset.h"
#include <algorithm>

template<typename TElement, typename TBlock = uint64_t>
class BitSet
{
public:
	typedef dynamic_bitset<TBlock> TStore;
	typedef BitSet<TElement, TBlock> TSet;

private:
	TStore store;

public:
	
	struct hash
	{
		size_t operator()(const TSet& _Keyval) const
		{	
			TStore::hash h;
			return h(_Keyval.store);
		}
	};

	explicit BitSet(size_t elements) 
		: store(elements)
	{
	}

	TElement FindNext(TElement element) const
	{
		store.find_next(element);
	}

	void Clear()
	{
		store.reset();
	}

	void Add(const TElement& element)
	{
		store.set(element);
	}

	void Remove(const TElement& element)
	{
		store.reset(element);
	}

	void UnionWith(const TSet& other)
	{
		store |= other.store;
	}

	void IntersectWith(const TSet& other)
	{
		store &= other.store;
	}

	void DifferenceWith(const TSet& other)
	{
		 store -= other.store;
	}

	void SymetricDifferenceWith(const TSet& other)
	{
		store ^= other.store;
	}

	bool Contains(const TElement& element) const
	{
		return store.test(element);
	}

	bool IsEmpty() const 
	{
		return store.none();
	}

	size_t Count() const 
	{
		return store.count();
	}

	static TSet Union(const TSet& lh, const TSet& rh)
	{
		TSet new_set(lh);
		new_set.UnionWith(rh);
		return new_set;
	}

	static TSet Intersect(const TSet& lh, const TSet& rh)
	{
		TSet new_set(lh);
		new_set.IntersectWith(rh);
		return new_set;
	}

	static TSet Difference(const TSet& lh, const TSet& rh)
	{
		TSet new_set(lh);
		new_set.DifferenceWith(rh);
		return new_set;
	}

	static TSet SymetricDifference(const TSet& lh, const TSet& rh)
	{
		TSet new_set(lh);
		new_set.SymetricDifferenceWith(rh);
		return new_set;
	}

	class Iterator
	{
	public:
	private:
		typename TStore::size_type i;
		const TStore* col;

		Iterator(typename TStore::size_type begin, const TStore& store)
			: i(begin), col(&store)
		{
		}

		friend class TSet;

	public:
		void MoveNext()
		{
			i = col->find_next(i);
		}

		bool IsEnd() const
		{
			return i == TStore::npos;
		}

		TElement GetCurrent() const
		{
			return static_cast<TElement>(i);
		}

		bool Equals(const Iterator& rh) const
		{
			return rh.i == i;
		}

		bool operator ==(const Iterator& rh) const
		{
			return Equals(rh);
		}

		bool operator !=(const Iterator& rh) const
		{
			return !Equals(rh);
		}
	};

	Iterator GetIterator() const 
	{
		return Iterator(store.find_first(), store);
	}

	bool operator==(const TSet& rh) const
	{
		return store == rh.store;
	}

	bool operator!=(const TSet& rh) const 
	{
		return store != rh.store;
	}

	bool operator<(const TSet& rh) const
	{
		return store < rh.store;
	}

	bool operator>(const TSet& rh) const
	{
		return store > rh.store;
	}
};

#include <set>
#include <algorithm>

template
<
	typename TElement, 
	class TElementCompare = std::less<TElement>, 
	class TAllocator = std::allocator<TElement>
>
class Set
{
public:
	typedef Set<TElement,TElementCompare,TAllocator> TSet;
	typedef std::set<TElement,TElementCompare,TAllocator> TStore;
	typedef typename TStore::iterator TStoreIterator;

private:
	TStore store;

public:
	Set(size_t capacity)
	{
		// cannot use the capacity hint
	}

	void Clear()
	{
		store.clear();
	}

	void Add(const TElement& element)
	{
		store.insert(element);
	}

	void Remove(const TElement& element)
	{
		store.erase(element);
	}

	void UnionWith(const TSet& other)
	{
		store.insert(other.store.begin(), other.store.end());
	}

	void IntersectWith(const TSet& other)
	{
		for(auto i=store.begin(); i!=store.end(); )
		{
			if(other.Contains(*i)) 
			{
				i++;
			} else {
				i = store.erase(i);
			}
		}
	}

	void DifferenceWith(const TSet& other)
	{
		store.erase(other.begin(), other.end());
	}

	void SymetricDifference(const TSet& other) 
	{
		throw;
	}

	bool Contains(const TElement& element) const
	{
		auto r = store.find(element);
		return r != store.end();
	}

	bool IsEmpty() const
	{
		return store.size() == 0;
	}

	size_t Count() const
	{
		return store.size();
	}

	static TSet Union(const TSet& lh, const TSet& rh)
	{
		TSet new_set;
		std::set_union
		(
			lh.store.begin(), lh.store.end(), 
			rh.store.begin(), rh.store.end(), 
			std::inserter(new_set.store, new_set.store.begin())
		);
		return new_set;
	}

	static Set<TElement> Intersect(const Set<TElement>& lh, const Set<TElement>& rh)
	{
		Set<TElement> new_set;
		std::set_intersection
		(
			lh.store.begin(), lh.store.end(), 
			rh.store.begin(), rh.store.end(), 
			std::inserter(new_set.store, new_set.store.begin())
		);
		return new_set;
	}

	static Set<TElement> Difference(const Set<TElement>& lh, const Set<TElement>& rh)
	{
		Set<TElement> new_set;
		std::set_difference
		(
			lh.store.begin(), lh.store.end(), 
			rh.store.begin(), rh.store.end(), 
			std::inserter(new_set.store, new_set.store.begin())
		);
		return new_set;
	}

	static Set<TElement> SymetricDifference(const Set<TElement>& lh, const Set<TElement>& rh)
	{
		Set<TElement> new_set;
		std::set_symmetric_difference
		(
			lh.store.begin(), lh.store.end(), 
			rh.store.begin(), rh.store.end(), 
			std::inserter(new_set.store, new_set.store.begin())
		);
		return new_set;
	}

	class Iterator
	{
	public:
	private:
		TStoreIterator i;
		const TStore* col;

		Iterator(TStoreIterator it, const TStore& c)
			: col(&c), i(it)
		{
		}

		friend class TSet;

	public:

		void MoveNext()
		{
			i++;
		}

		bool IsEnd() const
		{
			return i == col->end();
		}

		const TElement& GetCurrent() const
		{
			return *i;
		}

		bool Equals(const Iterator& rh) const
		{
			return i == rh.i;
		}

		bool operator==(const Iterator& rh) const
		{
			return Equals(rh);
		}

		bool operator!=(const Iterator& rh) const
		{
			return !Equals(rh);
		}
	};

	Set(Iterator begin, Iterator end)
		: store(begin.i, end.i)
	{
	}


	Iterator GetIterator() const
	{
		return Iterator(store.begin(), store);
	}

	Iterator FindLower(const TElement& element) const
	{
		auto i = store.lower_bound(element);
		return Iterator(i, store);
	}

	Iterator FindUpper(const TElement& element) const
	{
		auto i = store.upper_bound(element);
		return Iterator(i, store);
	}

	struct hash
	{
		size_t operator()(const TSet& _Keyval) const
		{	
		#ifdef _M_X64
			static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
			const size_t _FNV_offset_basis = 14695981039346656037ULL;
			const size_t _FNV_prime = 1099511628211ULL;

		 #else /* _M_X64 */
			static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
			const size_t _FNV_offset_basis = 2166136261U;
			const size_t _FNV_prime = 16777619U;
		 #endif /* _M_X64 */

			size_t _Val = _FNV_offset_basis;

			for(auto i=_Keyval.GetIterator(); !i.IsEnd(); i.MoveNext())
			{
				// fold in another value		
				auto z = i.GetCurrent();
				auto ss = sizeof(z);
				while(ss--) {
					_Val ^= (size_t)(z & 0xFF);
					_Val *= _FNV_prime;
					z >>= 8;
				}
			}

		 #ifdef _M_X64
			static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
			_Val ^= _Val >> 32;

		 #else /* _M_X64 */
			static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
		 #endif /* _M_X64 */

			return (_Val);
		}
	};
};