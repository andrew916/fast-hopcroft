// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Determinization.h"
#include "Dfa.h"
#include <vector>
#include <tuple>

/// Atomic FSA Minimization Algorithm
template<typename _TFsa, typename _TDfa = Dfa<typename _TFsa::TState, typename _TFsa::TSymbol>>
class MinimizationAtomic
{
public:
	typedef _TFsa TFsa;
	typedef _TDfa TDfa;
	typedef typename TFsa::TSet TSet;
	typedef typename TFsa::TState TState;
	typedef typename TFsa::TSymbol TSymbol;
	typedef typename TFsa::TState TAtomicState;

	typedef Determinization<TFsa, TFsa> TDeterminization;

private:

public:

	void InverseReplicaOfInverse(const TFsa& fsa, 		
		std::vector<std::list<TState>>& QQ, 
		std::vector<TAtomicState>& invQQ,
		std::vector<std::tuple<TAtomicState,TSymbol,TAtomicState>>& transitions, 
		std::vector<TAtomicState>& II, 
		std::vector<TState>& splits,
		TAtomicState& blocks
		)
	{
		using namespace std;
		const TState INVALID_IDX = -1;

		invQQ.resize(fsa.GetStates());
		QQ.resize(fsa.GetStates() * 2);
		fill(invQQ.begin(), invQQ.end(), INVALID_IDX);

		II.clear();

		for(auto i=fsa.GetFinals().GetIterator(); !i.IsEnd(); i.MoveNext())
		{
			TState q = i.GetCurrent();
			QQ[0].push_back(q);
			invQQ[q] = 0;
		}

		// empieza al menos con el bloque que contiene los finales
		blocks = 1;

		// esta garantizado que el atomico tiene cuando mucho el doble de estados
		TSet already(fsa.GetStates() * 2);		

		// los estados de la delta directa
		TSet delta(fsa.GetStates());

		for(TAtomicState currentBlock = 0; currentBlock < blocks; currentBlock++)
		{
			auto& P = QQ[currentBlock];
			// si de un split restan cero es porque es un bloque repetido
			if(splits[currentBlock] == 0) continue;

			for(auto a=0; a<fsa.GetAlphabetLength(); a++)
			{
				// calcular delta(P,a)
				delta.Clear();
				for(auto q=P.begin(); q!=P.end(); q++)
				{
					auto d = fsa.GetPredecessors(*q, a);
					delta.UnionWith(d);
				}

				// obtiene los splitters de la delta directa
				// loose_block usa cero porque nunca el bloque cero es usable (siempre tiene al menos los etados finales)
				TAtomicState loose_block = 0; 
				already.Clear();
				for(auto i=delta.GetIterator(); !i.IsEnd(); i.MoveNext())
				{
					auto q = i.GetCurrent();
										
					// asegura un bloque para el estado q					
					auto sp = invQQ[q];
					TAtomicState dest;
					if(sp == INVALID_IDX)
					{
						if(loose_block == 0) loose_block = blocks++;
						QQ[loose_block].push_back(q);
						invQQ[q] = loose_block;
						dest = loose_block;

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
								initial ||= fsa.IsInitial(*j);
							}
						}

						already.Add(sp);
						
						// si ya estaba repetido seguimos adelante
						if(isRepeat) continue;
						
						dest = blocks++;
						auto& Qdest = QQ[dest];
						Qdest.splice(Qdest.begin(), Qp);
						for(auto i=Qdest.begin(); i!=Qdest.end(); i++)
							invQQ[*i] = dest;

						if(initial) II.push_back(dest);
					}					
					transitions.push_back(make_tuple(dest, a, currentBlock));
				}
			}
		}
	}

	TDfa Minimize(const TFsa& fsa)
	{	
		using namespace std;

		TDeterminization det;
		vector<list<TState>> QQ;
		vector<TAtomicState> invQQ;
		vector<tuple<TAtomicState,TSymbol,TAtomicState>> transitions;
		vector<TAtomicState> II;
		vector<TState> splits;
		TAtomicState blocks;

		InverseReplicaOfInverse(fsa, QQ, invQQ, transitions, II, splits, blocks);

		TFsa fsa_i(fsa.GetAlphabetLength(), QQ.size());
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