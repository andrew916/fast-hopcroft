// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "dynamic_bitset.h"

template<typename TElement>
class BitSet
{
private:
	dynamic_bitset<TElement> store;

public:
	void Add(const TElement& element)
	{
		store.set(element);
	}

	void Remove(const TElement& element)
	{
		store.clear(element);
	}

	void UnionWith(const Set<TElement>& other)
	{
		store |= other.store;
	}

	void IntersectWith(const Set<TElement>& other)
	{
		store &= other.store;
	}

	void DifferenceWith(const Set<TElement>& other)
	{
		 store -= other.store;
	}

	bool Contains(const TElement& element) const
	{
		return store.test(element);
	}

	size_t Count() const 
	{
		return store.count();
	}

	static Set<TElement> Union(const Set<TElement>& lh, const Set<TElement>& rh)
	{
		Set<TElement> new_set(lh);
		new_set.UnionWith(rh);
		return new_set;
	}

	static Set<TElement> Intersect(const Set<TElement>& lh, const Set<TElement>& rh)
	{
		Set<TElement> new_set(lh);
		new_set.IntersectWith(rh);
		return new_set;
	}

	static Set<TElement> Difference(const Set<TElement>& lh, const Set<TElement>& rh)
	{
		Set<TElement> new_set(lh);
		new_set.DifferenceWith(rh);
		return new_set;
	}
}

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
private:
	std::set<TElement, TElementCompare, TAllocator> store;

public:
	void Add(const TElement& element)
	{
		store.insert(element);
	}

	void Remove(const TElement& element)
	{
		store.erase(element);
	}

	void UnionWith(const Set<TElement>& other)
	{
		store.insert(other.begin(), other.end());
	}

	void IntersectWith(const Set<TElement>& other)
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

	void DifferenceWith(const Set<TElement>& other)
	{
		store.erase(other.begin(), other.end());
	}

	bool Contains(const TElement& element) const
	{
		auto r = store.find(element);
		return r != store.end();
	}

	size_t Count() const
	{
		return store.size();
	}

	static Set<TElement> Union(const Set<TElement>& lh, const Set<TElement>& rh)
	{
		Set<TElement> new_set;
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
		std::set_intersect
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
};