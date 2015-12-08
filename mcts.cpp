#include <iostream>
using namespace std;

#include "mcts.h"


/************************************************************************/
template<typename State>
double Node<State>::cacul_UCT_score(double child_wins, int child_visits, int parent_visits){
	double uct_score = double(child_wins) / double(child_visits) +
			sqrt(2.0 * log(double(parent_visits + 1)) / child_visits);	
	return uct_score;
}

template<typename State>
bool Node<State>::has_untried_moves()
{
	return ! moves.empty();
}

template<typename State>
template<typename RandomEngine>
typename State::Move Node<State>::get_untried_move(RandomEngine* engine)
{
	assert( ! moves.empty());
	uniform_int_distribution<size_t> moves_distribution(0, moves.size() - 1);
	return moves[moves_distribution(*engine)];
}

template<typename State>
Node<State>* Node<State>::best_child()
{
	assert(moves.empty());
	assert( ! children.empty() );

	return *max_element(children.begin(), children.end(),
		[](Node* a, Node* b) { return a->visits < b->visits; });;
}

template<typename State>
Node<State>* Node<State>::select_child_UCT(df_stack_UCT_info *stack_UCT_info)
{
	assert( ! children.empty() );
	for (auto child: children) {
		child->UCT_score = cacul_UCT_score(child->wins, child->visits, this->visits);
	}

	sort(children.begin(), children.end(), [](Node* a, Node* b) { return a->UCT_score < b->UCT_score; });
	
	/* initialize the stack_UCT_info */
	stack_UCT_info->best_move_visits = children[0]->visits;
	stack_UCT_info->best_move_wins = children[0]->wins;
	stack_UCT_info->second_best_move_visits = children[1]->visits;
	stack_UCT_info->second_best_move_wins = children[1]->wins;
	stack_UCT_info->parent_visits = visits;

	second_best_child = children[1];

	return children[0]; 
	// return *max_element(children.begin(), children.end(),
	// 	[](Node* a, Node* b) { return a->UCT_score < b->UCT_score; });
}

template<typename State>
Node<State>* Node<State>::add_child(const Move& move, const State& state)
{
	auto node = new Node(state, move, this);
	children.push_back(node);
	assert( ! children.empty());

	auto itr = moves.begin();
	for (; itr != moves.end() && *itr != move; ++itr);
	assert(itr != moves.end());
	moves.erase(itr);
	return node;
}

template<typename State>
void Node<State>::update(double result)
{
	visits++;
	wins += result;

	stack_UCT_info->best_move_visits++;
	stack_UCT_info->best_move_wins += result;  // Hopefully this will also change the values in the stack
	//double my_wins = wins.load();
	//while ( ! wins.compare_exchange_strong(my_wins, my_wins + result));
}

template<typename State>
string Node<State>::to_string()
{
	stringstream sout;
	sout << "["
	     << "P" << 3 - player_to_move << " "
	     << "M:" << move << " "
	     << "W/V: " << wins << "/" << visits << " "
	     << "U: " << moves.size() << "]\n";
	return sout.str();
}

template<typename State>
string Node<State>::tree_to_string(int max_depth, int indent)
{
	if (indent >= max_depth) {
		return "";
	}

	string s = indent_string(indent) + to_string();
	for (auto child: children) {
		s += child->tree_to_string(max_depth, indent + 1);
	}
	return s;
}

template<typename State>
string Node<State>::indent_string(int indent)
{
	string s = "";
	for (int i = 1; i <= indent; ++i) {
		s += "| ";
	}
	return s;
}

