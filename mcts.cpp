#include <iostream>
using namespace std;

#include "mcts.h"


/************************************************************************/
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
		child->UCT_score = double(child->wins) / double(child->visits) +
			sqrt(2.0 * log(double(this->visits + 1)) / child->visits);
	}

	sort(children.begin(), children.end(), [](Node* a, Node* b) { return a->UCT_score < b->UCT_score; });
	
	/* initialize the stack_UCT_info */
	stack_UCT_info->best_move_visits = children[0]->visits;
	stack_UCT_info->best_move_wins = children[0]->wins;
	stack_UCT_info->second_best_move_visits = children[1]->visits;
	stack_UCT_info->second_best_move_wins = children[1]->wins;
	stack_UCT_info->parent_visits = visits;

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
void Node<State>::update(double result, stack<df_stack_UCT_info*>& df_UCT_stack)
{
	visits++;

	wins += result;
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

/************************************************************************/
void ComputeOptions::check_local_UCT_stack(stack<df_stack_UCT_info*>&){

}


template<typename State>
unique_ptr<Node<State>> ComputeOptions::compute_tree(const State root_state,
                                           const ComputeOptions options,
                                           mt19937_64::result_type initial_seed)
{
	mt19937_64 random_engine(initial_seed);

	assert(options.max_iterations >= 0 || options.max_time >= 0);
	// if (options.max_time >= 0) {
	// 	#ifndef USE_OPENMP
	// 	throw runtime_error("ComputeOptions::max_time requires OpenMP.");
	// 	#endif
	// }
	// Will support more players later.
	assert(root_state.player_to_move == 1 || root_state.player_to_move == 2);
	auto root = unique_ptr<Node<State>>(new Node<State>(root_state));

	// #ifdef USE_OPENMP
	// double start_time = ::omp_get_wtime();
	// double print_time = start_time;
	// #endif
	
	stack<df_stack_UCT_info*> df_UCT_stack;

	// Each iteration will again begin at the root node, which is inefficient!
	for (int iter = 1; iter <= options.max_iterations || options.max_iterations < 0; ++iter) {
		auto node = root.get(); // node->children.size() will increase by 1 for each iteration. the max size will be determined by state.has_moves()
		State state = root_state;

		/* Select */
		// Select a path through the tree to a leaf node.
		// While loop will not be entered until this node has added all its untried moves (added all its possible children)
		while (!node->has_untried_moves() && node->has_children()) {
			df_stack_UCT_info *stack_UCT_info = new df_stack_UCT_info(); 
			node = node->select_child_UCT(stack_UCT_info);
			df_UCT_stack.push(stack_UCT_info);
			state.do_move(node->move);
		}

		/* Expand */
		// If we are not already at the final state, expand the
		// tree with a new node and move there.
		if (node->has_untried_moves()) {
			auto move = node->get_untried_move(&random_engine);
			state.do_move(move);
			node = node->add_child(move, state); // The returnt node is the added child node, not the parent node, its children will be 0
		}

		/* Playout */
		// We now play randomly until the game ends.
		while (state.has_moves()) {
			state.do_random_move(&random_engine);
		}

		/* Backpropagate */
		// We have now reached a final state. Backpropagate the result
		// up the tree to the root node.
		while (node != nullptr) {
			node->update(state.get_result(node->player_to_move), df_UCT_stack);
			/* Check whether the situation is still satisfied or not */
			node = node->parent;
		}

		/* When update, also update the best move visits&wins in the stack_UCT_info */


		// #ifdef USE_OPENMP
		// if (options.verbose || options.max_time >= 0) {
		// 	double time = ::omp_get_wtime();
		// 	if (options.verbose && (time - print_time >= 1.0 || iter == options.max_iterations)) {
		// 		cerr << iter << " games played (" << double(iter) / (time - start_time) << " / second)." << endl;
		// 		print_time = time;
		// 	}

		// 	if (time - start_time >= options.max_time) {
		// 		break;
		// 	}
		// }
		// #endif
	}

	return root; // root->children.size() will be the number of moves, num_rows*num_cols is the first iteration
}

template<typename State>
typename State::Move ComputeOptions::compute_move(const State root_state,
                                  const ComputeOptions options)
{
	/* Will support more players later. */
	assert(root_state.player_to_move == 1 || root_state.player_to_move == 2);

	auto moves = root_state.get_moves();
	assert(moves.size() > 0);
	if (moves.size() == 1) {
		return moves[0];
	}

	// #ifdef USE_OPENMP
	// double start_time = ::omp_get_wtime();
	// #endif

	/* Start all jobs to compute trees. */
	//future<unique_ptr<Node<State>>> root_futures;
	ComputeOptions job_options = options;
	job_options.verbose = false;
	//for (int t = 0; t < options.number_of_threads; ++t) {
	// auto func = [&root_state, &job_options] () -> unique_ptr<Node<State>>
	// {
	// 	return compute_tree(root_state, job_options, 1012411 * 0 + 12515);
	// };

	// root_futures = async(launch::async, func);
	//}

	/* Collect the results. */
	unique_ptr<Node<State>> root = compute_tree(root_state, job_options, 1012411 * 0 + 12515);
	//for (int t = 0; t < options.number_of_threads; ++t) {
	//roots = move(root_futures.get()); // move() function on unique_ptr
	//}

	/* Merge the children of all root nodes. */
	map<typename State::Move, int> visits;
	map<typename State::Move, double> wins;
	long long games_played = 0;
	//for (int t = 0; t < options.number_of_threads; ++t) {
	//auto root = roots.get();
	games_played += root->visits;
	for (auto child = root->children.cbegin(); child != root->children.cend(); ++child) {
		visits[(*child)->move] += (*child)->visits;
		wins[(*child)->move]   += (*child)->wins;
	}
	//}

	/* Find the node with the most visits. */
	int best_visits = -1;
	typename State::Move best_move = typename State::Move();
	for (auto itr: visits) {
		if (itr.second > best_visits) {
			best_visits = itr.second;
			best_move = itr.first;
		}

		if (options.verbose) {
			cerr << "Move: " << (itr.first)[0] << ", " << (itr.first)[1]
			     << " (" << setw(2) << right << int(100.0 * itr.second / double(games_played) + 0.5) << "% visits)"
			     << " (" << setw(2) << right << int(100.0 * wins[itr.first] / best_visits  + 0.5)    << "% wins)" << endl;
		}
	}

	if (options.verbose) {
		double best_wins = wins[best_move];
		cerr << "----" << endl;
		cerr << "Best: " << best_move[0] << ", " << best_move[1]
		     << " (" << 100.0 * best_visits / double(games_played) << "% visits)"
		     << " (" << 100.0 * best_wins / best_visits << "% wins)" << endl;
	}

	return best_move;
}
