#include "piece.hpp"

Piece::Piece() {
	pieceType = PieceType::None;
	color = Color::None;
}

Piece::Piece(PieceType pieceType, Color color) {
	Piece::pieceType = pieceType;
	Piece::color = color;
}

PieceType Piece::getPieceType() const {
	return pieceType;
}

Color Piece::getColor() const {
	return color;
}

void Piece::draw(sf::RenderWindow& window, sf::Vector2f position) {
	if (color != Color::None && pieceType != PieceType::None) {
		ptrdiff_t c = std::distance(colorOrder.begin(), std::find(colorOrder.begin(), colorOrder.end(), color));
		ptrdiff_t p = std::distance(pieceOrder.begin(), std::find(pieceOrder.begin(), pieceOrder.end(), pieceType));

		sf::Sprite currentPiece(figures, sf::IntRect(p * figuresSquareSize.x, c * figuresSquareSize.y, figuresSquareSize.x, figuresSquareSize.y));
		currentPiece.setPosition(position);
		window.draw(currentPiece);
	}
}

void Piece::replace(Piece newPiece) {
	pieceType = newPiece.pieceType;
	color = newPiece.color;
}

void Piece::remove() {
	pieceType = PieceType::None;
	color = Color::None;
}

void Piece::setFigures(const std::string figuresPath, sf::Vector2f squareSize) {
	std::map<PieceType, sf::Sprite> pieceTypeSpriteMap;

	figures.loadFromFile(figuresPath);
	figuresSquareSize = sf::Vector2f(figures.getSize().x / 6.f, figures.getSize().y / 2.f);
	figuresScale = sf::Vector2f(squareSize.x / figuresSquareSize.x, squareSize.y / figuresSquareSize.y);

	colorOrder = { Color::Black, Color::White }; // Image must match this order
	pieceOrder = { PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen, PieceType::King, PieceType::Pawn }; // Image must match this order
}

sf::Texture Piece::figures;
sf::Vector2f Piece::figuresSquareSize;
sf::Vector2f Piece::figuresScale;
std::array<Color, 2> Piece::colorOrder;
std::array<PieceType, 6> Piece::pieceOrder;