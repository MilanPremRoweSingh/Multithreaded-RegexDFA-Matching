#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <stdio.h>
#include <iostream>
#include <omp.h>

using namespace std;

struct DFAState
{
	int id;
	unordered_map<char, DFAState*> transitionsOut;
};

void buildHardcodedDFA( vector<DFAState*>& outDFA ) //Builds DFA for ˆ(a+b+(c|d)+)+$
{
	outDFA.clear();

	DFAState* state0 = new DFAState();
	state0->id = 0;
	outDFA.push_back( state0 );

	DFAState* state1 = new DFAState();
	state1->id = 1;
	outDFA.push_back( state1 );

	DFAState* state2 = new DFAState();
	state2->id = 2;
	outDFA.push_back( state2 );

	DFAState* state3 = new DFAState();
	state3->id = 3;
	outDFA.push_back( state3 );

	state0->transitionsOut.insert( { 'a', state1 } );
	state0->transitionsOut.insert( { 'b', nullptr } );
	state0->transitionsOut.insert( { 'c', nullptr } );
	state0->transitionsOut.insert( { 'd', nullptr } );

	state1->transitionsOut.insert( { 'a', state1 } );
	state1->transitionsOut.insert( { 'b', state2 } );
	state1->transitionsOut.insert( { 'c', nullptr } );
	state1->transitionsOut.insert( { 'd', nullptr } );

	state2->transitionsOut.insert( { 'a', nullptr } );
	state2->transitionsOut.insert( { 'b', state2 } );
	state2->transitionsOut.insert( { 'c', state3 } );
	state2->transitionsOut.insert( { 'd', state3 } );

	state3->transitionsOut.insert( { 'a', state1 } );
	state3->transitionsOut.insert( { 'b', nullptr } );
	state3->transitionsOut.insert( { 'c', state3 } );
	state3->transitionsOut.insert( { 'd', state3 } );
}

int n;

int main( int argc, char* argv[] )
{
	if ( argc < 2 )
	{
		cout << "You must provide at least one argument";
		exit( 1 );
	}

	n = atoi( argv[1] );
	string strToMatch = "aaabbbcccddabccaabd"; //TEST

	vector<DFAState*> DFA;
	buildHardcodedDFA( DFA );

	int partLen		= strToMatch.length() / ( n + 1 );
	int partLenRem	= strToMatch.length() % ( n + 1 );
	vector<string> parts;
	int partPos = 0;
	for ( int i = 0; i < n; i++ )
	{
		int redistLen = 0; //Used to equally redistribute remained of partLen division
		if ( partLenRem > 0 )
		{
			redistLen = 1;
			partLenRem--;
		}

		parts.push_back( strToMatch.substr( partPos, partLen + redistLen ) );
		partPos += partLen + redistLen;
	}
	parts.push_back( strToMatch.substr( partPos, strToMatch.length() - n*partLen ) );

	bool mainIsMatch = true;
	DFAState* mainState = DFA[0];

	omp_set_dynamic( 0 );     // Allows us to enforce number of threads
	omp_set_num_threads( n+1 ); // Use n+1 threads

	vector<vector<DFAState*>> threadStates;
	threadStates.resize( n );
	for ( int i = 0; i < n; i++ )
	{
		threadStates.resize( DFA.size() - 1 ); // -1 because we dont consider the entry state for optimistic threads
		for ( int j = 1; j < DFA.size(); j++ )
		{
			threadStates[i][j - 1] = DFA[i];
		}
	}
//#pragma omp parallel for
	for ( int i = 0; i <= n; i++ )
	{
		if ( i == 0 ) //Normal Thread
		{
			string part = parts[i];
			for ( int strIdx = 0; strIdx < part.length(); strIdx++ )
			{
				try
				{
					mainState = mainState->transitionsOut.at( part.at( strIdx ) );
				}
				catch ( exception e )
				{
					mainState = nullptr;
				}
				if ( mainState == nullptr )
				{
					mainIsMatch = false;
					break;
				}
			}
		}
		else //Optimistic Threads
		{
			string part = parts[i];
			for ( int strIdx = 0; strIdx < part.length(); strIdx++ )
			{
				for ( int stateIdx = 0; stateIdx < threadStates[i].size(); stateIdx++ )
				{
					try
					{
						if( threadStates[i][stateIdx] != nullptr )
							threadStates[i][stateIdx] = threadStates[i][stateIdx]->transitionsOut.at( part.at( strIdx ) );
					}
					catch ( exception e )
					{
						threadStates[i][stateIdx] = nullptr;
					}
				}
			}
		}
	}

	system( "pause" ); //So we can see console output
}