#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <sstream>
#include <limits>
#include <algorithm>
#if defined(__GNUC__) || defined(__clang__)
#include <climits>
#endif

class Board;
class Chess;
class Piece;

namespace cc {
	struct BoardCoord {
		int row;
		int col;
		int local_idx;
		std::string alpha;
	};
	
	int calc_local_index(const int x, const int y, const int width) {
		return (y * width) + x;
	}
	
	std::string xy_to_alpha(int x, int y) {
		std::string out("Z9");
		out[0] = x + 'A';
		out[1] = y + '1';
		return out;
	}
	
	BoardCoord init_boardcoord_alpha(const std::string& alpha, const int width) {
		BoardCoord c;
		
		int x = alpha[0] - 'A';
		int y = alpha[1] - '1';
		
		c.row = x;
		c.col = y;
		c.alpha = alpha;
		c.local_idx = calc_local_index(x, y, width);
		
		return c;
	}
	BoardCoord init_boardcoord_xy(int x, int y, int width) {
		BoardCoord c;
		
		c.row = x;
		c.col = y;
		c.alpha = xy_to_alpha(x, y);
		c.local_idx = calc_local_index(x, y, width);
		
		return c;
	}

}


class Piece {
public:
	
	Piece(char symbol, std::string pce_name): 
		_symbol(symbol), 
		_name(pce_name), 
		_current_loc(nullptr) {}
	
	void set_attribute(const std::string& attr_name, const std::string& attr) {
		_attrs_map[attr_name] = attr;	
	}
	void assign_new_location(const cc::BoardCoord& coord) {
		_current_loc = std::make_unique<cc::BoardCoord>(coord);
	}
	cc::BoardCoord &get_location() const {
		return *_current_loc.get();
	}
	const std::map<std::string, std::string>& get_all_attributes() {
		return _attrs_map;
	}
	const std::string& get_attribute(std::string attr_name) {
		auto it = _attrs_map.find(attr_name);
		return it != _attrs_map.end() ? it->second : std::string("");// TODO: else it should return a local constant value instead.
	}
	
	char get_symbol() const {
		return _symbol;
	}
	
	
	std::string get_name() const {
		return _name;
	}
	
protected:
	std::map<std::string, std::string> _attrs_map;
	std::unique_ptr<cc::BoardCoord> _current_loc;
	std::string _name;
	char _symbol;
private:
	
		
};




class Board {
public:	
	static constexpr uint64_t ENUMERATED = 0x1;
	
	Board(const int height, const int width, const uint64_t flags) 
	: _height(height), _width(width), _flags(flags) {
		std::fill(_board_spaces_arr, _board_spaces_arr + width * height, '-');
	}
	
	void position_piece(Piece& pce, int x, int y) {
	
		cc::BoardCoord cur_loc = pce.get_location();
		int new_idx = cc::calc_local_index(x, y, _width);
		
		_board_spaces_arr[new_idx] = pce.get_symbol();
		
		pce.assign_new_location(new_idx);
		add_piece(&pce);
		
	}
	
	void add_piece(Piece &pce) {
		char symbol = pce_ptr->get_symbol();
		_pces_map[symbol] = std::make_shared<Piece>(pce_ptr); 
	}
	
	
	void draw() const {
	
		bool board_enumerated = this->is_flag_active(Board::ENUMERATED);
		
		// Enumerating top rows.
		if(board_enumerated) {
			std::cout << "  ";
			for(int x = 0 ; x < _width ; ++x) {
				std::cout << " " << static_cast<char>(x + 'A') << ' ';
			}
			
			std::cout << '\n';
		}
		
		for(int y = _height - 1 ; y >= 0 ; --y) {
			
			// Enumerating top columns.
			if(board_enumerated) std::cout << (y + 1) << " ";
			
			for(int x = 0 ; x < _width ; ++x) {
				int idx = y*_width+x;
				std::cout << "[" << _board_spaces_arr[idx] << "]";
			}
			// Enumerating bottom columns.
			if(board_enumerated) std::cout << " " << y + 1;
			std::cout << '\n';
			
		}
		
		// Enumerating bottom rows.
		if(board_enumerated) {
			std::cout << "  ";
			for(int x = 0 ; x < _width ; ++x) {
				std::cout << " " << static_cast<char>(x + 'A') << ' ';
			}
			std::cout << '\n';
		}
	
	}
	
	void move_piece(Piece& pce, const cc::BoardCoord& new_loc) {

	
		
		pce.assign_new_location(new_loc);
		
		int new_idx = new_loc.local_idx;
		int old_idx = pce.get_location().local_idx;
		
		_board_spaces_arr[pce.get_location().local_idx] = '-'; // Reset old
		_board_spaces_arr[new_loc.local_idx] = pce.get_symbol();   // Set new
		const_cast<Piece&>(pce).assign_new_location(new_loc);
		 
		
	}

