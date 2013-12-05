// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Determinization.h"
#include "Dfa.h"
#include "Nfa.h"
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

	void InverseReplicaOfInverse(const TDfa& fsa, 		
		std::vector<std::list<TState>>& QQ, 
		std::vector<TAtomicState>& invQQ,
		std::vector<std::tuple<TAtomicState,TSymbol,TAtomicState>>& transitions, 
		std::vector<TAtomicState>& II		
		)
	{
		using namespace std;
		const TState INVALID_IDX = -1;

		QQ.resize(1);
		QQ.reserve(fsa.GetStates()*2 + 1);

		invQQ.resize(fsa.GetStates());
		fill(invQQ.begin(), invQQ.end(), INVALID_IDX);

		QQ[0].clear();
		for(auto i=fsa.GetFinals().GetIterator(); !i.IsEnd(); i.MoveNext())
		{
			TState q = i.GetCurrent();
			QQ[0].push_back(q);
			invQQ[q] = 0;
		}
		
		II.clear();

		// esta garantizado que el atomico tiene cuando mucho el doble de estados
		TSet already(fsa.GetStates() * 2);		

		// los estados de la delta directa
		TSet delta(fsa.GetStates());

		for(TAtomicState currentBlock = 0; currentBlock < QQ.size(); currentBlock++)
		{
			auto& P = QQ[currentBlock];

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
					cout << "delta(P, a=" << static_cast<size_t>(a) << ") = " << to_string(P) << endl;
				}

				// obtiene los splitters de la delta directa
				// loose_block usa cero porque nunca el bloque cero es usable (siempre tiene al menos los etados finales)
				TAtomicState looseBlock = 0;
				TAtomicState insertionPoint = QQ.size();
				already.Clear();
				for(auto i=delta.GetIterator(); !i.IsEnd(); i.MoveNext())
				{
					auto q = i.GetCurrent();

					// asegura un bloque para el estado q					
					auto sp = invQQ[q];
					TAtomicState dest;
					if(sp == INVALID_IDX)
					{
						if(looseBlock == 0) 
						{ 
							looseBlock = QQ.size();
							QQ.emplace_back(); 							
						}
						QQ[looseBlock].push_back(q);
						invQQ[q] = looseBlock;
						dest = looseBlock;

						if(fsa.IsInitial(q)) II.push_back(dest);
					}
					else 
					{
						// Ya antes habiamos usado el bloque Q
						if(already.Contains(sp)) continue;

						// Usamos el bloque Q para intenar partir delta
						const auto& Q = QQ[sp];

						list<TState> Qp;

						// identifica si el bloque efectivamente particionara o 
						// esta repetido en Q
						bool isRepeat = true;
						bool initial = false;
						for(auto j=Q.begin(); j!=Q.end(); j++)
						{
							if(!delta.TestAndRemove(*j)) 
							{
								isRepeat = false;								
							} else {
								Qp.push_back(*j);
								initial = initial || fsa.IsInitial(*j);
							}
						}

						already.Add(sp);

						// si ya estaba repetido seguimos adelante
						if(isRepeat) continue;

						dest = QQ.size();
						QQ.emplace_back();
						auto& Qdest = QQ[dest];
						Qdest.splice(Qdest.begin(), Qp);
						for(auto i=Qdest.begin(); i!=Qdest.end(); i++)
							invQQ[*i] = dest;

						if(initial) II.push_back(dest);
					}					
					transitions.push_back(make_tuple(dest, a, currentBlock));
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