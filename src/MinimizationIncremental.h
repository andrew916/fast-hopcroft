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
	typedef uint64_t TPairIndex;
	
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
			: membership(size_), storage(size_)
		{
			Clear(size);
		}

		NumericPartition() 
			: size(0)
		{
		}

		void Clear(TElement size_)
		{
			using namespace std;
			size = size_;
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
			i = Find(i);
			j = Find(j);
			if(i == j) return i;
			assert(!storage[i].empty());
			assert(!storage[j].empty());
			// concatenate list j into i
			storage[i].splice(storage[i].begin(), storage[j]);
			// replace i using j
			replace(membership.begin(), membership.end(), j, i);			
			size--;
			return i;
		}

		void Compact()
		{
			// start scan from begin
			auto empty_it=storage.begin();
			while(empty_it != storage.end())
			{
				// find empty place
				while(empty_it != storage.end() && !empty_it->empty()) empty_it++;
				if(empty_it == storage.end()) break;
				auto empty_idx = distance(storage.begin(), empty_it);
				// find non-empty
				auto non_empty_it = next(empty_it);
				while(non_empty_it != storage.end() && non_empty_it->empty()) non_empty_it++;
				if(non_empty_it == storage.end()) break;
				auto non_empty_idx = distance(storage.begin(), non_empty_it);
				// move non-empty to empty place
				iter_swap(empty_it, non_empty_it);
				replace(membership.begin(), membership.end(), non_empty_idx, empty_idx);
				// advance search
				empty_it++;
			}
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
		
	TPairIndex GetPairIndex(TState states, TState p, TState q) const
	{
		return states*p + q;
	}

	std::tuple<TState,TState> GetPairFromIndex(TState states, TPairIndex index) const
	{
		return make_tuple
		(
			static_cast<TState>(index / states), 
			static_cast<TState>(index % states)
		);
	}

	bool EquivP(TState p, TState q, const TDfa& dfa, const NumericPartition& part, BitSet<TPairIndex>& neq, BitSet<TPairIndex>& equiv, BitSet<TPairIndex>& path)
	{
		using namespace std;
		if(ShowConfiguration) {
			cout << "Test equiv (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ")" << endl;
		}
		TState states = dfa.GetStates();
		TPairIndex root_pair = GetPairIndex(states, p,q);
		if(neq.Contains(root_pair)) return false;
		if(path.Contains(root_pair)) return true;
		path.Add(root_pair);
		for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
		{
			TState sp = dfa.GetSuccessor(p,a);
			TState sq = dfa.GetSuccessor(q,a);
			if(part.Find(sp) == part.Find(sq)) continue;
			if(sp > sq) std::swap(sp, sq);
			TPairIndex pair = GetPairIndex(states, sp,sq);
			if(!equiv.Contains(pair))
			{
				equiv.Add(pair);
				if(!EquivP(sp,sq, dfa, part, neq, equiv, path))
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

	bool ShowConfiguration;

	MinimizationIncremental() : ShowConfiguration(false)
	{
	}

	void Minimize(const TDfa& dfa, NumericPartition& part)
	{
		using namespace std;
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
					neq.Add(GetPairIndex(states, p,q));
					if(ShowConfiguration) {
						cout << "neq add (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ") -> " << static_cast<size_t>(GetPairIndex(states, p,q)) << endl;
					}
				}
			}
		}
		cout << neq.to_string() << endl;
		BitSet<TPairIndex> equiv(states*states);
		BitSet<TPairIndex> path(states*states);
		for(TState p=0; p<states; p++)
		{		
			for(TState q=p+1; q<states; q++)
			{
				if(ShowConfiguration) {
					cout << "test (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ") -> " << static_cast<size_t>(GetPairIndex(states, p,q)) << endl;
				}
				if(neq.Contains(GetPairIndex(states, p,q))) continue;
				if(part.Find(p) == part.Find(q)) continue;
				equiv.Clear();
				path.Clear();
				
				const bool isEquiv = EquivP(p, q, dfa, part, neq, equiv, path);
				auto it = isEquiv ? equiv.GetIterator() : path.GetIterator();
				if(ShowConfiguration) {
					cout << (isEquiv ? "YES" : "NO") << endl;
				}
				for(; !it.IsEnd(); it.MoveNext())
				{
					TPairIndex idx = it.GetCurrent();
					TState p_prime, q_prime;
					tie(p_prime, q_prime) = GetPairFromIndex(states, idx);
					assert(p_prime < q_prime);
					if(isEquiv) 
					{
						if(ShowConfiguration) {
							cout << "union (" << static_cast<size_t>(p_prime) << ", " << static_cast<size_t>(q_prime) << ")" << endl;
						}
						part.Union(p_prime, q_prime);
					} else {
						if(ShowConfiguration) {
							cout << "neq (" << static_cast<size_t>(p_prime) << ", " << static_cast<size_t>(q_prime) << ")" << endl;
						}
						neq.Add(idx); // p_prime, q_prime
					}
				}
			}
		}
	}

	TDfa BuildDfa(const TDfa& dfa, NumericPartition& seq)
	{
		seq.Compact();
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
