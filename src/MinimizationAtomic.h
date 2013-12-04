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
	typedef TFsa::TSet TSet;
	typedef typename TFsa::TState TState;
	typedef typename TFsa::TSymbol TSymbol;

	typedef Determinization<TFsa, TFsa> TDeterminization;

private:

public:

	void Replica(const TFsa& fsa)
	{
		using namespace std;
		const TState INVALID_IDX = -1;

		vector<list<TState>> QQ(1);
		vector<TState> invQQ(fsa.GetStates(), INVALID_IDX);

		for(auto i=fsa.GetFinals().GetIterator(); !i.IsEnd(); i.MoveNext())
		{
			TState q = i.GetCurrent();
			QQ[0].push_back(q);
			invQQ[q] = 0;
		}

		// esta garantizado que el atomico es de menos o igual numero de estados?
		vector<tuple<TState,list<TState>>> splitters(fsa.GetStates());
		TSet delta(fsa.GetStates());
		for(TState currentPartition = 0; currentPartition < QQ.size(); currentPartition++)
		{
			auto P = QQ[currentPartition];
			
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
				fill(splitters.begin(), splitters.end(), make_tuple(0, list<TState>()));
				for(auto i=delta.GetIterator(); !i.IsEnd(); i.MoveNext())
				{
					auto q = i.GetCurrent();
					auto sp = invQQ[q];
					if(sp != INVALID_IDX) 
					{
						auto& splitter = splitters[sp];
						if(get<0>(splitter) == 0) 
						{
							get<0>(splitter) = QQ[sp].size() - 1;
						} else {
							get<0>(splitter)--;
						}
						get<1>(splitter).push_back(q);
					}
				}
			}
		}
	}

	void Minimize(const TFsa& fsa)
	{	
		TDeterminization det;
		Replica(fsa);
	}

};