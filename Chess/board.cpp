#include "board.hpp"

Board::Board(const std::string boardPath, int windowSize, sf::Vector2i origOffset) {
	board.loadFromFile(boardPath);
	sf::Vector2f boardScale(windowSize / float(board.getSize().x), windowSize / float(board.getSize().y));

	boardSprite.setTexture(board);
	boardSprite.setScale(boardScale);

	offset = sf::Vector2i(int(origOffset.x * boardScale.x), int(origOffset.y * boardScale.y));
	squareSize = sf::Vector2f((windowSize - 2 * offset.x) / 8.f, (windowSize - 2 * offset.y) / 8.f);

	boardState = State();
}

sf::Vector2f Board::getSquareSize() const {
	return squareSize;
}

sf::Vector2i Board::getOffset() const {
	return offset;
}

bool Board::inBounds(sf::Vector2i square) const {
	return (square.x < 8 && square.x >= 0 && square.y < 8 && square.y >= 0);
}

sf::Vector2i Board::toSquare(sf::Vector2i pos) const {
	return sf::Vector2i(int((pos.x - offset.x) / squareSize.x), int((pos.y - offset.y) / squareSize.y));
}

sf::Vector2f Board::toPos(sf::Vector2i square) const {
	return sf::Vector2f(offset.x + square.x * squareSize.x, offset.y + square.y * squareSize.y);
}

bool Board::userMove() {
	std::pair<sf::Vector2i, sf::Vector2i> move = std::make_pair(buffer.front().origSquare, toSquare(buffer.front().destMousePos));
	bool valid = boardState.updateState(move);
	buffer.erase(buffer.begin());
	return valid;
}

void Board::computerMove(Engine& engine) {
	std::string FEN = boardState.toFEN();
	std::string move = engine.getNextMove(FEN);
	boardState.updateState(move);
}

void Board::draw(sf::RenderWindow& window, Moving mouse) const {
	window.clear();

	window.draw(boardSprite);
	std::array<std::array<Piece, 8>, 8> squares = boardState.getSquares();
	std::vector<Moving> movement(buffer);
	if (mouse.active)
		movement.push_back(mouse);
	for (size_t file = 0; file < 8; file++) {
		for (size_t rank = 0; rank < 8; rank++) {
			if (std::find_if(movement.begin(), movement.end(), [file, rank](const Moving& m) -> bool { return m.origSquare == sf::Vector2i(file, rank); }) == movement.end()) {
				squares[file][rank].draw(window, toPos(sf::Vector2i(file, rank)));
			}
		}
	}
	for (size_t i = 0; i < movement.size(); i++) {
		sf::Vector2f position = sf::Vector2f(movement[i].destMousePos) - 0.5f * squareSize;
		squares[movement[i].origSquare.x][movement[i].origSquare.y].draw(window, position);
	}

	window.display();
}