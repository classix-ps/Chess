#pragma once

#include "board.hpp"
#include <windows.h>

class Button {

};

class Game {
public:
	Game(LPCWSTR, std::string, int, sf::Vector2i, std::string, std::string);

	void play();
private:
	std::atomic_bool game;

	Board board;

	int windowSize;
	LPCWSTR enginePath;

	sf::Texture menu;
	sf::Sprite menuSprite;
	std::vector<Button> menuButtons;
	std::vector<Button> gameButtons;

	Moving mouse;
	void handleGameplay();
	void inGame(sf::RenderWindow&);
	void inMenu(sf::RenderWindow&);
	void handleWindow();
};