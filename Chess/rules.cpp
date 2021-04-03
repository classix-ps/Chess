#include "rules.hpp"

State::State() {
	std::array<Color, 2> colorOrder = { Color::Black, Color::White };
	std::array<PieceType, 8> pieceOrder = { PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen, PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook };
	for (size_t i = 0; i < colorOrder.size(); i++) {
		int rank = 5 * i + 1;
		for (size_t file = 0; file < 8; file++) {
			squares[file][rank] = Piece(PieceType::Pawn, colorOrder[i]);
		}
	}
	for (size_t i = 0; i < colorOrder.size(); i++) {
		int rank = 7 * i;
		for (size_t file = 0; file < 8; file++) {
			squares[file][rank] = Piece(pieceOrder[file], colorOrder[i]);
		}
	}
	for (size_t rank = 2; rank < 6; rank++) {
		for (size_t file = 0; file < 8; file++) {
			squares[file][rank] = Piece();
		}
	}
	turn = Color::White;

	canCastle = { {Color::Black, { {PieceType::King, true}, {PieceType::Queen, true} } }, {Color::White, { {PieceType::King, true}, {PieceType::Queen, true} } } };
	enPassant = sf::Vector2i(8, 8);
	halfmove = 0;
	fullmove = 1;
	repetition = 0;

	prevState = nullptr;
}

std::array<std::array<Piece, 8>, 8> State::getSquares() const {
	return squares;
}

bool inBounds(sf::Vector2i pos) {
	return (pos.x < 8 && pos.x >= 0 && pos.y < 8 && pos.y >= 0);
}

void addSquareFEN(std::string& FEN, Piece piece) {
	PieceType t = piece.getPieceType();
	if (t == PieceType::None) {
		if (isdigit(FEN.back())) {
			FEN.back() += 1;
		}
		else {
			FEN += '1';
		}
	}
	else {
		char notation = 'K' * (t == PieceType::King) + 'Q' * (t == PieceType::Queen) + 'R' * (t == PieceType::Rook) + 'B' * (t == PieceType::Bishop) + 'N' * (t == PieceType::Knight) + 'P' * (t == PieceType::Pawn);
		notation += 32 * (piece.getColor() == Color::Black);
		FEN += notation;
	}
}

std::string squareToNotation(sf::Vector2i square) {
	char file = 97 + square.x;
	char rank = 49 + 8 - square.y;
	return std::string({ file, rank });
}

sf::Vector2i notationToSquare(std::string notation) {
	int file = notation[0] - 97;
	int rank = 7 - (notation[1] - 49);
	return sf::Vector2i(file, rank);
}

std::string State::toFEN() const {
	std::string FEN;

	for (size_t rank = 0; rank < 8; rank++) {
		for (size_t file = 0; file < 8; file++) {
			addSquareFEN(FEN, squares[file][rank]);
		}
		FEN += '/';
	}

	FEN += turn == Color::Black ? " b " : " w ";

	if (canCastle.at(Color::White).at(PieceType::King))
		FEN += 'K';
	if (canCastle.at(Color::White).at(PieceType::Queen))
		FEN += 'Q';
	if (canCastle.at(Color::Black).at(PieceType::King))
		FEN += 'k';
	if (canCastle.at(Color::Black).at(PieceType::Queen))
		FEN += 'q';

	if (inBounds(enPassant)) {
		FEN += ' ' + squareToNotation(enPassant);
	}
	else {
		FEN += " -";
	}

	FEN += ' ' + std::to_string(halfmove);

	FEN += ' ' + std::to_string(fullmove);

	return FEN;
}

void State::directionalMoves(std::vector<sf::Vector2i>& moves, sf::Vector2i square, std::vector<sf::Vector2i> directions) const {
	Color color = squares[square.x][square.y].getColor();
	for (const sf::Vector2i& direction : directions) {
		for (sf::Vector2i pos = square + direction; inBounds(pos); pos += direction) {
			Color currentColor = squares[pos.x][pos.y].getColor();
			if (currentColor == Color::None) {
				moves.push_back(pos);
			}
			else {
				if (currentColor != color) {
					moves.push_back(pos);
				}
				break;
			}
		}
	}
}

void State::positionalMoves(std::vector<sf::Vector2i>& moves, sf::Vector2i square, std::vector<sf::Vector2i> positions) const {
	Color color = squares[square.x][square.y].getColor();
	for (const sf::Vector2i& position : positions) {
		sf::Vector2i pos = square + position;
		if (inBounds(pos)) {
			if (squares[pos.x][pos.y].getColor() != color) {
				moves.push_back(pos);
			}
		}
	}
}

