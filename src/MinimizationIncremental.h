// Septiembre 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <algorithm>
#include "Set.h"

// Incremental Almeida et al. Minimization Algorithm.
template<typename _TDfa>
class MinimizationIncremental
{
public:
	typedef _TDfa TDfa;
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
	
	TPairIndex GetPairIndex(TState p, TState q) const
	{
		assert(p < q);
		return (q*q-q)/2+p;
	}

	std::tuple<TState,TState> GetPairFromIndex(TPairIndex index) const
	{
		using namespace std;
		TState q = static_cast<TState>(sqrt((1 + 8*index)/4.0f) + 0.5f);
		TState p = static_cast<TState>(index - (q*q-q)/2);
		assert(p < q);
		assert(index == GetPairIndex(p,q));

		return make_tuple(p, q);
	}

	bool EquivP(TState p, TState q, const TDfa& dfa, const NumericPartition& part, BitSet<TPairIndex>& neq, BitSet<TPairIndex>& equiv, BitSet<TPairIndex>& path)
	{
		using namespace std;
		if(ShowConfiguration) {
			cout << "Test equiv (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ")" << endl;
		}
				
		if(dfa.IsFinal(p) != dfa.IsFinal(q)) return false;
		TPairIndex root_pair = GetPairIndex(p,q);
		if(neq.Contains(root_pair)) return false;
		if(path.TestAndAdd(root_pair)) return true;
		for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
		{
			TState sp = dfa.GetSuccessor(p, a);
			TState sq = dfa.GetSuccessor(q, a);
			if(sp == sq) continue;
			//if(part.Find(sp) == part.Find(sq)) continue;
			if(sp > sq) std::swap(sp, sq);
			TPairIndex pair = GetPairIndex(sp, sq);
			if(!equiv.TestAndAdd(pair))
			{
				if(ShowConfiguration) cout << "with symbol: " << static_cast<size_t>(a) << endl;
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

	
	std::string to_string(const std::list<TState>& ls)
	{
		using namespace std;
		string str;
		str.append("{");
		int cont=0;
		for(auto i=ls.begin(); i!=ls.end(); i++)
		{
			if(cont++ > 0) str.append(", ");
			size_t k = static_cast<size_t>(*i);
			str.append(std::to_string(k));
		}
		str.append("}");
		return str;
	}

	std::string to_string(const NumericPartition& p)
	{
		using namespace std;
		string str;
		str.append("{");
		TState i=0;
		for(auto j=p.GetStore().begin(); j!=p.GetStore().end(); j++)
		{
			if(j->size() == 0) continue;
			if(i > 0) str.append(", ");			
			str.append(to_string(*j));
			i++;
		}
		str.append("}");
		return str;
	}

	bool ShowConfiguration;

	MinimizationIncremental() : ShowConfiguration(false)
	{
	}

	void Minimize(const TDfa& dfa, NumericPartition& part)
	{
		using namespace std;
		TState states = dfa.GetStates();
		part.Clear(states);

		BitSet<TPairIndex> neq((states*states-states)/2);		
		BitSet<TPairIndex> equiv((states*states-states)/2);
		BitSet<TPairIndex> path((states*states-states)/2);
		for(TState p=0; p<states; p++)
		{
			for(TState q=p+1; q<states; q++)
			{
				if(ShowConfiguration) 
				{
					cout << "test (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ")" << endl;
				}
				if(dfa.IsFinal(p) != dfa.IsFinal(q)) continue;
				if(neq.Contains(GetPairIndex(p,q))) continue;
				if(part.Find(p) == part.Find(q)) continue;
				equiv.Clear();
				path.Clear();

				const bool isEquiv = EquivP(p, q, dfa, part, neq, equiv, path);
				if(ShowConfiguration) 
				{
					cout << (isEquiv ? "YES" : "NO") << endl;
				}
				if(isEquiv) for(auto it=equiv.GetIterator(); !it.IsEnd(); it.MoveNext())
				{
					TPairIndex idx = it.GetCurrent();
					TState p_prime, q_prime;
					tie(p_prime, q_prime) = GetPairFromIndex(idx);
					assert(p_prime < q_prime);
					if(ShowConfiguration) {
						cout << "union (" << static_cast<size_t>(p_prime) << ", " << static_cast<size_t>(q_prime) << ")" << endl;
					}
					part.Union(p_prime, q_prime);
				}
				else neq.UnionWith(path);
			}
		}
		if(ShowConfiguration)
		{
			cout << "Final P=" << to_string(part) << endl;
			cout << "Finished " << part.GetSize() << " states of " << dfa.GetStates() << endl;
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
		TDfa dfa_min = BuildDfa(dfa, part);
		return dfa_min;
	}
};
