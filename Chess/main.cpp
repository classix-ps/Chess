#include <SFML/Graphics.hpp>
#include <iostream>
#include <time.h>
#include "Connector.hpp"

#define Z 2

using namespace sf;

int size = Z * 56; // size of a square in image file
Vector2f offset(Z * 28, Z * 28); // Starts pieces away from the edges

Sprite f[32]; // figures, first 16 black, last 16 white
std::string position = ""; // record of all moves

int board[8][8] =
{ -1,-2,-3,-4,-5,-3,-2,-1,
 -6,-6,-6,-6,-6,-6,-6,-6,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  6, 6, 6, 6, 6, 6, 6, 6,
  1, 2, 3, 4, 5, 3, 2, 1 };

bool turn = true; // 0 for black, 1 for white
bool holdingPiece = false;

std::map<int, int> enPassantPos; // keeps track of when en passant took place and which pawn was taken
std::map<int, int> promotedPawns; // keeps track of promoted pawns and their new piece type

std::string toChessNote(Vector2f p) // Converts position of move to chess notation
{
	std::string s = "";
	s += char(p.x / size + 97);
	s += char(7 - p.y / size + 49);
	return s;
}

Vector2f toCoord(char a, char b) // Converts chess notation to positional coordinates
{
	int x = int(a) - 97;
	int y = 7 - int(b) + 49;
	return Vector2f(float(x * size), float(y * size));
}

void move(std::string str) // used mainly for loadPosition, executes a single move (including captures)
{
	Vector2f oldPos = toCoord(str[0], str[1]);
	Vector2f newPos = toCoord(str[2], str[3]);

	for (int i = 0; i < 32; i++) {
		if (f[i].getPosition() == newPos) {
			f[i].setPosition(Z * -100, Z * -100); // taken pieces pile
		}
	}

	for (int i = 0; i < 32; i++) {
		if (f[i].getPosition() == oldPos) {
			f[i].setPosition(newPos); // updates piece position in array
		}
	}
}

void loadPosition() // used if changes are made in 'position' and board state needs to be reloaded
{
	int k = 0;
	for (int i = 0; i < 8; i++) // load textures (check for promotions)
		for (int j = 0; j < 8; j++)
		{
			int n = board[i][j];
			if (!n) continue;
			int x = abs(n) - 1;
			int y = (n > 0);
			std::map<int, int>::iterator it = promotedPawns.find(k);
			if (it != promotedPawns.end()) { // sets crop in figures.png to match promoted piece
				f[k].setTextureRect(IntRect((size / Z) * (it->second - 1), (size / Z) * y, size / Z, size / Z));
			}
			else { // sets crop in figures.png to match piece
				f[k].setTextureRect(IntRect((size / Z) * x, (size / Z) * y, size / Z, size / Z));
			}
			f[k].setPosition(float(size * j), float(size * i));
			k++;
		}

	for (size_t i = 0; i < position.length(); i += 5) { // prints board by using all previous moves
		if (position.at(i) == ' ') { // adjusts for promotion, wherein wished piece is written after the move (e.g. e7e8q)
			if (i + 1 < position.length())
				i++;
			else
				break;
		}
		move(position.substr(i, 4));
		std::map<int, int>::iterator it = enPassantPos.find(i);
		if (it != enPassantPos.end()) {
			f[it->second].setPosition(Z * -100, Z * -100); // taken pieces pile
		}
	}
}

void promotion(int piece, int p) {
	bool color = piece < 16 ? 0 : 1;

	if (!p) { // user move
		while (1) {
			std::cout << std::endl << "Promotion (1 R, 2 N, 3 B, 4 Q): ";
			std::cin >> p;
			if (p >= 1 && p <= 4) {
				break;
			}
			else {
				std::cout << " Invalid input!";
			}
		}
	}

	promotedPawns.insert(std::pair<int, int>(piece, p));
	f[piece].setTextureRect(IntRect(size * (p - 1), size * color, size, size));
}

