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
	struct Coord {
		int bit_idx;
		uint64_t bitmask;
		int x, y;
		int bitboard_idx;
		int local_bit_idx;
	};
	Coord init_coord(int x, int y) {
		Coord c;
		
		c.bit_idx = (y * 8) + x;
		c.bitmask = 1ULL << c.bit_idx; 
		c.x = x;
		c.y = y;
		c.bitboard_idx = c.bit_idx / 64;
		c.local_bit_idx = c.bit_idx % 64;
		
		return c;
	}
	Coord init_coord(int bit_idx) {
		Coord c;
		
		c.bit_idx = bit_idx;
		c.bitmask = 1ULL << c.bit_idx; 
		c.x = c.bit_idx % 8;
		c.y = c.bit_idx / 8;
		c.bitboard_idx = c.bit_idx / 64;
		c.local_bit_idx = c.bit_idx % 64;
		
		return c;
	}
	Coord init_coord(uint64_t bitmask) {
	
		Coord c;
		c.bitmask = bitmask; 
		
		#if defined(__GNUC__) || defined(__clang__)
			c.bit_idx = __builtin_ctzll(c.bitmask);
		#else
		
			c.bit_idx = 0;
			while ((c.bitmask & 1) == 0 && c.bit_idx < 64) {
				c.bitmask >>= 1;
				c.bit_idx++;
			}
		#endif
		
		c.bitboard_idx = c.bit_idx / 64;
		c.local_bit_idx = c.bit_idx % 64;
		c.x = c.bit_idx % 8;
		c.y = c.bit_idx / 8;
		
		return c;
	}
	uint64_t xycoord_to_bitmask(int x, int y) {
		return (1ULL << ((y*8) + x));
	}
	uint64_t strcoord_to_bitmask(const std::string& coord) {
		if(coord.length() != 2) return 0;
	
	
		int x = coord[0] - 'A';
		int y = coord[1] - '1';
		
		return xycoord_to_bitmask(x, y);
	}
}


class Piece {
public:
	
	Piece(char pce_stamp, std::string symbol, std::string pce_name): 
		_stamp(pce_stamp),
		_symbol(symbol), 
		_name(pce_name), 
		_current_coord(nullptr) {}
	
	void set_attribute(const std::string& attr_name, const std::vector<std::string>& attrs) {
		_m_attrs[attr_name] = attrs;	
	}
	void assign_new_coord(std::shared_ptr<cc::Coord> coord) {
		_current_coord = std::move(coord);
	}
	std::shared_ptr<cc::Coord> get_current_coord() const {
		return _current_coord;
	}
	
	
	char stamp() const {
		return _stamp;
	}
	
	std::string get_symbol() const {
		return _symbol;
	}
	
	
	std::string get_name() const {
		return _name;
	}
	
protected:
	std::map<std::string, std::vector<std::string>> _m_attrs;
	std::shared_ptr<cc::Coord> _current_coord;
	std::string _name;
	std::string _symbol;
	char _stamp;
private:
	
		
};



class Board {
public:	
	Board() : _height(8), _width(8), _num_bitboards((_height * _width) / 64) {
		_v_board.resize(_num_bitboards, 0ULL); // init to 0
		
		
		for(int y = _height ; y >= 0 ; --y) {
			for(int x = 0 ; x < _width ; ++x) {
				cc::Coord coord = cc::init_coord(x, y);
				_m_pos[coord.bitmask] = std::make_shared<cc::Coord>(coord);
			}
		}
	}
	
	void add_piece(std::shared_ptr<Piece> pce_ptr, int x, int y) {
	
		auto coord = std::make_shared<cc::Coord>(cc::init_coord(x, y));
		pce_ptr->assign_new_coord(coord);
		
		_v_board[coord->bitboard_idx] |= coord->bitmask;
		_m_pces[pce_ptr->get_name()] = pce_ptr;
	}
	
