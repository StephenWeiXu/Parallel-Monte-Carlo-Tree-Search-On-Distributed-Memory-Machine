#ifndef MCTS_HELPER_HEADER
#define MCTS_HELPER_HEADER

#include "mcts.h"

/************************************************************************/
double cacul_UCT_score(double child_wins, int child_visits, int parent_visits){
	double uct_score = double(child_wins) / double(child_visits) +
			sqrt(2.0 * log(double(parent_visits + 1)) / child_visits);	
	return uct_score;
}

int check_local_UCT_stack(stack<df_stack_UCT_info*> UCT_stack, bool& need_backtrack){
	int count = 0;
	double uct_score_1, uct_score_2;
	while(!UCT_stack.empty()){
		df_stack_UCT_info* temp_UCT_info = UCT_stack.top();
		UCT_stack.pop();

		uct_score_1 = cacul_UCT_score(temp_UCT_info->best_move_wins, temp_UCT_info->best_move_visits, temp_UCT_info->parent_visits);
		uct_score_2 = cacul_UCT_score(temp_UCT_info->second_best_move_wins, temp_UCT_info->second_best_move_visits, temp_UCT_info->parent_visits);

		if (uct_score_1 < uct_score_2){
			need_backtrack = true;
			return count;  
		}else{
			count++;
		}
	}
	return count;
}


template<typename State>
unique_ptr<Node<State>> compute_tree(const State root_state,
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

	auto node = root.get();
	State state = root_state;
	// Each iteration will again begin at the root node, which is inefficient!
	for (int iter = 1; iter <= options.max_iterations || options.max_iterations < 0; ++iter) {
		// auto node = root.get(); // node->children.size() will increase by 1 for each iteration. the max size will be determined by state.has_moves()
		// State state = root_state;

		/* Select */
		// Select a path through the tree to a leaf node.
		// While loop will not be entered until this node has added all its untried moves (added all its possible children)
		while (!node->has_untried_moves() && node->has_children()) {
			auto temp_best_child = node->select_child_UCT(node->stack_UCT_info);
			df_UCT_stack.push(node->stack_UCT_info);
			state.do_move(node->move);
			node = temp_best_child;
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
		node->update(state.get_result(node->player_to_move));
		node = node->parent;
		bool need_backtrack_flag = false;
		int backtrack_pos = df_UCT_stack.size() - check_local_UCT_stack(df_UCT_stack, need_backtrack_flag);
		if(need_backtrack_flag){
			while(backtrack_pos > 0){
				node->update(state.get_result(node->player_to_move));
				node = node->parent;
				backtrack_pos--;
			}
		}

		node = node->second_best_child;

		// while (node != nullptr) {
		// 	node->update(state.get_result(node->player_to_move));
		// 	/* Check whether the situation is still satisfied or not */
		// 	node = node->parent;
		// }

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
typename State::Move compute_move(const State root_state,
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


#endif