#include "GameStateGame.h"
#include "SFML/Graphics.hpp"
#include "QDebug"
#include "iostream"
#include "windows.h"
#include "math.h"

#include "GameState.hpp"


GameStateGame::GameStateGame(Game *game){

    this->game = game;
    sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
    this->view.setSize(pos);
    pos *= 0.5f;
    this->view.setCenter(pos);

    font.loadFromFile("Media/font.ttf");
    textString.setFont(font);
    textString.setCharacterSize(20);
    textString.setColor(sf::Color::Black);

    gameOver = false;


    level = 1;
    asteroidCount = 0;
    createNewWave();
    this->game->gamePause = false;

    ship.setDefault();
    //bonusAster.;
}

void GameStateGame::createNewWave(){
    for (int i=0; i < level + 2; i++) {
        Asteroid a(0);
        asteroids.push_back(a);
    }
    setBonus(rand()%3);
}

void GameStateGame::draw(const float dt) {
    this->game->window.clear(sf::Color::Black);
    this->game->window.draw(this->game->background);

    this->game->window.draw(ship);

    for(std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it)
        this->game->window.draw(*it);

    for(std::vector<Asteroid>::iterator it = asteroids.begin(); it != asteroids.end(); ++it)
        this->game->window.draw(*it);

    for(std::vector<Bonus>::iterator it = bonuses.begin(); it != bonuses.end(); ++it)
        this->game->window.draw(*it);

    setTextForString("score " + std::to_string(ship.getScore()), sf::Vector2f(50,50));
    setTextForString("level " + std::to_string(level), sf::Vector2f(50,70));
    setTextForString("lives " + std::to_string(ship.getLives()), sf::Vector2f(50,90));

    return;
}

void GameStateGame::update(float frametime) {
    if(gameOver)
        return;

    ship.update(frametime);

    std::vector<Bullet>::iterator start_bullets = bullets.begin();
    while (start_bullets != bullets.end()) {
        if (start_bullets->isAlive()) {
            start_bullets->update(frametime);
            ++start_bullets;
        } else
            start_bullets = bullets.erase(start_bullets);
    }

    std::vector<Bonus>::iterator start_bonuses = bonuses.begin();
    while (start_bonuses != bonuses.end()) {
        if (start_bonuses->isAlive()) {
            start_bonuses->update(frametime);
            ++start_bonuses;
        } else
            start_bonuses = bonuses.erase(start_bonuses);
    }

    std::vector<Asteroid>::iterator start_asteroids = asteroids.begin();

    asteroidCount = 0;

    for(; start_asteroids != asteroids.end(); ++start_asteroids){
        if (start_asteroids->isAlive())
            ++asteroidCount;
    }

    if(asteroidCount == 0){
        level++;
        createNewWave();
    }

    start_asteroids = asteroids.begin();


    while (start_asteroids != asteroids.end()) {
        if (start_asteroids->isAlive()) {
            start_asteroids->update(frametime);
            ++start_asteroids;
        } else
            start_asteroids = asteroids.erase(start_asteroids);
    }

    std::vector<Asteroid> new_asteroids;
    start_asteroids = asteroids.begin();

    //player - asteroids collision
    while(start_asteroids != asteroids.end()){
    if(start_asteroids->checkPoint(ship.getPosition())){
        ship.decreaseLives();
        ship.reset();
    }
    ++start_asteroids;
    }

    start_bonuses = bonuses.begin();

    //player - bonus collision
    while(start_bonuses != bonuses.end()){
    if(start_bonuses->checkPoint(ship.getPosition())){
        switch (start_bonuses->getType()){
            case 1:
            start_bonuses->life(ship);
            start_bonuses->setUnactive();
            break;
            case 2:
            start_bonuses->destroy(asteroids, ship, asteroidCount);
            start_bonuses->setUnactive();
            break;
            case 3:
            start_bonuses->speedy(ship);
            start_bonuses->setUnactive();
            break;
        }
    }
    ++start_bonuses;
    }


    if(ship.getLives() <= 0){
        this->game->gamePause = true;
        Sleep(5);
        endGame();
    }

    start_asteroids = asteroids.begin();

    while (start_asteroids != asteroids.end()) {

        start_bullets = bullets.begin();
        while (start_bullets != bullets.end()) {
            if (!start_bullets->isAlive()) {
                ++start_bullets;
                continue;
            }

            if(start_asteroids->checkPoint(ship.getPosition())){
                ship.lives -= 1;
                ship.reset();
            }

            //check for collision
            if (start_asteroids->checkPoint(start_bullets->getPosition())){
                start_bullets->kill();
                start_asteroids->breakDown();
                ship.addScore(50);
                --asteroidCount;


                if (start_asteroids->isAlive()) {
                    sf::Vector2f position = start_asteroids->getPosition();
                    float angle = rand() % 360;
                    Asteroid a(position, angle, start_asteroids->getLevel());
                    new_asteroids.push_back(a);
                }
                break;
            }
            ++start_bullets;
        }
        ++start_asteroids;
        ++start_bullets;
    }
    asteroids.insert(asteroids.end(), new_asteroids.begin(), new_asteroids.end());
}

void GameStateGame::handleInput() {
    sf::Event event;

    while(this->game->window.pollEvent(event))
    {
        ship.onEvent(event);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            Bullet bullet(ship.getPosition(), ship.getRotation());
            bullets.push_back(bullet);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
            this->game->gamePause = true;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::U)) {
            this->game->gamePause = false;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            this->game->pushState(new GameStateEditor(this->game));
        }


        switch(event.type)
        {
        case sf::Event::Closed:
        {
            this->game->window.close();
            break;
        }
        case sf::Event::Resized:
        {
            view.setSize(event.size.width, event.size.height);
            this->game->background.setPosition(this->game->window.mapPixelToCoords(sf::Vector2i(0, 0), this->view));
            this->game->background.setScale(
                  float(event.size.width) / float(this->game->background.getTexture()->getSize().x),
                  float(event.size.height) / float(this->game->background.getTexture()->getSize().y));
            break;
         }
         default: break;

        }
    }

    return;
}

void GameStateGame::endGame(){
    this->game->pushState(new GameStateEnd(this->game, ship.getScore()));
    return;
}

void GameStateGame::setTextForString(std::string text, sf::Vector2f pos){
    textString.setColor(sf::Color::White);
    textString.setString(text);
    textString.setPosition(pos);
    this->game->window.draw(textString);
}

void GameStateGame::setBonus(int randNum){

    if(randNum == 0){
        Bonus a(1);
        bonuses.push_back(a);}
    else if(randNum == 1){
        Bonus b(2);
        bonuses.push_back(b);}
    else if(randNum == 2){
        Bonus c(3);
        bonuses.push_back(c);}
}