void State::getMoves(std::vector<sf::Vector2i>& moves, sf::Vector2i square, MoveType moveType = MoveType::Move) const {
	switch (squares[square.x][square.y].getPieceType()) {
	case PieceType::King:
		positionalMoves(moves, square, { {-1, 1}, {-1, 0}, {-1, -1}, {0, 1}, {0, -1}, {1, 1}, {1, 0}, {1, -1} });
		break;
	case PieceType::Queen:
		directionalMoves(moves, square, { {-1, 1}, {-1, -1}, {1, 1}, {1, -1}, {-1, 0}, {1, 0}, {0, 1}, {0, -1} });
		break;
	case PieceType::Rook:
		directionalMoves(moves, square, { {-1, 0}, {1, 0}, {0, 1}, {0, -1} });
		break;
	case PieceType::Bishop:
		directionalMoves(moves, square, { {-1, 1}, {-1, -1}, {1, 1}, {1, -1} });
		break;
	case PieceType::Knight:
		positionalMoves(moves, square, { {-2, 1}, {-2, -1}, {-1, 2}, {-1, -2}, {1, 2}, {1, -2}, {2, 1}, {2, -1} });
		break;
	case PieceType::Pawn:
		Color color = squares[square.x][square.y].getColor();
		Color enemyColor = color == Color::White ? Color::Black : Color::White;
		switch (moveType) {
		case MoveType::Move: {
			square.y += color == Color::Black ? 1 : -1;
			if (squares[square.x][square.y].getColor() == Color::None) {
				moves.push_back(square);
			}
			std::vector<sf::Vector2i> attackSquares = { square + sf::Vector2i(-1, 0), square + sf::Vector2i(1, 0) };
			for (const sf::Vector2i& attackSquare : attackSquares) {
				if (inBounds(attackSquare) && squares[attackSquare.x][attackSquare.y].getColor() == enemyColor || attackSquare == enPassant) {
					moves.push_back(attackSquare);
				}
			}
			break;
		}
		case MoveType::Attack:
			std::vector<sf::Vector2i> positions = color == Color::Black ? std::vector<sf::Vector2i>{ sf::Vector2i(-1, 1), sf::Vector2i(1, 1) } : std::vector<sf::Vector2i>{ sf::Vector2i(-1, -1), sf::Vector2i(1, -1) };
			positionalMoves(moves, square, positions);
			break;
		}
		break;
	}
}

std::vector<sf::Vector2i> State::findAttackedSquares() const {
	std::vector<sf::Vector2i> attacked;

	Color enemyColor = turn == Color::White ? Color::Black : Color::White;

	for (size_t file = 0; file < 8; file++) {
		for (size_t rank = 0; rank < 8; rank++) {
			if (squares[file][rank].getColor() == enemyColor) {
				getMoves(attacked, sf::Vector2i(file, rank), MoveType::Attack);
			}
		}
	}

	return attacked;
}

void State::promote(sf::Vector2i square) {

}

bool State::check() const {
	std::vector<sf::Vector2i> isAttacked = findAttackedSquares();
	for (const sf::Vector2i& attacked : isAttacked) {
		if (squares[attacked.x][attacked.y].getPieceType() == PieceType::King) {
			return true;
		}
	}
	return false;
}

bool State::mate() const { // if the king is attacked no matter what move is made, checkmate (if he is not attacked in current state, stalemate)
	return false;
}

