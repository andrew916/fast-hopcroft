// Septiembre 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <algorithm>
#include "Set.h"

// Incremental Almeida et al. Minimization Algorithm.
template<typename TDfa>
class MinimizationIncremental
{
public:
	typedef TDfa TDfa;
	typedef typename TDfa::TState TState;
	typedef typename TDfa::TSymbol TSymbol;
	
	class NumericPartition
	{
	public:
		typedef TState TElement;
		typedef std::vector<std::list<TElement>> TStore;

	private:
		std::vector<TElement> membership;
		TStore storage;
		TElement size;

	public:
		NumericPartition(TElement size_)
			: size(size_), membership(size_), storage(size_)
		{
			Clear(size);
		}

		NumericPartition() 
			: size(0)
		{
		}

		void Clear(TElement size)
		{
			using namespace std;
			membership.resize(size);
			storage.resize(size);
			for(TElement i=0; i<size; i++)
			{
				membership[i] = i;
				storage[i].clear();
				storage[i].emplace_back(i);
			}
		}

		TElement Find(TElement e) const
		{
			return membership[e];
		}

		TElement Union(TElement i, TElement j)
		{
			using namespace std;
			// concatenate list j into i
			storage[i].splice(storage[i].begin(), storage[j]);
			// replace i using j
			replace(membership.begin(), membership.end(), i, j);			
			size--;
			return i;
		}

		const TStore& GetStore() const
		{
			return storage;
		}

		const TElement GetSize() const 
		{
			return size;
		}
	};

	typedef typename NumericPartition::TStore TNumericPartitionStore;
private:
	
	template<typename TPairIndex>
	bool EquivP(TState p, TState q, const TDfa& dfa, const NumericPartition& part, BitSet<TPairIndex>& equiv, BitSet<TPairIndex>& path)
	{
		TState states = dfa.GetStates();
		TPairIndex root_pair = p*states+q;
		if(path.Contains(root_pair)) return true;
		path.Add(root_pair);
		for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
		{
			TState sp = dfa.GetSuccessor(p,a);
			TState sq = dfa.GetSuccessor(q,a);
			sp = part.Find(sp);
			sq = part.Find(sq);
			if(sp == sq) continue;
			if(sp > sq) std::swap(sp, sq);
			TPairIndex pair = sp*states+sq;
			if(!equiv.Contains(pair))
			{
				equiv.Add(pair);
				if(!EquivP(sp,sq, dfa, part, equiv, path))
				{
					return false;
				}
				path.Remove(pair);
			}
		}
		equiv.Add(root_pair);
		return true;
	}

public:

	void Minimize(const TDfa& dfa, NumericPartition& part)
	{
		typedef uint64_t TPairIndex;
		TState states = dfa.GetStates();
		part.Clear(states);
		BitSet<TPairIndex> neq(states*states);
		
		for(TState q=0; q<states; q++)
		{
			bool fq = dfa.IsFinal(q);
			for(TState p=1; p<q; p++)
			{
				bool fp = dfa.IsFinal(p);
				if((fp && !fq) || (!fp && fq))
				{
					neq.Add(p*states+q);
				}
			}
		}
		BitSet<TPairIndex> equiv(states*states);
		BitSet<TPairIndex> path(states*states);
		for(TState p=0; p<states; p++)
		{		
			for(TState q=p+1; q<states; q++)
			{
				if(neq.Contains(p*states+q)) continue;
				if(part.Find(p) == part.Find(q)) continue;
				equiv.Clear();
				path.Clear();
				assert(!neq.Contains(p*states+q));
				bool isEquiv = EquivP(p, q, dfa, part, equiv, path);
				auto it = isEquiv ? equiv.GetIterator() : path.GetIterator();
				for(; !it.IsEnd(); it.MoveNext())
				{
					TPairIndex idx = it.GetCurrent();
					TState q_prime = idx % states;
					TState p_prime = idx / states;
					if(isEquiv) 
					{
						part.Union(p_prime, q_prime);
					} else {
						neq.Add(p_prime*states+q_prime);
					}
				}
			}
		}
	}

	TDfa BuildDfa(const TDfa& dfa, const NumericPartition& seq)
	{
		TDfa dfa_min(dfa.GetAlphabetLength(), seq.GetSize());
		for(auto k=dfa.GetInitials().GetIterator(); !k.IsEnd(); k.MoveNext())
		{
			TState initial_state = k.GetCurrent();
			initial_state = seq.Find(initial_state);
			dfa_min.SetInitial(initial_state);
		}
		for(auto k=dfa.GetFinals().GetIterator(); !k.IsEnd(); k.MoveNext())
		{
			TState final_state = k.GetCurrent();
			final_state = seq.Find(final_state);
			dfa_min.SetFinal(final_state);
		}
		
		for(auto i : seq.GetStore())
		{
			if(i.empty()) continue;
			TState src = *i.begin();
			TState src_c = seq.Find(src);
			for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
			{
				TState tgt = dfa.GetSuccessor(src, a);
				TState tgt_c = seq.Find(tgt);
				dfa_min.SetTransition(src_c, a, tgt_c);
			}			
		}
		return dfa_min;
	}

	TDfa Minimize(const TDfa& dfa)
	{
		NumericPartition part;
		Minimize(dfa, part);
		TDfa dfa_min = BuildDfa(dfa, seq);
		return dfa_min;
	}
};
