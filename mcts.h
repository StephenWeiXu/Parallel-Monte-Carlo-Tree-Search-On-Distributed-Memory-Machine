#ifndef MCTS_HEADER
#define MCTS_HEADER

#include <algorithm>
#include <cstdlib>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <cassert>

using namespace std;


/************************************************************************/
// Monte Carlo Tree Search for finite games.
//
// Based on Python code at
// http://mcts.ai/code/python.html
//
// Uses the "root parallelization" technique [1].
//
// This game engine can play any game defined by a state like this:
/*

class GameState
{
public:
	typedef int Move;
	static const Move no_move = ...

	void do_move(Move move);
	template<typename RandomEngine>
	void do_random_move(*engine);
	bool has_moves() const;
	vector<Move> get_moves() const;

	// Returns a value in {0, 0.5, 1}.
	// This should not be an evaluation function, because it will only be
	// called for finished games. Return 0.5 to indicate a draw.
	double get_result(int current_player_to_move) const;

	int player_to_move;

	// ...
private:
	// ...
};

*/
//
// See the examples for more details. Given a suitable State, the
// following function (tries to) compute the best move for the
// player to move.
//
/************************************************************************/

struct ComputeOptions
{
	//int number_of_threads;
	int max_iterations;
	double max_time;
	bool verbose;

	ComputeOptions(){
		//number_of_threads = 8;
		max_iterations = 10000;
		max_time = -1.0; // default is no time limit.
		verbose = false;
	}
};

template<typename State>
typename State::Move compute_move(const State root_state,
                                  const ComputeOptions options = ComputeOptions());


//
// This class is used to build the game tree. The root is created by the users and
// the rest of the tree is created by add_node.
//
/************************************************************************/
/* Function Description
	1. has_untried_moves
	2. get_untried_moves
	3. best_child
	4. has_children
	5. select_child_UCT
	6. add_child
	7. update
*/
/************************************************************************/


template<typename State>
class Node
{
public:
	typedef typename State::Move Move;

	// Define class variables
	const Move move;
	Node* const parent;
	const int player_to_move;
	
	double wins;
	int visits;

	vector<Move> moves;
	vector<Node*> children;

	Node(const State& state);
	~Node();

	bool has_untried_moves() const;
	template<typename RandomEngine>
	Move get_untried_move(RandomEngine* engine) const;
	Node* best_child() const;

	bool has_children() const
	{
		return ! children.empty();
	}

	Node* select_child_UCT() const;
	Node* add_child(const Move& move, const State& state);
	void update(double result);

	string to_string() const;
	string tree_to_string(int max_depth = 1000000, int indent = 0) const;

private:
	Node(const State& state, const Move& move, Node* parent);

	string indent_string(int indent) const;

	Node(const Node&);
	Node& operator = (const Node&);

	double UCT_score;
};


// Node class constructors
template<typename State>
Node<State>::Node(const State& state) :
	move({-1, -1}),
	parent(nullptr),
	player_to_move(state.player_to_move),
	wins(0),
	visits(0),
	moves(state.get_moves()),
	UCT_score(0)
{ }

template<typename State>
Node<State>::Node(const State& state, const Move& move_, Node* parent_) :
	move(move_),
	parent(parent_),
	player_to_move(state.player_to_move),
	wins(0),
	visits(0),
	moves(state.get_moves()),
	UCT_score(0)
{ }

template<typename State>
Node<State>::~Node()
{
	for (auto child: children) {
		delete child;
	}
}

#endif
