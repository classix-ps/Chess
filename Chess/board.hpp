#pragma once

#include "rules.hpp"
#include "connector.hpp"

struct Moving {
	bool active;
	sf::Vector2i origSquare;
	sf::Vector2i destMousePos;
};

class Board {
public:
	Board(const std::string boardPath, int, sf::Vector2i);

	sf::Vector2f getSquareSize() const;
	sf::Vector2i getOffset() const;

	sf::Vector2i toSquare(sf::Vector2i) const;
	sf::Vector2f toPos(sf::Vector2i) const;

	std::vector<Moving> buffer; // atomic

	bool userMove();
	void computerMove(Engine&);

	void draw(sf::RenderWindow&, Moving) const;
private:
	bool inBounds(sf::Vector2i) const;

	State boardState;

	sf::Vector2f squareSize;
	sf::Vector2i offset;

	sf::Texture board;
	sf::Sprite boardSprite;
};