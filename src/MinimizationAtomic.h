// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Determinization.h"
#include "Dfa.h"
#include "Nfa.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>

/// Atomic FSA Minimization Algorithm
template<typename _TDfa = Dfa<typename _TFsa::TState, typename _TFsa::TSymbol>>
class MinimizationAtomic
{
public:	
	typedef _TDfa TDfa;
	typedef typename TDfa::TSet TSet;
	typedef typename TDfa::TState TState;
	typedef typename TDfa::TSymbol TSymbol;
	typedef typename TDfa::TState TAtomicState;

	typedef Nfa<TState, TSymbol> TNfa;

	typedef Determinization<TDfa, TNfa> TDeterminization;

	typedef std::unordered_set<TSet, typename TSet::hash> TSetOfSets;
	typedef typename TSetOfSets::iterator TSetOfSetsIterator;
	typedef std::tuple<TSetOfSetsIterator, TSymbol, TSetOfSetsIterator> TTransition;

	bool ShowConfiguration;

private:

public:

	template<typename T>
	std::string to_string(const std::list<T>& l) const
	{
		using namespace std;
		string str;
		str.append("{");
		int cont=0;
		for(auto i=l.begin(); i!=l.end(); i++)
		{
			if(cont++ > 0) str.append(", ");
			size_t st = static_cast<size_t>(*i);
			str.append(std::to_string(st));
		}
		str.append("}");
		return str;	
	}

	std::string to_string(const TSet& s) const
	{
		using namespace std;
		string str;
		str.append("{");
		int cont=0;
		for(auto i=s.GetIterator(); !i.IsEnd(); i.MoveNext())
		{
			if(cont++ > 0) str.append(", ");
			size_t st = static_cast<size_t>(i.GetCurrent());
			str.append(std::to_string(st));
		}
		str.append("}");
		return str;
	}

	void InverseReplicaOfInverse(const TDfa& fsa,
		TSetOfSets& QQ,		
		std::vector<TTransition>& transitions,
		std::vector<TSetOfSetsIterator>& II
		)
	{
		using namespace std;
		const TState INVALID_IDX = -1;

		vector<TAtomicState> invQQ;

		QQ.clear();

		vector<TSetOfSetsIterator> LL;

		invQQ.resize(fsa.GetStates());
		fill(invQQ.begin(), invQQ.end(), INVALID_IDX);

		QQ.insert(fsa.GetFinals());
		LL.push_back(QQ.begin());

		II.clear();

		// los estados de la delta directa
		TSet delta(fsa.GetStates());
		TSetOfSets PP, PmQ;

		while(!LL.empty())
		{
			const auto& P = *LL.begin();

			if(ShowConfiguration)
			{
				cout << "block: " << to_string(*P) << endl;
			}

			for(auto a=0; a<fsa.GetAlphabetLength(); a++)
			{
				// calcular delta(P,a)
				delta.Clear();
				for(auto itQ=P->GetIterator(); !itQ.IsEnd(); itQ.MoveNext())
				{
					auto q = itQ.GetCurrent();
					const auto& d = fsa.GetPredecessors(q, a);
					delta.UnionWith(d);
				}

				if(ShowConfiguration)
				{
					cout << "delta(P, a=" << static_cast<size_t>(a) << ") = " << to_string(delta) << endl;
				}

				auto rp = TSet::Intersect(delta, *P);
				auto rn = TSet::Difference(delta, rp);

				auto Sprime = QQ.insert(rp); // QQ U PP
				transitions.push_back(make_tuple(Sprime.first, a, P));

				// si insertó es porque es nueva, hay que procesarla luego
				if(Sprime.second) 
				{
					LL.push_back(Sprime.first);					
				}				
			}

			if(ShowConfiguration)
			{
				cout << "NewSplits = ";
				for(auto i=QQ.begin(); i!=QQ.end(); i++)
				{
					cout << to_string(*i) << " ";
				}
				cout << endl;
			}
		}
	}

	TDfa Minimize(const TDfa& fsa)
	{	
		using namespace std;

		TDeterminization det;
		TSetOfSets QQ;
		vector<TTransition> transitions;
		vector<TSetOfSetsIterator> II;

		InverseReplicaOfInverse(fsa, QQ, transitions, II);

		TNfa fsa_i(fsa.GetAlphabetLength(), QQ.size());
		fsa_i.SetFinal(0);
		for(auto i=II.begin(); i!=II.end(); i++)
		{
			//fsa_i.SetInitial();
		}

		for(auto i=transitions.begin(); i!=transitions.end(); i++)
		{
			//fsa_i.SetTransition(get<0>(*i), get<1>(*i), get<2>(*i));
		}

		TDeterminization::TDfaState dfaNewStates;
		TDeterminization::TVectorDfaState dfaFinalStates;
		TDeterminization::TVectorDfaEdge dfaEdges;
		det.Determinize(fsa_i, &dfaNewStates, dfaFinalStates, dfaEdges);
		return det.BuildDfa(fsa_i.GetAlphabetLength(), dfaNewStates, dfaFinalStates, dfaEdges);
	}

};