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
    if ((x + shipType - 1 > 9 && horizontal) || (y + shipType - 1 > 9 && !horizontal) || shipCount == 0 || !valid) {
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
            if (!onePlayerReady) {
                onePlayerReady = true;
                rc = 2;
            } else {
                rc = 3;
            }
            mutex.unlock();
        } else {
            rc = 1;
        }
    }
    for (int i = 0; i < currentPlayer -> ships.size(); i++) {
        for (int j = 0; j < currentPlayer -> ships[i].size(); j++) {
            std::cout << currentPlayer -> ships[i][j].x << " " << currentPlayer -> ships[i][j].y << std::endl;
        }
        std::cout << std::endl;
    }
    return rc;
}