	const Piece* get_piece_by_symbol(char symbol) {
		auto it = _pces_map.find(symbol);
		if(it != _pces_map.end() && it->second) {
			return it->second.get();
		}
		return nullptr;
	}
	const Piece* get_piece_by_name(const std::string& pce_name) const {
		for(const auto& pair : _pces_map) {
			if(pair.second && pair.second->get_name() == pce_name) {
				return pair.second.get();
			}
		}
		return nullptr;
	}
	
	void set_enumerate() {
		turn_on(Board::ENUMERATED);
	}
	void clean_enumerate() {
		turn_off(Board::ENUMERATED);
	}
	
private:
	
	std::map<char, std::shared_ptr<Piece>> _pces_map;
	int _height, _width;
	char _board_spaces_arr[300];
	uint64_t _flags; 
	std::istringstream _iss;
	
	// XXX: Logic
	void turn_off(const uint64_t flag) {
		_flags &= ~flag;
	}
	
	void turn_on(const uint64_t flag) {
		_flags |= flag;
	}
	bool is_flag_active(const uint64_t flag) const {
		return (_flags & flag) != 0;
	}
	
};


class Chess {
public:
	
	Chess() : _board_ptr(std::move(std::make_unique<Board>(8, 8))), _playing(true) {
		generate_black_pieces();
		generate_white_pieces();
	}
	~Chess() = default;
	
	void play() {
		if(!_board_ptr) {
			throw std::runtime_error("Unexpected, we don't have a board to play with.");
		}
		while(_playing) {
			_board_ptr->draw();
			get_user_input();
			process_input();
		}
	}
	
	void process_input() {
		std::vector<std::string> tokenized_input_lst;
		std::string input;
		
		
		
		while(_iss >> input) {
	
			if(input == "quit") {
				_playing = false; // Bye!
				break;
			}
			std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) { return std::tolower(c); });
			tokenized_input_lst.push_back(input);
		}
		
		if(_iss.fail() && !_iss.eof()) {
			std::cerr << "Error processing input." << std::endl;
		}
		
		update_board_w_input(tokenized_input_lst);
	}
	
	bool is_valid_move(Piece *pce_ptr, cc::BoardCoord& from, cc::BoardCoord& to) {
		
		// Logic for later.
		return true;
	}
	/**
	*	Updates the board based on the user's input...
	*	for actions relevant to the game universe, actions s.a.: "move", "analyze", "run", etc...
	*	input: 
	*		tokens - A series of strings containing user input separated by space using `split`.
	*	output:
	*		void
	*/
	void update_board_w_input(const std::vector<std::string>& input_lst) {
		if(input_lst.empty()) return;
		
		std::string action_token = input_lst[0];
		
		if(action_token == "move" && input_lst.size() >= 3) {
			std::string pce_name = input_lst[1];
			std::string pce_new_loc = input_lst[2];
			move_piece_action(pce_name, pce_new_loc);
		} 
		else if (action_token == "check" && input_lst.size() >= 2 ) {
			std::string pce_name = input_lst[1];
			//check_action(pce_name); // 
		} 
		else if (action_token == "make" && input_lst.size() >= 2) {
			if(input_lst[1] == "enumerate") {
				_board_ptr->set_enumerate();
			}
		} 
		else if (action_token == "clean" && input_lst.size() >= 2) {
			if(input_lst[1] == "enumerate") {
				_board_ptr->clean_enumerate();
			}
		} 
		else {
			std::cout << "Can't process the command." << std::endl;
		}
	}
	
	void toggle_playing() {
		_playing = !_playing;
	}
	void get_user_input() {
		std::string input;
		std::cout << "\n>: ";
		if(!std::getline(std::cin, input)) {
			std::cerr << "Error reading input." << std::endl;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return get_user_input();
		}
		
		clean_stream();
		_iss.str(input);
	}
