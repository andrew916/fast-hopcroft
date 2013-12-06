// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Determinization.h"
#include "Dfa.h"
#include "Nfa.h"
#include <unordered_set>
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
		std::unordered_set<TSet>& QQ,
		std::unordered_set<TSet>& LL,
		std::vector<TAtomicState>& invQQ,
		std::vector<std::tuple<TAtomicState,TSymbol,TAtomicState>>& transitions, 
		std::vector<TAtomicState>& II		
		)
	{
		using namespace std;
		const TState INVALID_IDX = -1;

		QQ.clear();
		LL.clear();

		invQQ.resize(fsa.GetStates());
		fill(invQQ.begin(), invQQ.end(), INVALID_IDX);

		QQ.insert(fsa.GetFinals());
		LL.insert(fsa.GetFinals());
		
		II.clear();
				
		// los estados de la delta directa
		TSet delta(fsa.GetStates());		
		unordered_set<TSet> PP, PmQ, newQQ;

		while(!LL.empty())
		{
			const auto& P = *LL.begin();
			LL.erase(LL.begin());

			if(ShowConfiguration)
			{
				cout << "block: " << static_cast<size_t>(currentBlock) << " " << to_string(P) << endl;
			}

			for(auto a=0; a<fsa.GetAlphabetLength(); a++)
			{
				// calcular delta(P,a)
				delta.Clear();
				for(auto q=P.begin(); q!=P.end(); q++)
				{
					auto d = fsa.GetPredecessors(*q, a);
					delta.UnionWith(d);
				}

				if(ShowConfiguration)
				{
					cout << "delta(P, a=" << static_cast<size_t>(a) << ") = " << to_string(delta) << endl;
				}

				// obtiene los splitters de la delta directa
				// loose_block usa cero porque nunca el bloque cero es usable (siempre tiene al menos los etados finales)
				TAtomicState looseBlock = 0;
				TAtomicState insertionPoint = QQ.size();
				PP.Clear();
				for(auto i=delta.GetIterator(); !i.IsEnd(); i.MoveNext())
				{
					auto q = i.GetCurrent();

					// asegura un bloque para el estado q					
					auto sp = invQQ[q];
					
					if(sp == INVALID_IDX) continue;

					auto rp = TSet::Intersect(delta, P);
					auto rn = TSet::Difference(delta, rp);
					PP.Add(rp);
					PP.Add(rn);
				}
				uordered_set_difference(PP.begin(), PP.end(), QQ.begin(), QQ.end(), PmQ.begin());
				for(auto i=PP.begin(); i!=PP.end(); i++) 
				{
					auto Spime = QQ.insert(*i); // QQ U PP
					transitions.push_back(make_tuple(Sprime->first, a, itP));
				}
								
				if(ShowConfiguration)
				{
					cout << "NewSplits = ";
					for(auto i=insertionPoint; i<QQ.size(); i++)
					{
						cout << "[" << static_cast<size_t>(i) << "]" << to_string(QQ[i]) << " ";
					}
					cout << endl;
				}
			}
		}
	}

	TDfa Minimize(const TDfa& fsa)
	{	
		using namespace std;

		TDeterminization det;
		vector<list<TState>> QQ;
		vector<TAtomicState> invQQ;
		vector<tuple<TAtomicState,TSymbol,TAtomicState>> transitions;
		vector<TAtomicState> II;
		
		InverseReplicaOfInverse(fsa, QQ, invQQ, transitions, II);

		TNfa fsa_i(fsa.GetAlphabetLength(), QQ.size());
		fsa_i.SetFinal(0);
		for(auto i=II.begin(); i!=II.end(); i++)
		{
			fsa_i.SetInitial(*i);
		}

		for(auto i=transitions.begin(); i!=transitions.end(); i++)
		{
			fsa_i.SetTransition(get<0>(*i), get<1>(*i), get<2>(*i));
		}

		TDeterminization::TDfaState dfaNewStates;
		TDeterminization::TVectorDfaState dfaFinalStates;
		TDeterminization::TVectorDfaEdge dfaEdges;
		det.Determinize(fsa_i, &dfaNewStates, dfaFinalStates, dfaEdges);
		return det.BuildDfa(fsa_i.GetAlphabetLength(), dfaNewStates, dfaFinalStates, dfaEdges);
	}

};