#pragma once

#include "piece.hpp"

enum class MoveType {Move, Attack};

struct MoveData { // Make constructor that takes a move and pushes it back into moves, add moves for castling and en passant
	std::vector<std::pair<sf::Vector2i, sf::Vector2i>> moves;

	bool valid = false;
	std::vector<std::pair<Color, PieceType>> castle;
	sf::Vector2i enPassant = sf::Vector2i(8, 8);
	bool halfmove = false;

	std::string error;

	MoveData(std::pair<sf::Vector2i, sf::Vector2i> move) { moves.push_back(move); }
};

class State {
public:
	State();
	//State(State*);

	std::array<std::array<Piece, 8>, 8> getSquares() const;
	std::string toFEN() const;

	bool updateState(std::pair<sf::Vector2i, sf::Vector2i>);
	bool updateState(std::string);
private:
	void directionalMoves(std::vector<sf::Vector2i>&, sf::Vector2i, std::vector<sf::Vector2i>) const;
	void positionalMoves(std::vector<sf::Vector2i>&, sf::Vector2i, std::vector<sf::Vector2i>) const;
	void getMoves(std::vector<sf::Vector2i>&, sf::Vector2i, MoveType) const;
	std::vector<sf::Vector2i> findAttackedSquares() const;
	void promote(sf::Vector2i);
	bool check() const;
	bool mate() const;
	MoveData validMove(std::pair<sf::Vector2i, sf::Vector2i>) const;

	// FEN Components + threefold repetition
	std::array<std::array<Piece, 8>, 8> squares;
	Color turn;
	std::map<Color, std::map<PieceType, bool>> canCastle;
	sf::Vector2i enPassant;
	int halfmove; // 50 move rule
	int fullmove;
	int repetition;

	State* prevState;
};