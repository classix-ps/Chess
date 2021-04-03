#include "game.hpp"

int main() {
	Game game(L"C:/Program Files/Stockfish/stockfish_x86-64-avx2.exe", "../images/boardStone.png", 680, { 20, 20 }, "../images/figuresStone.png", "../images/menu.jpg");
	game.play();

	return 0;
}