#include <iostream>
#include <ctime>
using namespace std;

#include "mcts.cpp"
#include "gomoku.h"

/* Use a constant instead of transposing the key depending on the side to play.
	This is more efficient, since I can more easily undo operations (so I can
	incrementally update the key). */
static const uint64_t play1_to_move_key = 0x8913125CFB309AFC; 

/********************************************************************************/
typedef vector<int> Move; 
const char GomokuState::player_markers[3] = {'.', 'X', 'O'}; 

const string POSITION_ALREADY_USED = "Invalid Move: position is already used!";
const string INVALID_INDEX = "Invalid Move: the index is out of range!";

// Input - move(vctor<int>): first element is the row index, and the second element is the column index
void GomokuState::do_move(Move move)
{
	assert(0 <= move[0] && move[0] < num_rows);
	assert(0 <= move[1] && move[1] < num_cols);
	assert(board[move[0]][move[1]] == player_markers[0]);
	check_invariant();

	//int row = num_rows - 1;
	//while (board[row][move] != player_markers[0]) row--;
	board[move[0]][move[1]] = player_markers[player_to_move];
	last_row = move[0];
	last_col = move[1];

	player_to_move = 3 - player_to_move;
}

template<typename RandomEngine>
void GomokuState::do_random_move(RandomEngine* engine)
{
	assert(has_moves());
	check_invariant();
	uniform_int_distribution<int> row_random(0, num_rows-1);
	uniform_int_distribution<int> col_random(0, num_cols-1);
	Move random_move = {-1, -1};
	int row_rand, col_rand;

	while (true) {
		row_rand = row_random(*engine);
		col_rand = col_random(*engine);
		random_move = {row_rand, col_rand};
		if (board[row_rand][col_rand] == player_markers[0]) {
			do_move(random_move);
			return;
		}
		random_move.clear();
	}
}

bool GomokuState::has_moves() const
{
	check_invariant();

	char winner = get_winner();
	if (winner != player_markers[0]) {
		return false;
	}

	for (int row=0; row < num_rows; ++row){
		for (int col = 0; col < num_cols; ++col) {
			if (board[row][col] == player_markers[0]) {
				return true;
			}
		}
	}
	return false;
}

vector<Move> GomokuState::get_moves() const
{
	check_invariant();

	vector<Move> moves;
	if (get_winner() != player_markers[0]) {
		return moves; // return empty 2d vector, which means there is already a winner
	}

	moves.reserve(num_rows*num_cols);

	for (int row=0; row < num_rows; ++row){
		for (int col = 0; col < num_cols; ++col) {
			if (board[row][col] == player_markers[0]) {
				moves.push_back({row, col});
			}
		}
	}
	return moves;
}

char GomokuState::get_winner() const
{
	if (last_col < 0) {
		return player_markers[0];
	}

	// We only need to check around the last piece played.
	auto piece = board[last_row][last_col];

	// X X X X
	int left = 0, right = 0;
	for (int col = last_col - 1; col >= 0 && board[last_row][col] == piece; --col) left++;
	for (int col = last_col + 1; col < num_cols && board[last_row][col] == piece; ++col) right++;
	if (left + 1 + right >= 5) {
		return piece;
	}

	// X
	// X
	// X
	// X
	int up = 0, down = 0;
	for (int row = last_row - 1; row >= 0 && board[row][last_col] == piece; --row) up++;
	for (int row = last_row + 1; row < num_rows && board[row][last_col] == piece; ++row) down++;
	if (up + 1 + down >= 5) {
		return piece;
	}

	// X
	//  X
	//   X
	//    X
	up = 0;
	down = 0;
	for (int row = last_row - 1, col = last_col - 1; row >= 0 && col >= 0 && board[row][col] == piece; --row, --col) up++;
	for (int row = last_row + 1, col = last_col + 1; row < num_rows && col < num_cols && board[row][col] == piece; ++row, ++col) down++;
	if (up + 1 + down >= 5) {
		return piece;
	}

	//    X
	//   X
	//  X
	// X
	up = 0;
	down = 0;
	for (int row = last_row + 1, col = last_col - 1; row < num_rows && col >= 0 && board[row][col] == piece; ++row, --col) up++;
	for (int row = last_row - 1, col = last_col + 1; row >= 0 && col < num_cols && board[row][col] == piece; --row, ++col) down++;
	if (up + 1 + down >= 5) {
		return piece;
	}

	return player_markers[0];
}

