#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <stdio.h>
#include <iostream>
#include <omp.h>
#include <time.h>

using namespace std;

struct DFAState
{
	int id;
	unordered_map<char, DFAState*> transitionsOut;
};

struct DFAStateFast
{
	int id;
	int transitionsOut[4];
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

void buildHardcodedDFA( DFAStateFast** outDFA, int& size ) //Builds DFA for ˆ(a+b+(c|d)+)+$
{
	//outDFA.clear();

	DFAStateFast* state0 = new DFAStateFast();
	state0->id = 0;
	//outDFA.push_back( state0 );
	outDFA[0] = ( state0 );

	DFAStateFast* state1 = new DFAStateFast();
	state1->id = 1;
	outDFA[1] = ( state1 );

	DFAStateFast* state2 = new DFAStateFast();
	state2->id = 2;
	outDFA[2] = ( state2 );

	DFAStateFast* state3 = new DFAStateFast();
	state3->id = 3;
	outDFA[3] = ( state3 );

	state0->transitionsOut[0] = 1;
	state0->transitionsOut[1] = -1;
	state0->transitionsOut[2] = -1;
	state0->transitionsOut[3] = -1;

	state1->transitionsOut[0] = 1;
	state1->transitionsOut[1] = 2;
	state1->transitionsOut[2] = -1;
	state1->transitionsOut[3] = -1;

	state2->transitionsOut[0] = -1;
	state2->transitionsOut[1] = 2;
	state2->transitionsOut[2] = 3;
	state2->transitionsOut[3] = 3;

	state3->transitionsOut[0] = 1;
	state3->transitionsOut[1] = -1;
	state3->transitionsOut[2] = 3;
	state3->transitionsOut[3] = 3;

	size = 4;
}

int n;

char *buildString( int size )
{
	int i;
	char *s = (char *)malloc( sizeof( char )*( size ) );
	if ( s == NULL )
	{
		printf( "\nOut of memory!\n" );
		exit( 1 );
	}
	int max = size - 3;

	/* seed the rnd generator (use a fixed number rather than the time for testing) */
	srand( (unsigned int)time( NULL ) );

	/* And build a long string that might actually match */
	int j = 0;
	while ( j<max )
	{
		s[j++] = 'a';
		while ( rand() % 1000<997 && j<max )
			s[j++] = 'a';
		if ( j<max )
			s[j++] = 'b';
		while ( rand() % 1000<997 && j<max )
			s[j++] = 'b';
		if ( j<max )
			s[j++] = ( rand() % 2 == 1 ) ? 'c' : 'd';
		while ( rand() % 1000<997 && j<max )
			s[j++] = ( rand() % 2 == 1 ) ? 'c' : 'd';
	}
	s[max] = 'a';
	s[max + 1] = 'b';
	s[max + 2] = ( rand() % 2 == 1 ) ? 'c' : 'd';
	s[max + 3] = '\0';
	return s;
}

int main( int argc, char* argv[] )
{
	if ( argc < 2 )
	{
		cout << "You must provide at least one argument";
		exit( 1 );
	}

	n = atoi( argv[1] );
	char* strcToMatch = buildString( 10000000 );
	string strToMatch( strcToMatch );

	DFAStateFast** DFA = (DFAStateFast**)malloc( sizeof(DFAStateFast*) * 4 );
	int DFASize;
	buildHardcodedDFA( DFA, DFASize );

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
	DFAStateFast* mainState = DFA[0];

	//omp_set_dynamic( 0 );     // Allows us to enforce number of threads
	omp_set_num_threads( n+1 ); // Use n+1 threads

	//vector<vector<DFAStateFast*>> threadStates;
	DFAStateFast*** threadStates = (DFAStateFast***)malloc( sizeof( DFAStateFast** ) * n );
	for ( int i = 0; i < n; i++ )
	{
		threadStates[i] = (DFAStateFast**)malloc( sizeof( DFAState* ) * (DFASize - 1)  );
		for ( int j = 1; j < 4; j++ )
		{
			threadStates[i][j - 1] = DFA[j];
		}
	}
	time_t start = clock();

#pragma omp parallel 
	//for ( int i = 0; i <= n; i++ )
	{
		int i = omp_get_thread_num();
		if ( i == 0 ) //Normal Thread
		{
			string part = parts[i];
			for ( int strIdx = 0; strIdx < part.length(); strIdx++ )
			{
				//mainState = mainState->transitionsOut.at( part.at( strIdx ) );
				if ( mainState == nullptr )
					break;

				int idx = mainState->transitionsOut[(int)( ( part.at( strIdx ) ) - 'a' )];
				if ( idx > -1 )
					mainState = DFA[idx];
				else
					mainState = nullptr;
			}
		}
		else //Optimistic Threads
		{
			string part = parts[i];
			for ( int strIdx = 0; strIdx < part.length(); strIdx++ )
			{
				for ( int stateIdx = 0; stateIdx < DFASize-1; stateIdx++ )
				{
					if ( threadStates[i - 1][stateIdx] != nullptr )
					{
						int idx = threadStates[i - 1][stateIdx]->transitionsOut[(int)( ( part.at( strIdx ) ) - 'a' )];
						if( idx > -1 )
							threadStates[i - 1][stateIdx] = DFA[ idx ];
						else 
							threadStates[i - 1][stateIdx] = nullptr;
						//threadStates[i-1][stateIdx] = threadStates[i-1][stateIdx]->transitionsOut.at( part.at( strIdx ) );
					}
				}
			}
		}
	}

	for ( int i = 0; i < n; i++ )
	{
		if ( mainState == nullptr )
		{	
			cout << "No match found" << endl;
			exit( 1 );
		}

		//mainState =  mainState->transitionsOut.at( parts[i+1].at( 0 ) );
		int idx = mainState->transitionsOut[(int)( ( parts[i + 1].at( 0 ) ) - 'a' )];
		if( idx > -1 )
			mainState = DFA[mainState->transitionsOut[(int)( ( parts[i + 1].at( 0 ) ) - 'a' )]];
		else
		{
			break;
		}
		if ( mainState->id == 0 )
		{
			mainState = nullptr;
			break;
		}

		mainState = threadStates[i][mainState->id - 1];
	}
	time_t end = clock();

	cout << difftime( end, start ) << std::endl;

	if ( mainState != nullptr )
	{
		cout << "The generated string matches with the given regex" << endl;
	}
	else
	{
		cout << "The generated string doesnt match with the given regex" << endl;
	}

	for ( int i = 0; i < n; i++ )
	{
		free(threadStates[i]);
	}
	free( threadStates );

	for ( int i = 0; i < DFASize; i++ )
		delete DFA[i];
	free( DFA );

	system( "pause" ); //So we can see console output
}