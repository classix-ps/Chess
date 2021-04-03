#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <map>

enum class Color {White, Black, None};
enum class PieceType {Pawn, Knight, Bishop, Rook, Queen, King, None};

class Piece {
public:
	Piece();
	Piece(PieceType, Color);

	PieceType getPieceType() const;
	Color getColor() const;

	void draw(sf::RenderWindow&, sf::Vector2f);
	void replace(Piece);
	void remove();

	static void setFigures(const std::string, sf::Vector2f);
private:
	PieceType pieceType;
	Color color;

	static sf::Texture figures;
	static sf::Vector2f figuresSquareSize;
	static sf::Vector2f figuresScale;
	static std::array<Color, 2> colorOrder;
	static std::array<PieceType, 6> pieceOrder;
};