	void draw() {
	
		uint64_t all_occupancies = _v_board[0];
		
		for(int y = _height - 1 ; y >= 0 ; --y) {
			for(int x = 0 ; x < _width ; ++x) {
				cc::Coord coord = cc::init_coord(x, y);
				_m_pos[coord.bitmask] = std::make_shared<cc::Coord>(coord);
				
				bool has_piece = (_v_board[coord.bitboard_idx] & coord.bitmask) != 0;
				
				if(has_piece) {
				
					for(auto& [pce_name, pce_ptr] : _m_pces) {
						if(pce_ptr->get_current_coord()->bitmask == coord.bitmask) {
							std::cout << "[" << pce_ptr->get_symbol() << "]";
							break;
						}
					}
				} else {
					std::cout << "[-]";
				}
			}
			std::cout << '\n';
		}
	
	}
	
	
	
	void move_piece(uint64_t from, uint64_t to) {
		int from_idx = cc::init_coord(from).bitboard_idx;
		int to_idx = cc::init_coord(to).bitboard_idx;
		
		_v_board[from_idx] &= ~from;
		_v_board[to_idx] |= to;
		
		for(auto& [name, pce_ptr] : _m_pces) {
			if(pce_ptr->get_current_coord()->bitmask == from) {
				auto new_coord = std::make_shared<cc::Coord>(cc::init_coord(to));
				pce_ptr->assign_new_coord(new_coord);
				break;
			}
		}
		
		
	}
	
	std::shared_ptr<Piece> get_piece_by_name(std::string pce_name) {
		return _m_pces.count(pce_name) ? _m_pces[pce_name] : nullptr;
	}
private:
	int _height, _width;
	std::vector<uint64_t> _v_board;
	std::map<uint64_t, std::shared_ptr<cc::Coord>> _m_pos; // Piece's positions in the map. 
	int _num_bitboards;
	std::istringstream _iss;
	std::map<std::string, std::shared_ptr<Piece>> _m_pces;	
	
	void move_bitmap(uint64_t from, uint64_t to) {
		_v_board[0] &= ~from;
		_v_board[0] |= to;
	}
		
};



class Chess {
public:
	Chess(std::shared_ptr<Board> board) : _board_ptr(std::move(board)), _playing(true) {
		auto rook = std::make_shared<Piece>('r', "♜", "rook");
		_board_ptr->add_piece(rook, 0, 0);
	}
	~Chess() = default;
	void play() {
		std::string player_response;
		while(_playing) {
			_board_ptr->draw();
			this->get_user_input();
			this->process_input();
		}
	}
	void process_input() {
		std::vector<std::string> v_tokens;
		std::string token;
		while(_iss >> token) {
			v_tokens.push_back(token);
			if(token == "quit") _playing = false;
		}

		if(!_iss.eof()) {
			std::cerr << "Error processing input." << std::endl;
		}
		this->update_board_w_input(v_tokens);
	}
	bool is_valid_move(Piece *pce_ptr, uint64_t from, uint64_t to) {
		auto from_coord = cc::init_coord(from);
		auto to_coord = cc::init_coord(to);
		
		// Pieces logic here.
		return true;
	}
	void update_board_w_input(const std::vector<std::string>& tokens) {
		if(tokens.empty()) return;
		
		std::string action_token = tokens[0];
		
		if(action_token == "move" && tokens.size() >= 3) {
			std::string pce_name = tokens[1];
			std::shared_ptr<Piece> pce_ptr = _board_ptr->get_piece_by_name(pce_name);
			
			if(pce_ptr) { // If the piece exists on the board
				uint64_t target_bitmask = cc::strcoord_to_bitmask(tokens[2]);
				uint64_t current_bitmask = pce_ptr->get_current_coord()->bitmask;
				
				if(this->is_valid_move(pce_ptr.get(), current_bitmask, target_bitmask)) {
					_board_ptr->move_piece(current_bitmask, target_bitmask);
				} else {
					std::cerr << "Illegal move!\n";
				}
			} else {
				std::cerr << "Piece not found: " << pce_name << "\n";
			}
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
		
		_iss.clear();
		_iss.str(input);
	}
private:
	std::shared_ptr<Board> _board_ptr;
	std::istringstream _iss;
	bool _playing;
};


int main() {
	auto myBoard = std::make_shared<Board>();
	
	Chess game_0(myBoard);
	game_0.play();
	
	return 0;
}
