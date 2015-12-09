#include "gomoku.cpp"
#include "mcts.cpp"

using namespace std;

int main()
{
	bool human_player = true;
	time_t start, end;

	ComputeOptions player1_options, player2_options;
	player1_options.max_iterations = 100000;
	player1_options.verbose = true;
	player2_options.max_iterations =  10000;
	player2_options.verbose = true;

	GomokuState state;
	MCTS mcts_computation;
	while (state.has_moves()) {
		cout << endl << "State: " << state << endl;

		GomokuState::Move move = {-1, -1};
		if (state.player_to_move == 1) {
			cout << "Computer is caculating its move..." << endl;
			time(&start);
			move = mcts_computation.compute_move(state, player1_options);
			state.do_move(move);
			time(&end);
			cout << "***************************" << endl;
			cout << "Computer move spends time: " << (end-start) << " seconds" << endl;
		}
		else {
			if (human_player) {
				int row_move, col_move;
				while (true) {
					cout << "Input your move: \n";
					cout << "  Row index(0-5): ";
					cin >> row_move;
					cout << "  Column index(0-5): ";
					cin >> col_move;
					move = {row_move, col_move};

					string invalid_move_msg;
					if (!state.check_invalid_move(row_move, col_move, invalid_move_msg)){
						cout << invalid_move_msg << endl << endl;
						continue;
					}
					break;
				}
				state.do_move(move);
				move.clear();
			}
			else {
				move = mcts_computation.compute_move(state, player2_options);
				state.do_move(move);
			}
		}
	}

	cout << endl << "Final state: " << state << endl;

	if (state.get_result(2) == 1.0) {
		cout << "Player 1 wins!" << endl;
	}
	else if (state.get_result(1) == 1.0) {
		cout << "Player 2 wins!" << endl;
	}
	else {
		cout << "We tie!" << endl;
	}

	return 0;
}