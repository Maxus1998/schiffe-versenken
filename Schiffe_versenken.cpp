//
// Created by Julian on 29.12.2018.
//

#include <vector>
#include <mutex>
#include <iostream>
#include "Schiffe_versenken.hpp"

/*
 * return value:
 * 0: ship can´t be set
 * 1: ship was set
 * 2: ship was set and the player who set it is done
 * 3: ship was set and both players are done
 * shipType:
 * 2: submarine
 * 3: destroyer
 * 4: cruiser
 * 5: battleship
 */

void Schiffe_versenken::initialize() {
    for (int i = 0; i < 100; i++) {
        player1.shots_taken[i] = 0;
        player2.shots_taken[i] = 0;
    }
}

int Schiffe_versenken::setShip(int space, bool horizontal, bool player, int shipType) {
    player_t* currentPlayer;
    if (player) {
        currentPlayer = &player1;
    } else {
        currentPlayer = &player2;
    }
    int shipCount;
    switch(shipType) {
        case 2: shipCount = currentPlayer -> submarine;
                break;
        case 3: shipCount = currentPlayer -> destroyer;
                break;
        case 4: shipCount = currentPlayer -> cruiser;
                break;
        case 5: shipCount = currentPlayer -> battleship;
                break;
    }
    int x = space % 10;
    int y = space / 10;
    int rc;
    bool valid = true;
    for (int i = 0; i < currentPlayer -> ships.size(); i++) {
        for (int j = 0; j < currentPlayer -> ships[i].size(); j++) {
            for (int k = 0; k < shipType; k++) {
                if ((horizontal && currentPlayer -> ships[i][j].x == x + k && currentPlayer -> ships[i][j].y == y)
                  ||(!horizontal && currentPlayer -> ships[i][j].x == x && currentPlayer -> ships[i][j].y + k == y)) {
                    valid = false;
                }
            }
        }
    }
    if ((x + shipType - 1 > 9 && horizontal) || (y + shipType - 1 > 9 && !horizontal) || shipCount == 0 || !valid || space < 0 || space > 99) {
        rc = 0;
    } else {
        std::vector<coordinates_t> ship;
        for (int i = 0; i < shipType; i++) {
            coordinates_t coordinates;
            if (horizontal) {
                coordinates.x = x + i;
                coordinates.y = y;
            } else {
                coordinates.x = x;
                coordinates.y = y + i;
            }
            ship.push_back(coordinates);
        }
        currentPlayer -> ships.push_back(ship);
        switch(shipType) {
            case 2: currentPlayer -> submarine--;
                break;
            case 3: currentPlayer -> destroyer--;
                break;
            case 4: currentPlayer -> cruiser--;
                break;
            case 5: currentPlayer -> battleship--;
                break;
        }
        if (currentPlayer -> battleship == 0 && currentPlayer -> cruiser == 0 && currentPlayer -> destroyer == 0 && currentPlayer -> submarine == 0) {
            mutex.lock();
            if (player) {
                player1Ready = true;
            } else {
                player2Ready = true;
            }
            if (player1Ready && player2Ready) {
                rc = 3;
            } else {
                rc = 2;
            }
            mutex.unlock();
        } else {
            rc = 1;
        }
    }
    return rc;
}

/*
 * return value:
 * 0: Move is not valid
 * 1: No hit
 * 2: Ship is hit
 * 3: ship has been sunk
 * 4: ship has been sunk and game is over
 */
int Schiffe_versenken::makeMove(int space, bool player) {
    int rc;
    int x = space % 10;
    int y = space / 10;

    player_t* currentPlayer;
    player_t* enemy;
    if (player) {
        currentPlayer = &player1;
        enemy = &player2;
    } else {
        currentPlayer = &player2;
        enemy = &player1;
    }
    if (player != player1sTurn || space < 0 || space > 99 || currentPlayer -> shots_taken[space] == 1) {
        rc = 0;
    } else {
        bool hit = false;
        int shipIndex;
        int shipPart;
        for (int i = 0; i < enemy -> ships.size(); i++) {
            for (int j = 0; j < enemy -> ships[i].size(); j++) {
                if (enemy -> ships[i][j].x == x && enemy -> ships[i][j].y == y) {
                    hit = true;
                    shipIndex = i;
                    shipPart = j;
                    break;
                }
            }
        }
        if (hit) {
            enemy -> ships[shipIndex].erase(enemy -> ships[shipIndex].begin() + shipPart);
            if (enemy -> ships[shipIndex].empty()) {
                enemy -> ships.erase(enemy -> ships.begin() + shipIndex);
                if (enemy -> ships.empty()) {
                    rc = 4;
                } else {
                    rc = 3;
                }
            } else {
                rc = 2;
            }
        } else {
            rc = 1;
        }
        player1sTurn = !player1sTurn;
        currentPlayer -> shots_taken[space] = 1;
    }
    return rc;
}

void Schiffe_versenken::deleteShips(bool player) {
    player_t* currentPlayer;
    bool* ready;
    if (player) {
        currentPlayer = &player1;
        ready = &player1Ready;
    } else {
        currentPlayer = &player2;
        ready = &player2Ready;
    }
    currentPlayer -> ships.clear();
    currentPlayer -> battleship = 1;
    currentPlayer -> cruiser = 2;
    currentPlayer -> destroyer = 3;
    currentPlayer -> submarine = 4;
    if (*ready) {
        *ready = false;
    }
}