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
    int battleship = 1;
    int cruiser = 2;
    int destroyer = 3;
    int submarine = 4;
    std::vector<std::vector<coordinates>> ships;
    char shots_taken[100];
}player_t;

class Schiffe_versenken {
    player_t player1;
    player_t player2;
    std::mutex mutex;
    bool player1sTurn = true;
    bool player1Ready = false;
    bool player2Ready = false;
public:
    void initialize();
    int setShip(int space, bool horizontal, bool player, int shiptype);
    int makeMove(int space, bool player);
    void deleteShips(bool player);
};


#endif //SCHIFFE_VERSENKEN_SCHIFFE_VERSENKEN_HPP
