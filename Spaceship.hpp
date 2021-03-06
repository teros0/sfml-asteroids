#ifndef _SPACESHIP_HPP_
#define _SPACESHIP_HPP_

#include <math.h>
#include <SFML/Graphics.hpp>
#include "Constants.hpp"

class Spaceship: public sf::Drawable, public sf::Transformable {
    static float acceleration;
    static float max_speed;
    static float rotation_speed;

    public:
        Spaceship();
        ~Spaceship();

        void reset();
        void update(float frametime);
        void draw(sf::RenderTarget& target, sf::RenderStates states) const;
        void onEvent(const sf::Event& event);

        void addScore(int);
        int  getScore();
        void setDefault();
        void speedBonus();

        int lives;
        int getLives();
        void increaseLives();
        void decreaseLives();

    private:

        sf::Vector2f speed;
        sf::ConvexShape shape;
        int h_move, v_move;
        int score;
        int stopSpeed;
};
#endif