bool checkCol(Vector2f pos, bool color) { // checks if pieces of certain color occupy a given square
	for (int i = color * 16; i < color * 16 + 16; i++) {
		if (f[i].getPosition() == pos) {
			return true;
		}
	}
	return false;
}
bool checkAll(Vector2f pos) { // checks if any piece occupies a given square
	for (int i = 0; i < 32; i++) {
		if (f[i].getPosition() == pos) {
			return true;
		}
	}
	return false;
}
bool canTake(std::string str, int piece) { // checks if a piece could capture on a certain square (used mainly for tracking check)
	Vector2f oldPos = toCoord(str[0], str[1]);
	Vector2f newPos = toCoord(str[2], str[3]);
	if (oldPos == newPos) {
		return false;
	}

	bool color = piece < 16 ? 0 : 1;

	int prom = 0;
	std::map<int, int>::iterator it = promotedPawns.find(piece);
	if (it != promotedPawns.end()) {
		prom = it->second;
	}

	int startFile = int(str[0]) - 96;
	int endFile = int(str[2]) - 96;
	int startRank = int(str[1]) - 48;
	int endRank = int(str[3]) - 48;
	if (piece == 4 || piece == 28) { //-----------------------------KING-----------------------------
		if (abs(endFile - startFile) <= 1 && abs(endRank - startRank) <= 1) { // test move distance
			return true;
		}
		else return false;
	}
	else if (piece == 3 || piece == 27 || prom == 4) { //-----------------------------QUEEN-----------------------------
		if (abs(endFile - startFile) == abs(endRank - startRank)) { // diagonal move
			int dFile = (endFile > startFile) * 2 - 1;
			int dRank = (endRank > startRank) * 2 - 1;
			for (int i = startFile + dFile, j = startRank + dRank; i != endFile; i += dFile, j += dRank) {
				Vector2f currentPos = toCoord(i + 96, j + '0');
				if (checkAll(currentPos)) {
					return false;
				}
			}
			return true;
		}
		else if (endFile == startFile) { // vertical move
			int dRank = (endRank > startRank) * 2 - 1;
			for (int i = startRank + dRank; i != endRank; i += dRank) {
				Vector2f currentPos = toCoord(startFile + 96, i + '0');
				if (checkAll(currentPos)) {
					return false;
				}
			}
			return true;
		}
		else if (endRank == startRank) { // horizontal move
			int dFile = (endFile > startFile) * 2 - 1;
			for (int i = startFile + dFile; i != endFile; i++) {
				Vector2f currentPos = toCoord(i + 96, startRank + '0');
				if (checkAll(currentPos)) {
					return false;
				}
			}
			return true;
		}
		else return false;
	}
	else if (piece == 0 || piece == 7 || piece == 24 || piece == 31 || prom == 1) { //-----------------------------ROOK-----------------------------
		if (endFile == startFile) { // vertical move
			int dRank = (endRank > startRank) * 2 - 1;
			for (int i = startRank + dRank; i != endRank; i += dRank) {
				Vector2f currentPos = toCoord(startFile + 96, i + '0');
				if (checkAll(currentPos)) {
					return false;
				}
			}
			return true;
		}
		else if (endRank == startRank) { // horizontal move
			int dFile = (endFile > startFile) * 2 - 1;
			for (int i = startFile + dFile; i != endFile; i++) {
				Vector2f currentPos = toCoord(i + 96, startRank + '0');
				if (checkAll(currentPos)) {
					return false;
				}
			}
			return true;
		}
		else return false;
	}
	else if (piece == 2 || piece == 5 || piece == 26 || piece == 29 || prom == 3) { //-----------------------------BISHOP-----------------------------
		if (abs(endFile - startFile) == abs(endRank - startRank)) { // diagonal move
			int dFile = (endFile > startFile) * 2 - 1;
			int dRank = (endRank > startRank) * 2 - 1;
			for (int i = startFile + dFile, j = startRank + dRank; i != endFile; i += dFile, j += dRank) {
				Vector2f currentPos = toCoord(i + 96, j + '0');
				if (checkAll(currentPos)) {
					return false;
				}
			}
			return true;
		}
		else return false;
	}
	else if (piece == 1 || piece == 6 || piece == 25 || piece == 30 || prom == 2) { //-----------------------------KNIGHT-----------------------------
		if ((abs(endFile - startFile) == 2 && abs(endRank - startRank) == 1) || (abs(endFile - startFile) == 1 && abs(endRank - startRank) == 2)) {
			return true;
		}
		else return false;
	}
	else { //-----------------------------PAWN-----------------------------
		int d = color ? 1 : -1;
		if (abs(endFile - startFile) == 1 && endRank == startRank + d) {
			return true;
		}
		else return false;
	}

	return true;
}
bool isAttacked(Vector2f pos, bool color) { // checks if a piece could be taken by an enemy piece
	for (int i = !color * 16; i < !color * 16 + 16; i++) {
		Vector2f oldPos = f[i].getPosition();
		std::string str = toChessNote(oldPos) + toChessNote(pos);
		if (canTake(str, i)) {
			return true;
		}
	}
	return false;
}
std::string validMove(std::string str, int piece) { // checks if a move follows the rules of chess
	Vector2f oldPos = toCoord(str[0], str[1]);
	Vector2f newPos = toCoord(str[2], str[3]);

	bool color = piece < 16 ? 0 : 1;
	if (color != turn) {
		return "Not your turn!";
	}

	int prom = 0;
	std::map<int, int>::iterator it = promotedPawns.find(piece);
	if (it != promotedPawns.end()) {
		prom = it->second;
	}

	int startFile = int(str[0]) - 96;
	int endFile = int(str[2]) - 96;
	int startRank = int(str[1]) - 48;
	int endRank = int(str[3]) - 48;
	if (piece == 4 || piece == 28) { //-----------------------------KING-----------------------------
 		if ((str == "e1g1" || str == "e1c1" || str == "e8g8" || str == "e8c8") && (startRank == (piece == 4) * 7 + 1)) { // castling
			if (position.find(toChessNote(oldPos)) != std::string::npos) { // if King has moved
				return "King has moved before, can't Castle";
			}
			int dFile = (endFile > startFile) * 2 - 1;
			std::string rookPos(1, char(7 * (dFile > 0) + 97));
			rookPos += str.substr(1, 1);
			if (position.find(rookPos) != std::string::npos) {
				return "Rook has moved before, can't Castle";
			}
			for (int i = startFile + dFile; i >= 2 && i <= 7; i += dFile) { // checks for pieces
				Vector2f currentPos = toCoord(i + 96, startRank + '0');
				if (checkAll(currentPos)) {
					return "Piece is in the way of Castle";
				}
			}
			for (int i = startFile; i != endFile + dFile; i += dFile) { // checks for check
				Vector2f currentPos = toCoord(i + 96, startRank + '0');
				if (isAttacked(currentPos, color)) {
					return "Can't Castle through check";
				}
			}
			std::string rookEndPos(1, char(startFile + dFile + 96));
			rookEndPos += str.substr(1, 1);
			return "X" + rookPos + rookEndPos;
		}
		if (abs(endFile - startFile) > 1 || abs(endRank - startRank) > 1) { // test move distance
			return "King moved too far";
		}
		if (checkCol(newPos, color)) {
			return "Same color piece is in the way of King";
		}
	}
	else if (piece == 3 || piece == 27 || prom == 4) { //-----------------------------QUEEN-----------------------------
		if (abs(endFile - startFile) != abs(endRank - startRank) && endFile != startFile && endRank != startRank) { // test move pattern
			return "Queen can't move this way";
		}
		if (abs(endFile - startFile) == abs(endRank - startRank)) { // diagonal move
			int dFile = (endFile > startFile) * 2 - 1;
			int dRank = (endRank > startRank) * 2 - 1;
			for (int i = startFile, j = startRank; i != endFile; i += dFile, j += dRank) {
				Vector2f currentPos = toCoord(i + 96, j + '0');
				if (checkAll(currentPos)) {
					return "Piece is in the way of Queen move";
				}
			}
		}
		else if (endFile == startFile) { // vertical move
			int dRank = (endRank > startRank) * 2 - 1;
			for (int i = startRank; i != endRank; i += dRank) {
				Vector2f currentPos = toCoord(startFile + 96, i + '0');
				if (checkAll(currentPos)) {
					return "Piece is in the way of Queen move";
				}
			}
		}
		else if (endRank == startRank) { // horizontal move
			int dFile = (endFile > startFile) * 2 - 1;
			for (int i = startFile; i != endFile; i += dFile) {
				Vector2f currentPos = toCoord(i + 96, startRank + '0');
				if (checkAll(currentPos)) {
					return "Piece is in the way of Queen move";
				}
			}
		}
		if (checkCol(newPos, color)) {
			return "Same color piece is on Queens destination";
		}
	}
	else if (piece == 0 || piece == 7 || piece == 24 || piece == 31 || prom == 1) { //-----------------------------ROOK-----------------------------
		if (endFile != startFile && endRank != startRank) {
			return "Rook cant move this way";
		}
		if (endFile == startFile) { // vertical move
			int dRank = (endRank > startRank) * 2 - 1;
			for (int i = startRank; i != endRank; i += dRank) {
				Vector2f currentPos = toCoord(startFile + 96, i + '0');
				if (checkAll(currentPos)) {
					return "Piece is in the way of Rook move";
				}
			}
		}
		else { // horizontal move
			int dFile = (endFile > startFile) * 2 - 1;
			for (int i = startFile; i != endFile; i += dFile) {
				Vector2f currentPos = toCoord(i + 96, startRank + '0');
				if (checkAll(currentPos)) {
					return "Piece is in the way of Rook move";
				}
			}
		}
		if (checkCol(newPos, color)) {
			return "Same color piece is on Rooks destination";
		}
	}
	else if (piece == 2 || piece == 5 || piece == 26 || piece == 29 || prom == 3) { //-----------------------------BISHOP-----------------------------
		if (abs(endFile - startFile) != abs(endRank - startRank)) { // test move pattern
			return "Bishop can't move this way";
		}
		if (abs(endFile - startFile) == abs(endRank - startRank)) { // diagonal move
			int dFile = (endFile > startFile) * 2 - 1;
			int dRank = (endRank > startRank) * 2 - 1;
			for (int i = startFile, j = startRank; i != endFile; i += dFile, j += dRank) {
				Vector2f currentPos = toCoord(i + 96, j + '0');
				if (checkAll(currentPos)) {
					return "Piece is in the way of Bishop move";
				}
			}
		}
		if (checkCol(newPos, color)) {
			return "Same color piece is on Bishops destination";
		}
	}
	else if (piece == 1 || piece == 6 || piece == 25 || piece == 30 || prom == 2) { //-----------------------------KNIGHT-----------------------------
		if ((abs(endFile - startFile) != 2 || abs(endRank - startRank) != 1) && (abs(endFile - startFile) != 1 || abs(endRank - startRank) != 2)) {
			return "Knight can't move this way";
		}
		if (checkCol(newPos, color)) {
			return "Same color piece is on Knights destination";
		}
	}
	else { //-----------------------------PAWN-----------------------------
		int dRank = color ? 1 : -1;
		if (endFile == startFile) {
			if (endRank == startRank + dRank) { // normal move
				if (checkAll(newPos)) {
					return "Piece is in the way of Pawn move";
				}
			}
			else if (startRank == 7 - 5 * color && endRank == 5 - color) { // double move
				Vector2f skipSquare = toCoord(startFile + 96, (6 - 3 * color) - '0');
				if (checkAll(skipSquare) || checkAll(newPos)) {
					return "Piece is in the way of Pawn move";
				}
			}
			else {
				return "Pawn can't move this way";
			}
		}
		else if (abs(endFile - startFile) == 1 && endRank == startRank + dRank) {
			bool pawnCapture = false;
			if (checkCol(newPos, !color)) {
				pawnCapture = true;
			}
			bool enPassant = false;
			std::string lastMove = str.substr(2, 1) + std::to_string(color * 5 + 2) + str.substr(2, 1) + std::to_string(color + 4);
			Vector2f passSquare = toCoord(endFile + 96, startRank + '0');
			int n;
			for (n = 8 * !color + 8; n < 8 * !color + 16; n++) {
				if (f[n].getPosition() == passSquare) {
					enPassant = true;
					break;
				}
			}
			if (position.length() > 0 && position.substr(position.length() - 5, 4) != lastMove) {
				enPassant = false;
			}
			if (!pawnCapture && !enPassant) {
				return "Pawn can't move diagonally if there's no piece to take";
			}
			if (enPassant == true) {
				enPassantPos.insert(std::pair<int, int>(position.length(), n));
				f[n].setPosition(Z * -100, Z * -100);
			}
		}
		else {
			return "Pawn can't move this way";
		}
		if (endRank == color * 7 + 1) { // promotion
			return "Y" + std::to_string(piece);
		}
	}

	int j;
	for (j = !color * 16; j < !color * 16 + 16; j++) {
		if (f[j].getPosition() == newPos) {
			f[j].setPosition(Z * -100, Z * -100);
			break;
		}
	}
	f[piece].setPosition(newPos);
	if (isAttacked(f[color * 24 + 4].getPosition(), color)) {
		f[piece].setPosition(oldPos);
		if (j > !color * 16 && j < !color * 16 + 16) f[j].setPosition(newPos);
		return "Move would leave King in Check";
	}
	f[piece].setPosition(oldPos);
	if (j > !color * 16 && j < !color * 16 + 16) f[j].setPosition(newPos);

	return "";
}
bool mate() { // if the king is attacked no matter what move is made, checkmate (if he is not attacked in current state, stalemate)
	for (int i = turn * 16; i < turn * 16 + 16; i++) {
		if (toChessNote(f[i].getPosition()) == "_9") continue; // taken pieces dont need to be checked
		Vector2f oldPos = f[i].getPosition();
		for (int file = 1; file <= 8; file++) {
			for (int rank = 1; rank <= 8; rank++) {
				Vector2f newPos = toCoord(file + 96, rank + '0');
				std::string str = toChessNote(oldPos) + toChessNote(newPos);
				std::string error = validMove(str, i);
				if (error == "" || error[0] == 'X' || error[0] == 'Y') {
					return false;
				}
			}
		}
	}
	if (isAttacked(f[turn * 24 + 4].getPosition(), turn)) {
		std::string winner = !turn ? " White wins!" : " Black wins!";
		std::cout << std::endl << "Checkmate!" << winner << std::endl;
	}
	else {
		std::cout << std::endl << "Stalemate!" << std::endl;
	}
	return true;
}

