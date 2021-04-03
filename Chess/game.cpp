#include "game.hpp"
#include <thread>

Game::Game(LPCWSTR path, std::string boardPath, int size, sf::Vector2i offset, std::string figuresPath, std::string menuPath) : board("../images/boardStone.png", size, offset), windowSize(size), enginePath(path) {
	Piece::setFigures("../images/figuresStone.png", board.getSquareSize());

	menu.loadFromFile(menuPath);
	sf::Vector2f menuScale(windowSize / float(menu.getSize().x), windowSize / float(menu.getSize().y));
	menuSprite.setTexture(menu);
	menuSprite.setScale(menuScale);


}

void Game::handleGameplay() {
	Engine engine(enginePath);
	engine.ConnectToEngine();

	while (game) {
		if (!board.buffer.empty()) {
			if (board.userMove()) {
				Sleep(1000);
				board.computerMove(engine);
			}
		}
	}

	engine.CloseConnection();
}

void Game::inGame(sf::RenderWindow& window) {
	game = true;
	std::thread gameplay(&Game::handleGameplay, this);

	sf::Event e;
	while (window.isOpen()) {
		mouse.destMousePos = sf::Mouse::getPosition(window);
		board.draw(window, mouse);
		while (window.pollEvent(e)) {
			if (e.type == sf::Event::Closed) { // Exit
				window.close();
			}
			else if (e.type == sf::Event::KeyPressed) {
				if (e.key.code == sf::Keyboard::BackSpace) { // Undo

				}
				else if (e.key.code == sf::Keyboard::Escape) { // Menu
					game = false;
					gameplay.join();
					return;
				}
			}
			else if (e.type == sf::Event::MouseButtonPressed && e.key.code == sf::Mouse::Left) {
				mouse.origSquare = board.toSquare(sf::Mouse::getPosition(window));
				mouse.active = true;
			}
			else if (e.type == sf::Event::MouseButtonReleased && e.key.code == sf::Mouse::Left) {
				mouse.destMousePos = sf::Vector2i(board.toPos(board.toSquare(mouse.destMousePos))) + sf::Vector2i(0.5f * board.getSquareSize());
				board.buffer.push_back(mouse);
				mouse.active = false;
			}
		}
	}

	game = false;
	gameplay.join();
}

void Game::inMenu(sf::RenderWindow& window) {
	sf::Event e;
	while (window.isOpen()) {
		window.clear();
		window.draw(menuSprite);
		window.display();
		while (window.pollEvent(e)) {
			if (e.type == sf::Event::Closed) { // Exit
				window.close();
			}
			else if (e.type == sf::Event::MouseButtonPressed && e.key.code == sf::Mouse::Left) {
				return;
			}
		}
	}
}

void Game::handleWindow() {
	sf::RenderWindow window(sf::VideoMode(windowSize, windowSize), "Chess");
	while (window.isOpen()) {
		inMenu(window);
		inGame(window);
	}
}

void Game::play() {
	handleWindow();
}