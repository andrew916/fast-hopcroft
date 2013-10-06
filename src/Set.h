// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "dynamic_bitset.h"
#include <algorithm>
#include <string>
#include <boost/functional/hash.hpp>

template<typename _TElement, typename TBlock = uint64_t>
class BitSet
{
public:
	typedef _TElement TElement;
	typedef dynamic_bitset<TElement, TBlock> TStore;
	typedef BitSet<TElement, TBlock> TSet;

private:
	TStore store;

public:
	
	struct hash
	{
		size_t operator()(const TSet& _Keyval) const
		{	
			return _Keyval.store.hash_value();
		}
	};

	explicit BitSet(TElement elements) 
		: store(elements)
	{
	}

	TElement FindNext(TElement element) const
	{
		store.find_next(element);
	}

	TElement GetElementAt(TElement pos) const 
	{
		return store.element_in_position(pos);
	}

	void Clear()
	{
		store.clear();
	}

	void Add(const TElement& element)
	{
		store.add(element);
	}

	void Remove(const TElement& element)
	{
		store.remove(element);
	}

	bool TestAndAdd(const TElement& element)
	{
		return store.test_and_add(element);
	}

	bool TestAndRemove(const TElement& element)
	{
		return store.test_and_remove(element);
	}

	void UnionWith(const TSet& other)
	{
		store.union_with(other.store);
	}

	void IntersectWith(const TSet& other)
	{
		store.intersect_with(other.store);
	}

	void DifferenceWith(const TSet& other)
	{
		 store.difference_with(other.store);
	}

	void SymetricDifferenceWith(const TSet& other)
	{
		store.symetric_difference_with(other.store);
	}

	void Complement()
	{
		store.complement();
	}

	bool Contains(const TElement& element) const
	{
		return store.contains(element);
	}

	bool IsEmpty() const 
	{
		return store.none();
	}

	TElement Count() const 
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
		
		typename TStore::iterator i;
		
		Iterator(const typename TStore::iterator& _i)
			: i(_i)
		{
		}

		friend TSet;

	public:
		void MoveNext()
		{
			++i;
		}

		bool IsEnd() const
		{
			return i.is_end();
		}

		TElement GetCurrent() const
		{
			return static_cast<TElement>(*i);
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
		return Iterator(store.begin());
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

	std::string to_string()
	{
		std::string str("{");
		bool b = false;
		for(auto i=GetIterator(); !i.IsEnd(); i.MoveNext())
		{			
			if(b) str.append(", ");
			b = true;
			str.append(std::to_string(static_cast<size_t>(i.GetCurrent())));			
		}
		str.append("}");
		return str;
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

	Set()
	{
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

		friend TSet;

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
			return boost::hash_value(_Keyval.store);			
		}
	};

	std::string to_string()
	{
		std::string str("{");
		bool b = false;
		for(auto i=GetIterator(); !i.IsEnd(); i.MoveNext())
		{			
			if(b) str.append(", ");
			b = true;
			str.append(std::to_string(static_cast<size_t>(i.GetCurrent())));			
		}
		str.append("}");
		return str;
	}

	bool operator ==(const TSet& rh) const
	{
		using namespace std;
		if(rh.store.size() != store.size()) return false;
		if(rh.store.empty()) return true;
		return equal(store.begin(), store.end(), rh.store.begin());
	}
};
