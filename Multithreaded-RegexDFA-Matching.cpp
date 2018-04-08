#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <string>

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
	state0->id = 1;
	outDFA.push_back( state1 );

	DFAState* state2 = new DFAState();
	state0->id = 2;
	outDFA.push_back( state2 );

	DFAState* state3 = new DFAState();
	state0->id = 3;
	outDFA.push_back( state3 );

	DFAState* state4 = new DFAState();
	state0->id = 4;
	outDFA.push_back( state4 );

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

	state2->transitionsOut.insert( { 'a', state1 } );
	state2->transitionsOut.insert( { 'b', nullptr } );
	state2->transitionsOut.insert( { 'c', state3 } );
	state2->transitionsOut.insert( { 'd', state3 } );
}



int main( char* argv, int argc)
{
	string strToMatch = "aaabbbcccddabcaabd"; //TEST

	vector<DFAState*> DFA;
	buildHardcodedDFA( DFA );

	system( "pause" ); //So we can see console output
}