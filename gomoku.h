#include <algorithm>
#include <iostream>
using namespace std;

#include "mcts.h"
#include "mersenne.h"


class GomokuState
{
private:
	int num_rows, num_cols;
	vector<vector<char>> board;
	int last_col;
	int last_row;

	void check_invariant() const{
		assert(player_to_move == 1 || player_to_move == 2);
	}

public:
	typedef vector<int> Move;
	//static const Move no_move(temp, temp+1);
	int player_to_move;
	static const char player_markers[3];
	vector<vector<vector<uint64_t>>> zobrist_table;

	GomokuState(int num_rows_ = 9, int num_cols_ = 9){
		player_to_move = 1;
	    num_rows = num_rows_;
	    num_cols = num_cols_;
		last_col = -1;
		last_row = -1;
		board.resize(num_rows, vector<char>(num_cols, player_markers[0]));
		zobrist_table.resize(num_rows, vector<vector<uint64_t>>(num_cols, vector<uint64_t>(3, 0)));
		initialize_zobrist_table();
	}

	void do_move(Move move);
	template<typename RandomEngine>
	void do_random_move(RandomEngine* engine);
	bool has_moves() const;
	vector<Move> get_moves() const;
	char get_winner() const;
	double get_result(int current_player_to_move) const;
	void print(ostream& out) const;
	bool check_invalid_move(int row, int col, string& error_msg);

	/* Generate Zobrist hash keys */
	int get_pos_marker_ind(char board_pos_marker);
	void initialize_zobrist_table();
	uint64_t caculate_zobrist_key(int player_move, vector<int> board_pos);
	uint64_t update_zobrist_key(uint64_t old_key, char old_marker, char new_marker, vector<int> board_pos);

};

ostream& operator << (ostream& out, const GomokuState& state)
{
	state.print(out);
	return out;
}