private:
	std::shared_ptr<Board> _board_ptr;
	std::istringstream _iss;
	bool _playing;
	
	
	// Pieces generation... 
	void generate_black_pieces() {
	
		Piece rook_b_0 = Piece('r', "♖", "black_rook_0");
		Piece knight_b_0 = Piece('k', "♘", "black_knight_0"); 
		Piece bishop_b_0 = Piece('t', "♗", "black_bishop_0"); 
		Piece queen_b = Piece('q', "♕", "black_queen"); 
		Piece king_b = Piece('k', "♔", "black_king"); 
		Piece bishop_b_1 = Piece('t', "♗", "black_bishop_1"); 
		Piece knight_b_1 = Piece('k', "♘", "black_knight_1"); 
		Piece rook_b_1 = Piece('r', "♖", "black_rook_1");
		Piece peon_b_0 = Piece('p', "♙", "black_peon_0");
		Piece peon_b_1 = Piece('p', "♙", "black_peon_1");
		Piece peon_b_2 = Piece('p', "♙", "black_peon_2");
		Piece peon_b_3 = Piece('p', "♙", "black_peon_3");
		Piece peon_b_4 = Piece('p', "♙", "black_peon_4");
		Piece peon_b_5 = Piece('p', "♙", "black_peon_5");
		Piece peon_b_6 = Piece('p', "♙", "black_peon_6");
		Piece peon_b_7 = Piece('p', "♙", "black_peon_7");
		generate_rook_move_logic(rook_b_0);
		_board_ptr->position_piece(rook_b_0, 0, 7);
		_board_ptr->position_piece(knight_b_0, 1, 7);
		_board_ptr->position_piece(bishop_b_0, 2, 7);
		_board_ptr->position_piece(queen_b, 3, 7);
		_board_ptr->position_piece(king_b, 4, 7);
		_board_ptr->position_piece(bishop_b_1, 5, 7);
		_board_ptr->position_piece(knight_b_1, 6, 7);
		_board_ptr->position_piece(rook_b_1, 7, 7);
		_board_ptr->position_piece(peon_b_0, 0, 6);
		_board_ptr->position_piece(peon_b_1, 1, 6);
		_board_ptr->position_piece(peon_b_2, 2, 6);
		_board_ptr->position_piece(peon_b_3, 3, 6);
		_board_ptr->position_piece(peon_b_4, 4, 6);
		_board_ptr->position_piece(peon_b_5, 5, 6);
		_board_ptr->position_piece(peon_b_6, 6, 6);
		_board_ptr->position_piece(peon_b_7, 7, 6);
		
	}
	
	void generate_white_pieces() {
		Piece rook_w_0 = Piece('R', "♜", "white_rook_0");
		Piece knight_w_0 = Piece('K', "♞", "white_knight_0"); 
		Piece bishop_w_0 = Piece('T', "♝", "white_bishop_0"); 
		Piece queen_w = Piece('Q', "♛", "white_queen"); 
		Piece king_w = Piece('K', "♚", "white_king"); 
		Piece bishop_w_1 = Piece('T', "♝", "white_bishop_1"); 
		Piece knight_w_1 = Piece('K', "♞", "white_knight_1"); 
		Piece rook_w_1 = Piece('R', "♜", "white_rook_1");
		Piece peon_w_0 = Piece('P', "♟", "white_peon_0");
		Piece peon_w_1 = Piece('P', "♟", "white_peon_1");
		Piece peon_w_2 = Piece('P', "♟", "white_peon_2");
		Piece peon_w_3 = Piece('P', "♟", "white_peon_3");
		Piece peon_w_4 = Piece('P', "♟", "white_peon_4");
		Piece peon_w_5 = Piece('P', "♟", "white_peon_5");
		Piece peon_w_6 = Piece('P', "♟", "white_peon_6");
		Piece peon_w_7 = Piece('P', "♟", "white_peon_7");
		_board_ptr->position_piece(rook_w_0, 0, 0);
		_board_ptr->position_piece(knight_w_0, 1, 0);
		_board_ptr->position_piece(bishop_w_0, 2, 0);
		_board_ptr->position_piece(queen_w, 3, 0);
		_board_ptr->position_piece(king_w, 4, 0);
		_board_ptr->position_piece(bishop_w_1, 5, 0);
		_board_ptr->position_piece(knight_w_1, 6, 0);
		_board_ptr->position_piece(rook_w_1, 7, 0);
		_board_ptr->position_piece(peon_w_0, 0, 1);
		_board_ptr->position_piece(peon_w_1, 1, 1);
		_board_ptr->position_piece(peon_w_2, 2, 1);
		_board_ptr->position_piece(peon_w_3, 3, 1);
		_board_ptr->position_piece(peon_w_4, 4, 1);
		_board_ptr->position_piece(peon_w_5, 5, 1);
		_board_ptr->position_piece(peon_w_6, 6, 1);
		_board_ptr->position_piece(peon_w_7, 7, 1);
		
	}
	
	void clean_stream() {
		_iss.clear();
		_iss.str("");
	}
	
	void generate_rook_move_logic(std::shared_ptr<Piece>& pce_ptr) {
		
		pce_ptr->set_attribute("movement", "& all_x all_y");
		
	}
	
	// XXX: LOGIC 
	
	/**
	*	Dynamically generates and combines two lists of coordinates based on two logical orders.
	*	input:
	*		coord_order_0 - A set of coordinates represented as a logical string.
	*		coord_order_y - Another set of coordinates, the same as coord_order_0.
	*	output:
	*		A list of Coord objects.
	*/
	//std::vector<Coord>& _and(const std::string& coord_order_0, const std::string& coord_order_1) {
	//	return 
	//}
	
	// XXX: ACTIONS
	void move_piece_action(const std::string& pce_name, const std::string& to) {
		auto pce_ptr = _board_ptr->get_piece_by_name(pce_name);
		auto new_loc = cc::init_boardcoord(to);
			
		if(pce_ptr && new_loc) { // If the piece exists on the board
			
			_board_ptr->move_piece(pce_ptr, new_loc);
			
		} else {	
			std::cerr << "Piece not found: " << pce_name << "\n";
		}
	}

	
};


int main() {
	auto myBoard = std::make_shared<Board>(8, 8, Board::ENUMERATED);
	
	Chess game_0(myBoard);
	game_0.play();
	
	return 0;
}