MoveData State::validMove(std::pair<sf::Vector2i, sf::Vector2i> move) const {
	MoveData moveData(move);

	sf::Vector2i s1(move.first);
	sf::Vector2i s2(move.second);

	Color color = squares[s1.x][s1.y].getColor();
	PieceType pieceType = squares[s1.x][s1.y].getPieceType();
	Color enemyColor = turn == Color::White ? Color::Black : Color::White;

	if (color == Color::None) { // No piece on square
		return moveData;
	}
	if (color != turn) { // Wrong colored piece moved
		moveData.error = "It's not your turn!";
		return moveData;
	}

	std::vector<sf::Vector2i> standardMoves;
	getMoves(standardMoves, s1);
	if (std::find(standardMoves.begin(), standardMoves.end(), s2) == standardMoves.end()) { // Not among standard moves or invalid
		if (pieceType == PieceType::King) {
			int rank = turn == Color::Black ? 0 : 7;
			PieceType side = s2.x - s1.x == 2 ? PieceType::King : PieceType::Queen;
			if (s1 == sf::Vector2i(4, rank) && sf::Vector2i(abs(s2.x - s1.x), s2.y - s1.y) == sf::Vector2i(2, 0)) { // Castling
				if (!canCastle.at(turn).at(side)) { // Can't castle anymore
					moveData.error = "You are no longer able to castle this way!";
					return moveData;
				}
				std::vector<sf::Vector2i> attacked = findAttackedSquares();
				for (sf::Vector2i currentSquare = s1; currentSquare.x % 7; currentSquare.x += (s2.x - s1.x) / 2) {
					if (abs(currentSquare.x - 4) < 2 && std::find(attacked.begin(), attacked.end(), currentSquare) != attacked.end()) {
						moveData.error = "Can't castle through check!";
						return moveData;
					}
					if (currentSquare.x % 4 && squares[currentSquare.x][currentSquare.y].getColor() != Color::None) {
						moveData.error = "Can't castle through piece!";
						return moveData;
					}
				}
				moveData.moves.push_back(std::make_pair(sf::Vector2i(side == PieceType::King ? 7 : 0, rank), sf::Vector2i((s1.x + s2.x) / 2, rank)));
				moveData.castle.insert(moveData.castle.end(), { std::make_pair(turn, PieceType::King), std::make_pair(turn, PieceType::Queen) });
			}
			else {
				return moveData;
			}
		}
		else if (pieceType == PieceType::Pawn) {
			int dir = turn == Color::Black ? 1 : -1;
			if (s2 == s1 + sf::Vector2i(0, 2 * dir) && s1.y == (7 - 5 * dir) / 2) { // First
				for (sf::Vector2i currentSquare = s1 + sf::Vector2i(0, dir); currentSquare.y != s2.y + dir; currentSquare.y += dir) {
					if (squares[currentSquare.x][currentSquare.y].getColor() != Color::None) {
						moveData.error = "Pawn can't advance two squares through a piece!";
						return moveData;
					}
				}
				moveData.enPassant = s1 + sf::Vector2i(0, dir);
			}
			else {
				return moveData;
			}
		}
		else {
			return moveData;
		}
	}

	if (pieceType == PieceType::Rook) {
		int rank = turn == Color::Black ? 0 : 7;
		if (s1 == sf::Vector2i(7, rank)) {
			moveData.castle.push_back(std::make_pair(turn, PieceType::King));
		}
		else if (s1 == sf::Vector2i(0, rank)) {
			moveData.castle.push_back(std::make_pair(turn, PieceType::Queen));
		}
	}

	if (pieceType == PieceType::Pawn) {
		if (s2 == enPassant) {
			int dir = turn == Color::Black ? 1 : -1;
			moveData.moves.push_back(std::make_pair(enPassant - sf::Vector2i(0, dir), enPassant - sf::Vector2i(0, dir)));
		}
	}

	if (squares[s2.x][s2.y].getColor() == enemyColor || pieceType == PieceType::Pawn) {
		moveData.halfmove = true;
	}

	if (check()) {
		moveData.error = "Move would leave you in check!";
		return moveData;
	}

	moveData.valid = true;
	return moveData;
}

bool State::updateState(std::pair<sf::Vector2i, sf::Vector2i> move) {
	MoveData moveData = validMove(move);

	if (moveData.valid) { // Update state
		for (std::pair<sf::Vector2i, sf::Vector2i>& m : moveData.moves) {
			squares[m.second.x][m.second.y].replace(squares[m.first.x][m.first.y]);
			squares[m.first.x][m.first.y].remove();
		}

		for (const std::pair<Color, PieceType>& castling : moveData.castle) {
			canCastle.at(castling.first).at(castling.second) = false;
		}

		enPassant = moveData.enPassant;

		halfmove = moveData.halfmove ? 0 : halfmove + 1; // Reset if capture or pawn move, increment otherwise

		fullmove += (turn == Color::Black); // Increment after black's turn
		turn = turn == Color::White ? Color::Black : Color::White;
	}

	return moveData.valid;
}
bool State::updateState(std::string move) {
	std::pair<sf::Vector2i, sf::Vector2i> moveSq = std::make_pair(notationToSquare(move.substr(0, 2)), notationToSquare(move.substr(2, 2)));
	return updateState(moveSq);
}