double GomokuState::get_result(int current_player_to_move) const
{
	assert( ! has_moves());
	check_invariant();

	auto winner = get_winner();
	if (winner == player_markers[0]) {
		return 0.5;
	}

	if (winner == player_markers[current_player_to_move]) {
		return 0.0;
	}
	else {
		return 1.0;
	}
}

void GomokuState::print(ostream& out) const
{
	out << endl;
	out << " ";
	for (int col = 0; col < num_cols - 1; ++col) {
		out << col << ' ';
	}
	out << num_cols - 1 << endl;
	for (int row = 0; row < num_rows; ++row) {
		out << "|";
		for (int col = 0; col < num_cols - 1; ++col) {
			out << board[row][col] << ' ';
		}
		out << board[row][num_cols - 1] << "| "  << row << endl;
	}
	out << "+";
	for (int col = 0; col < num_cols - 1; ++col) {
		out << "--";
	}
	out << "-+" << endl;
	out << player_markers[player_to_move] << " to move " << endl << endl;
}

bool GomokuState::check_invalid_move(int row, int col, string& error_msg){
	if(row < 0 || row > num_rows || col < 0 || col > num_cols){
		error_msg = INVALID_INDEX;
		return false;
	}
	if (board[row][col] != player_markers[0]){
		error_msg = POSITION_ALREADY_USED;
		return false;
	}
	return true;
}

/* Generate Zobrist hash keys */
int GomokuState::get_pos_marker_ind(char board_pos_marker){
	if (board_pos_marker == player_markers[0]){
		return 0;
	}else if (board_pos_marker == player_markers[1]){
		return 1;
	}else{
		return 2;
	}	
}

void GomokuState::initialize_zobrist_table(){
	MersenneTwister rand_gene;
	for (int i=0; i< num_rows; i++){
		for (int j=0; j< num_cols; j++){
			for (int k=0; k<3; k++){
				zobrist_table[i][j][k] = rand_gene.Rand64();
			}
		}
	}
}

uint64_t GomokuState::caculate_zobrist_key(int player_move, vector<int> board_pos){
	uint64_t zobrist_key = 0;
	int marker = get_pos_marker_ind(board[board_pos[0]][board_pos[1]]);
	zobrist_key ^= zobrist_table[i][j][marker];

	if (player_move == 1){
		zobrist_key ^= play1_to_move_key;
	}

	return zobrist_key;
}

uint64_t GomokuState::update_zobrist_key(uint64_t old_key, char old_marker, char new_marker, vector<int> board_pos){
	uint64_t new_key = old_key;
	int old_marker_ind = get_pos_marker_ind(old_marker);
	int new_marker_ind = get_pos_marker_ind(new_marker);
	new_key ^= zobrist_table[board_pos[0]][board_pos[1]][old_marker_ind];
	new_key ^= zobrist_table[board_pos[0]][board_pos[1]][new_marker_ind];
	new_key ^= play1_to_move_key;
	return new_key;
}

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
	while (state.has_moves()) {
		cout << endl << "State: " << state << endl;

		GomokuState::Move move = {-1, -1};
		if (state.player_to_move == 1) {
			cout << "Computer is caculating its move..." << endl;
			time(&start);
			move = compute_move(state, player1_options);
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
					cout << "Row index(0-5): ";
					cin >> row_move;
					cout << "Column index(0-5): ";
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
				move = compute_move(state, player2_options);
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

// int main()
// {

// 	GomokuState state;
	
// 	uint64_t initial_key = state.caculate_zobrist_key(1);
// 	cout << "initial key: " << initial_key << endl;

// 	cout << "now update" << endl;

// 	uint64_t new_key = state.update_zobrist_key(initial_key, state.player_markers[0], state.player_markers[1], {1,3});

// 	cout << "new key: " << new_key << endl;

// 	return 0;

// }