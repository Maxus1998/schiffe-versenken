//
// Created by Julian on 29.12.2018.
//

#ifndef SCHIFFE_VERSENKEN_SCHIFFE_VERSENKEN_HPP
#define SCHIFFE_VERSENKEN_SCHIFFE_VERSENKEN_HPP

#include <mutex>

typedef struct coordinates{
    int x;
    int y;
}coordinates_t;

typedef struct player{
    int battleship = 0;
    int cruiser = 0;
    int destroyer = 0;
    int submarine = 1;
    std::vector<std::vector<coordinates>> ships;
    char shots_taken[100];
}player_t;

class Schiffe_versenken {
    player_t player1;
    player_t player2;
    bool onePlayerReady = false;
    std::mutex mutex;
    bool player1sTurn = true;
public:
    void initialize();
    int setShip(int space, bool horizontal, bool player, int shiptype);
    int makeMove(int space, bool player);
};


#endif //SCHIFFE_VERSENKEN_SCHIFFE_VERSENKEN_HPP
