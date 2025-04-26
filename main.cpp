#include <iostream>
#include <string>
#include <vector>


//
/**
	After receiving a specified width and height, Board class generates a board.
	input:
		width - width length of the board as an integer.
		height - height length of the board as an integer.
*/
class Board {
public:
	Board(int width, int height) : _width(width), _height(height), _area(width * height), _board(width * height, '-') {}
private:
	const int _width, _height, _area;
	std::vector<char> _board;
};
//

int main() {
	Board(8, 8);
}