int main()
{
	LPCWSTR enginePath = L"C:/Program Files/stockfish-11-win/Windows/stockfish_20011801_32bit.exe";
	ConnectToEngine(enginePath);

	Texture t1, t2;
	t1.loadFromFile("../images/figures0.png");
	t2.loadFromFile("../images/board0.png");

	for (int i = 0; i < 32; i++) {
		f[i].setTexture(t1);
		f[i].setScale(float(Z), float(Z));
	}
	Sprite sBoard(t2);
	sBoard.setScale(float(Z), float(Z));

	RenderWindow window(VideoMode(unsigned int(sBoard.getGlobalBounds().width), unsigned int(sBoard.getGlobalBounds().height)), "Chess");

	loadPosition();

	bool isMove = false;
	float dx = 0, dy = 0;
	Vector2f oldPos, newPos;
	std::string str;
	int n = 0;

	while (window.isOpen())
	{
		Vector2i pos = Mouse::getPosition(window) - Vector2i(offset);

		Event e;
		while (window.pollEvent(e))
		{
			if (e.type == Event::Closed)
				window.close();
			
			if (e.type == Event::KeyPressed) {
				/////move back///////
				if (e.key.code == Keyboard::BackSpace)
				{
					if (position.length() > 9 && (position.substr(position.length() - 10, 9) == "e1g1 h1f1" || position.substr(position.length() - 10, 9) == "e8g8 h8f8" || position.substr(position.length() - 10, 9) == "e1c1 a1d1" || position.substr(position.length() - 10, 9) == "e8c8 a8d8")) { // checks for castling
						position.erase(position.length() - 11, 10);
					}
					else {
						std::string lastMove = position.substr(position.length() - 5, 4); // checks for promotion
						std::string lastPos = lastMove.substr(2, 2);
						int lastPieceMoved;
						for (lastPieceMoved = 0; lastPieceMoved < 32; lastPieceMoved++) {
							if (toChessNote(f[lastPieceMoved].getPosition()) == lastPos) {
								break;
							}
						}
						bool color = lastPieceMoved < 16 ? 0 : 1;
						if (lastPieceMoved >= 8 && lastPieceMoved < 24 && lastMove[1] - '0' == color * 5 + 2 && lastMove[3] - '0' == color * 7 + 1) {
							std::map<int, int>::iterator it = promotedPawns.find(lastPieceMoved);
							promotedPawns.erase(it);
							f[lastPieceMoved].setTextureRect(IntRect(5 * size, color * size, size, size));
						}

						if (position.length() < 6)
							position.clear();
						else
							position.erase(position.length() - 6, 5);
					}
					loadPosition();
					turn = !turn;
				}
				/////comp move///////
				else if (e.key.code == Keyboard::Space)
				{
					str = getNextMove(toUCI(position));

					oldPos = toCoord(str[0], str[1]);
					newPos = toCoord(str[2], str[3]);

					for (int i = 0; i < 32; i++)
						if (f[i].getPosition() == oldPos) {
							n = i;
							break;
						}

					std::string error = validMove(str.substr(0, 4), n);
					if (error[0] == 'X') // castling
						str += error.substr(1, 4) + ' ';
					else if (error[0] == 'Y') { // promotion
						int piece = std::stoi(error.substr(1));
						int promote = (str[4] == 'r') * 1 + (str[4] == 'n') * 2 + (str[4] == 'b') * 3 + (str[4] == 'q') * 4;
						promotion(piece, promote);
						str += ' ';
					}
					position += str;
					loadPosition();
					turn = !turn;
					std::cout << str;
					if (turn)
						std::cout << std::endl;
					mate();
				}
				else if (e.key.code == Keyboard::Escape)
				{
					window.close();
				}
			}

			/////drag and drop///////
			if (e.type == Event::MouseButtonPressed && e.key.code == Mouse::Left)
			{
				for (int i = 0; i < 32; i++) {
					if (f[i].getGlobalBounds().contains(float(pos.x), float(pos.y))) // checks if mouse is pressed in same square as a piece
					{
						isMove = true;
						n = i;
						dx = pos.x - f[i].getPosition().x;
						dy = pos.y - f[i].getPosition().y;
						oldPos = f[i].getPosition();
						holdingPiece = true;
						break;
					}
				}
			}
			if (e.type == Event::MouseButtonReleased && e.key.code == Mouse::Left && holdingPiece)
			{
				isMove = false;
				Vector2f p = f[n].getPosition() + Vector2f(float(size / 2), float(size / 2));
				newPos = Vector2f(float(size * int(p.x / size)), float(size * int(p.y / size)));

				str = toChessNote(oldPos) + toChessNote(newPos);

				if (p.x < 0 || p.x > 1344 || p.y < 0 || p.y > 1344 || newPos == oldPos) { // out of bounds or same square
					f[n].setPosition(oldPos);
				}
				else {
					std::string error = validMove(str, n);

					if (error == "" || error[0] == 'X' || error[0] == 'Y') { // valid move, castling, promotion
						position += str + " ";
						move(str);
						if (error[0] == 'X') { // castling
							position += error.substr(1, 4) + " ";
							move(error.substr(1, 4));
						}
						else if (error[0] == 'Y') { // promotion
							int piece = std::stoi(error.substr(1));
							promotion(piece, 0);
						}
						f[n].setPosition(newPos);
						turn = !turn;
						std::cout << str << ' ';
						if (turn)
							std::cout << std::endl;
						mate();
					}
					else {
						f[n].setPosition(oldPos);
						std::cout << error << std::endl;
					}
				}
				holdingPiece = false;
			}
		}
		if (isMove) f[n].setPosition(pos.x - dx, pos.y - dy);

		////// draw  ///////
		window.clear();
		window.draw(sBoard);
		for (int i = 0; i < 32; i++) f[i].move(offset);
		for (int i = 0; i < 32; i++) window.draw(f[i]); //window.draw(f[n]);
		for (int i = 0; i < 32; i++) f[i].move(-offset);
		window.display();
	}
	CloseConnection();

	return 0;
}