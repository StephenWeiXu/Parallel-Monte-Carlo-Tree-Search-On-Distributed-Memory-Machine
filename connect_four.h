#include <algorithm>
#include <iostream>
using namespace std;

#include "mcts.h"

class ConnectFourState
{
public:
	typedef int Move;
	static const Move no_move = -1;
	int player_to_move;
	static const char player_markers[3]; 

	ConnectFourState(int num_rows_ = 6, int num_cols_ = 7){
		player_to_move = 1;
	    num_rows = num_rows_;
	    num_cols = num_cols_;
		last_col = -1;
		last_row = -1;
		board.resize(num_rows, vector<char>(num_cols, player_markers[0]));
	}

	void do_move(Move move);
	template<typename RandomEngine>
	void do_random_move(RandomEngine* engine);
	bool has_moves() const;
	vector<Move> get_moves() const;
	char get_winner() const;
	double get_result(int current_player_to_move) const;
	void print(ostream& out) const;

private:
	void check_invariant() const
	{
		assert(player_to_move == 1 || player_to_move == 2);
	}

	int num_rows, num_cols;
	vector<vector<char>> board;
	int last_col;
	int last_row;
};

ostream& operator << (ostream& out, const ConnectFourState& state)
{
	state.print(out);
	return out;
}

