//
// client.cpp
// ~~~~~~~~~~~~~~~
//
// This has been made using the official boost example for a chat server application.
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "message.hpp"
#include <chrono>
#include <thread>

using boost::asio::ip::tcp;

typedef std::deque<message> message_queue;

const int defaultTimeout = -1;

class session
{
private:
    std::vector<std::string> openGames;
    std::vector<std::vector<int>> ships;
    std::vector<std::vector<int>> shots;
    int whoIAm;
    char* playerName;
    char* opponentName;

public:
    session()
    {
        ships.resize(10);
        for ( int i = 0 ; i < ships.size(); i++ )
        {
            ships[i].resize(ships.size());
        }
        shots.resize(10);
        for ( int i = 0 ; i < ships.size(); i++ )
        {
            shots[i].resize(shots.size());
            for(int j = 0; j < 10; j++)
            {
                shots[i][j] = 0;
            }
        }
    }

    void setWhoIAm(int who)
    {
        whoIAm = who;
    }
    int getWhoIAm()
    {
        return whoIAm;
    }

    /**
     *
     * @param row
     * @param column
     * @param direction
     * @param type
     * @return
     */
    bool setShip(char row, int column, int direction, char type)
    {
        int rowNumber = int(row) - 97;
        if(rowNumber < -32 || rowNumber > 9)
        {
            std::cout << "The ship can not be placed at this row (" << rowNumber << ")" << std::endl;
            return false;
        }
        if(rowNumber < 0) rowNumber = rowNumber + 32;

        if(column < 0 || column > 9)
        {
            std::cout << "The ship can not be placed at this column (" << column << ")" << std::endl;
            return false;
        }

        int shipLength = 0;
        switch(type)
        {
            case 's':
                shipLength = 2;
                break;
            case 'd':
                shipLength = 3;
                break;
            case 'c':
                shipLength = 4;
                break;
            case 'b':
                shipLength = 5;
                break;
        }
        for (int i = 0; i < shipLength - 1; i++) {
            if (direction == 0) {
                if (ships[rowNumber + i][column] != 0) {
                    std::cout << "Another ship already occupies at least one of the spaces" << std::endl;
                    return false;
                }
            } else {
                if (ships[rowNumber][column + i]) {
                    std::cout << "Another ship already occupies at least one of the spaces" << std::endl;
                    return false;
                }
            }
        }
        if(direction == 0)      //vertikal
        {
            if(shipLength > 10-rowNumber)
            {
                std::cout << "Ship is too long for this position";
                return false;
            }
            for(int i = 0; i < shipLength; i++)
            {
//                std::cout << rowNumber-i << " - " << column << std::endl;
                ships[rowNumber+i][column] = type;
            }
        }
        else if(direction == 1) //horizontal
        {
            if(shipLength > 10-column)
            {
                std::cout << "Ship is too long for this position";
                return false;
            }
            for(int i = 0; i < shipLength; i++)
            {
//                std::cout << rowNumber << " - " << column+i << std::endl;
                ships[rowNumber][column+i] = type;
            }
        }
        std::cout << "ship set" << std::endl;
        return true;



    }
    void unsetShipAt(char row, int column, int direction, char type)
    {
        int rowNumber = int(row) - 97;
        if(rowNumber < -32 || rowNumber > 9)
        {
            return;
        }
        if(rowNumber < 0) rowNumber = rowNumber + 32;

        if(column < 0 || column > 9)
        {
            return;
        }

        int shipLength = 0;
        switch(type)
        {
            case 's':
                shipLength = 2;
                break;
            case 'd':
                shipLength = 3;
                break;
            case 'c':
                shipLength = 4;
                break;
            case 'b':
                shipLength = 5;
                break;
        }
        if(direction == 0)      //vertikal
        {
            if(shipLength > 10-rowNumber)
            {
                return;
            }
            for(int i = 0; i < shipLength; i++)
            {
//                std::cout << rowNumber-i << " - " << column << std::endl;
                ships[rowNumber+i][column] = ' ';
            }
        }
        else if(direction == 1) //horizontal
        {
            if(shipLength > 10-column)
            {
                return;
            }
            for(int i = 0; i < shipLength; i++)
            {
//                std::cout << rowNumber << " - " << column+i << std::endl;
                ships[rowNumber][column+i] = ' ';
            }
        }
        return;
    }
    char getShipAt(int row, int column)
    {
        return ships[row][column];
    }

    void setShotAt(int position)
    {
        int row;
        int column;

        column = position % 10;
        row = position / 10;
        std::cout << "Shot at "<< char(row+65) << "|" << column << std::endl;
        if(getShipAt(row,column) != ' ')
        {
            std::cout << getShipAt(row,column) << " hit" << std::endl;
            ships[row][column] = 'X';
        }
    }
    bool setShotAt(char row, int column, int hit)
    {
        int rowNumber = int(row) - 97;
        if(rowNumber < -32 || rowNumber > 9)
        {
            std::cout << "You can not shoot at this row (" << rowNumber << ")" << std::endl;
            return false;
        }
        if(rowNumber < 0) rowNumber = rowNumber + 32;

        if(column < 0 || column > 9)
        {
            std::cout << "You can not shoot at this column (" << column << ")" << std::endl;
            return false;
        }
        if(hit >= 1)
        {
            shots[rowNumber][column] = 1;
        }
        else if(hit == 0)
        {
            shots[rowNumber][column] = 2;
        }
    }
    int getShotAt(int row, int column)
    {
        return shots[row][column];
    }

    void addGameToList(char* gameName)
    {
        std::string nameString(gameName);
        openGames.push_back(nameString);
    }
    void removeGameFromList(char* gameName)
    {
        std::cout << "Currently it is not possible to delete a game";
    }
    void forgetAllGames()
    {
        openGames.clear();
    }
    std::vector<std::string> getGameList()
    {
        return openGames;
    }

    void setPlayerName(char* name)
    {
        playerName = name;
    }
    char* getPlayerName()
    {
        return playerName;
    }

    void setOpponentName(char* name)
    {
        opponentName = name;
    }
    char* getOpponentName()
    {
        return opponentName;
    }
};

class client
{
public:
    client(
            boost::asio::io_context& io_context
            , const tcp::resolver::results_type& endpoints
            , session *gameSession
            ): io_context_(io_context),socket_(io_context)
    {
        runningGame = gameSession;
        do_connect(endpoints);
    }
    bool messageReceived = false;
    int messageType = 0;
    char* msgBody;

    void write(const message& msg)
    {
        boost::asio::post(io_context_,
                          [this, msg]()
                          {
                              bool write_in_progress = !write_msgs_.empty();
                              write_msgs_.push_back(msg);
                              if (!write_in_progress)
                              {
                                  do_write();
                              }
                          });
    }

    void close()
    {
        boost::asio::post(io_context_, [this]() { socket_.close(); });
    }

private:
    void do_connect(const tcp::resolver::results_type& endpoints)
    {
        boost::asio::async_connect(socket_, endpoints,
                                   [this](boost::system::error_code ec, tcp::endpoint)
                                   {
                                       if (!ec)
                                       {
                                           do_read_header();
                                       }
                                   });
    }

    void do_read_oneMoreByte() {
        {
            boost::asio::async_read(socket_,
                                    boost::asio::buffer(read_msg_.body(), 1),
                                    [this](boost::system::error_code ec, std::size_t /*length*/)
                                    {
                                        if (!ec)
                                        {
                                            read_msg_.decode_header(int(reinterpret_cast<const unsigned char&>(read_msg_.body()[0])) * 2);
//                                            std::cout << "---Number of Moves: " << int(reinterpret_cast<const unsigned char&>(read_msg_.body()[0])) << std::endl;
                                            do_read_body(1);
                                        }
                                        else
                                        {
                                            socket_.close();
                                        }
                                    });
        }
    }

    void do_read_header()
    {
//        std::cout << "message received" << std::endl;
        messageReceived = true;
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), message::header_length),
                                [this](boost::system::error_code ec, std::size_t /*length*/)
                                {
                                    messageType = int(read_msg_.data()[0]);
//                                    std::cout << "---" << int(read_msg_.data()[0]) << std::endl;
                                    if (int(reinterpret_cast<const unsigned char&>(read_msg_.data()[0])) == 0b10001000)
                                    {
//                                        std::cout << "---Reading one more byte" << std::endl;
                                        do_read_oneMoreByte();
                                    }
                                    else if (!ec && read_msg_.decode_header())
                                    {
                                        if (read_msg_.body_length() > 0) {
                                            //std::cout << "valid response" << std::endl;
                                            do_read_body();
                                        } else {
                                            if (read_msg_.data()[0] == - 1) {
//                                                std::cout << "---error while reading";
                                            }
                                            do_read_header();
                                        }
                                    }
                                    else
                                    {
//                                        std::cout << "---socket closed" << std::endl;
                                        socket_.close();
                                    }
                                });
    }

    void do_read_body(int x = 0)
    {
        msgBody = read_msg_.body();
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body() + x, read_msg_.body_length() - x),
                                [this](boost::system::error_code ec, std::size_t /*length*/)
                                {
                                    if (!ec)
                                    {
                                        messageType = int(reinterpret_cast<const unsigned char&>(read_msg_.data()[0]));
                                        if (int(reinterpret_cast<const unsigned char&>(read_msg_.data()[0]) == 0b10001000))     //initialize game
                                        {
                                            for (int i = 2; i < 22; i = i + 2) {
                                                std::cout << int(read_msg_.body()[i]) << ((read_msg_.body()[i + 1] >>  2 ) & 1) << ((read_msg_.body()[i + 1] >>  1 ) & 1) << ((read_msg_.body()[i + 1] >>  0 ) & 1) << std::endl;
                                            }
                                            for (int i = 22; i < 22 + int(reinterpret_cast<const unsigned char&>(read_msg_.body()[00])) * 2; i = i + 2) {
                                            }
                                        }
                                        else if(messageType == 129)
                                        {
                                            char* newGameName = read_msg_.body();
                                            runningGame->addGameToList(newGameName);
                                        }
                                        do_read_header();
                                    }
                                    else
                                    {
                                        socket_.close();
                                    }
                                });
    }

    void do_write()
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                         write_msgs_.pop_front();
                                         if (!write_msgs_.empty())
                                         {
                                             do_write();
                                         }
                                     }
                                     else
                                     {
                                         socket_.close();
                                     }
                                 });
    }

private:
    session *runningGame;
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    message read_msg_;
    message_queue write_msgs_;
};

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

int waitForMessage(client *c, int timeout, int type = 0)
{
    auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float timePassed = 0;
    do
    {
        now = std::chrono::steady_clock::now();
        timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
    }while((!c->messageReceived || (std::abs(c->messageType) != type && type != 0)) && (timePassed < timeout || timeout == -1));
    if(c->messageReceived)
    {
        c->messageReceived = false;
        int messageType = c->messageType;
        c->messageType = 0;
        return messageType;
    }
    return 0;
}

void requestOngoingGames(client *serverConnection)
{
    message msg;
    char header = 0b00000001;
    msg.encode_header(header);
    serverConnection->write(msg);
}
void displayGamesList(session *gameSession, client *serverConnection)
{
    gameSession->forgetAllGames();
    requestOngoingGames(serverConnection);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));                                   //Waiting 1 second for messages because number of games is unknown
    std::cout << "Found " << gameSession->getGameList().size()<< " players:" << std::endl;
    std::cout << "Ongoing games:" << std::endl;
    std::cout << "id  |  name" << std::endl;
    for(int i = 0; i < gameSession->getGameList().size(); i++)
    {
        std::cout << i << ":     " << gameSession->getGameList().at(i) << std::endl;
    }
}
bool joinGame(char* gameName, client *serverConnection, session *gameSession)
{
    message msg;
    char header = 0b00000010;
    msg.encode_header(header);
    std::memcpy(msg.body(), gameName, msg.body_length());
    serverConnection->write(msg);
    int returnMsgType = waitForMessage(serverConnection, defaultTimeout, 130);
    if(returnMsgType != 130)
    {
        return false;
    }
    gameSession->setOpponentName(serverConnection->msgBody);

    return true;
}
std::string joinGame(session *gameSession, client *serverConnection)
{
    gameSession->setWhoIAm(0);
    std::string chosenOption = "r";
    bool displayGameList = true;
    while(chosenOption == "r")
    {
        if(displayGameList)
        {
            displayGamesList(gameSession, serverConnection);
            std::cout << "Enter the id to start, 'r' to refresh or 'h' if you want to host a game" << std::endl;
            displayGameList = false;
        }

        std::cin >> chosenOption;
        if(chosenOption.at(0) == 'R' || chosenOption.at(0) == 'r')
        {
            chosenOption = 'r';
            displayGameList = true;
        }

        if(is_number(chosenOption))                     //if player picked a game
        {
            int index = atoi(chosenOption.c_str());
            if(index > gameSession->getGameList().size() || index < 0)
            {
                std::cout << "invalid number" << std::endl;
                displayGameList = true;
            }
            else
            {
                std::string gameNameString = gameSession->getGameList().at(index);
                char* gameName = _strdup(gameNameString.c_str());
                bool joined = joinGame(gameName, serverConnection, gameSession);
                if(joined)
                {
                    std::cout << "Join successful" << std::endl;
                    return "joined";
                }
                else
                {
                    std::cout << "Join failed" << std::endl;
                    chosenOption = 'r';
                }
            }
        }
    }
    return chosenOption;
}
bool host(session *gameSession, client *serverConnection)
{
    gameSession->setWhoIAm(1);
    std::cout << "Hosting a game" << std::endl;
    message msg;
    char header = 0b00000000;
    msg.encode_header(header);
    serverConnection->write(msg);
    if(waitForMessage(serverConnection, defaultTimeout) > 0)
    {
        std::cout << "Server did not accept the host request" << std::endl;
        return false;
    }

    do
    {
        std::cout << "Waiting for joining players..." << std::endl;

    }while(waitForMessage(serverConnection, 5000) == 0);

    std::cout << "Player joined" << std::endl;
}

void setName(session *gameSession, client *serverConnection)
{
    message msg;
    msg.encode_header(0b00000101);
    char line[message::max_body_length + 1];
    std::cout << "Please enter your name: ";
    std::cin >> line;
    std::memcpy(msg.body(), line, msg.body_length());
    serverConnection->write(msg);
    gameSession->setOpponentName(line);
}

void displayBoardShips(session *gameSession)
{
    std::cout << "  0|1|2|3|4|5|6|7|8|9" << std::endl;
    char rowLetter = 'A';
    for(int j = 0; j<10; j++)
    {
        std::cout << rowLetter;
        for(int i = 0; i < 10; i++) std::cout << " " << gameSession->getShipAt(j, i);
        std::cout << " " << rowLetter << std::endl;
        rowLetter++;
    }
    std::cout << "  0|1|2|3|4|5|6|7|8|9" << std::endl;
    
}
void displayBoardShots(session *gameSession)
{
    std::cout << "  0|1|2|3|4|5|6|7|8|9" << std::endl;
    char rowLetter = 'A';
    for(int j = 0; j<10; j++)
    {
        std::cout << rowLetter;
        for(int i = 0; i < 10; i++)
        {

            if(gameSession->getShotAt(j, i) == 1)
            {
                std::cout << " X";
            }
            else if(gameSession->getShotAt(j, i) == 2)
            {
                std::cout << " O";
            }
            else
            {
                std::cout << "  ";
            }
        }
        std::cout << " " << rowLetter << std::endl;
        rowLetter++;
    }
    std::cout << "  0|1|2|3|4|5|6|7|8|9" << std::endl;
}

bool checkCharInVector(std::vector<char> *theVector, char theChar)
{
    bool inVector = false;
    for(int i = 0; i < theVector->size(); i++)
    {
        if(theVector->at(i) == theChar) inVector = true;
    }
    return inVector;
}

int sendShipToServer(client *serverConnection, char row, int column, int direction, char shipType)
{
    int successful = 0;
    int rowNumber = int(row) - 97;
    if(rowNumber < 0) rowNumber = rowNumber + 32;
    int position = rowNumber * 10 + column;
    message msg;
    msg.encode_header(0b00000011);
    char line[message::max_body_length + 1] = "";
    line[0] = position;
    char shipAndDirection = 0;
    switch(shipType)
    {
        case 'b':
            shipAndDirection |= 0 << 0;
            shipAndDirection |= 0 << 1;
            break;
        case 'c':
            shipAndDirection |= 1 << 0;
            shipAndDirection |= 0 << 1;
            break;
        case 'd':
            shipAndDirection |= 0 << 0;
            shipAndDirection |= 1 << 1;
            break;
        case 's':
            shipAndDirection |= 1 << 0;
            shipAndDirection |= 1 << 1;
            break;
    }
    shipAndDirection |= direction << 2;
    line[1] = shipAndDirection;

    std::memcpy(msg.body(), line, msg.body_length());
    serverConnection->write(msg);
    int resultType = std::abs(waitForMessage(serverConnection, 500, 125));
    switch(resultType)
    {
        case 125:
            successful = 1;
            break;
        case 132:
            successful = 2;
            break;
        case 123:
            successful = 3;
            break;
        default:
            successful = 0;
    }
    return successful;

}

int setShips(session *gameSession, client *serverConnection)
{
    std::cout << "Now is the time to place your ships on the field" << std::endl;
    std::cout << "You got submarines (s), cruiser (c), destroyer (d) and battleships (b)" << std::endl;
    displayBoardShips(gameSession);

    int countSet = 0;
    int status = 0;

    std::vector<char> placementOrder = {'s', 's', 's', 's', 'd', 'd', 'd', 'c', 'c', 'b'};
    std::vector<char*> shipTypes = {"submarine", "destroyer", "cruiser", "battleship"};
    int internalShipCount = 1;
    int shipTypeCount = 0;
    while(countSet < 10)
    {
        std::cout << "Place your " << internalShipCount << ". " << shipTypes.at(shipTypeCount) << std::endl;
        int direction;

        do
        {
            std::cout << "How shall the ship be placed? Horizontal (1) or vertical (0): ";
            char directionChar;
            std::cin >> directionChar;
            direction = (int)directionChar - 48;
            std::cout << std::endl;
        }while(direction < 0 ||direction > 1);

        std::vector<char> allowedRows = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
        char pickedRow;
        do
        {
            std::cout << "Row (A-J)   : ";
            std::cin >> pickedRow;
            std::cout << std::endl;
        }while(!checkCharInVector(&allowedRows, pickedRow));

        int pickedColumn;
        do
        {
            std::cout << "Column (0-9): ";
            std::cin >> pickedColumn;
            std::cout << std::endl;
        }while(pickedColumn < 0 || pickedColumn > 9);

        bool shipSet = gameSession->setShip(pickedRow, pickedColumn, direction, placementOrder.at(countSet));
        if(shipSet)
        {
            status = sendShipToServer(serverConnection, pickedRow, pickedColumn, direction, placementOrder.at(countSet));
            if(status > 0)
            {
                countSet++;
                internalShipCount++;
                if(countSet == 4 || countSet == 7 || countSet == 9 || countSet == 10)
                {
                    internalShipCount = 1;
                    shipTypeCount++;
                }
                displayBoardShips(gameSession);
            }
            else
            {
                gameSession->unsetShipAt(pickedRow, pickedColumn, direction, placementOrder.at(countSet));
                displayBoardShips(gameSession);
            }
        }
    }
    return status;
}

int sendShotToServer(client *serverConnection, int row, int column)
{
    int position = row * 10 + column;
    message msg;
    msg.encode_header(0b00000100);
    char line[message::max_body_length + 1] = "";
    line[0] = position;
    std::memcpy(msg.body(), line, msg.body_length());
    serverConnection->write(msg);
    bool waitForResult = true;
    int result;
    while(waitForResult)
    {
        result = waitForMessage(serverConnection, 500, 134);
        if(result == 134 || result == 135)
        {
            waitForResult = false;
        }
    }
    if(result == 134)
    {
        int hit = int(serverConnection->msgBody[1]);
        if(hit == 6 ||hit == 2)
        {
            std::cout << "Ship hit" << std::endl;
            return 1;
        }
        else if(hit == 7 ||hit == 3)
        {
            std::cout << "Ship destroyed" << std::endl;
            return 2;
        }
        else
        {
            return 0;
        }
    }
    if(result == 135)
    {
        return 4;
    }
    return 0;
}
void displayBoards(session *gameSession)
{
    displayBoardShips(gameSession);
    displayBoardShots(gameSession);
}
void shoot(client *serverConnection, session *gameSession)
{
    std::cout << "pick where you want to shoot" << std::endl;
    displayBoardShots(gameSession);

    std::vector<char> allowedRows = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
    bool shotSuccessful = false;
    while(!shotSuccessful)
    {
        char pickedRow;
        int pickedColumn;
        do
        {
            std::cout << "Row (A-J)   : ";
            std::cin >> pickedRow;
            std::cout << std::endl;
        }while(!checkCharInVector(&allowedRows, pickedRow));

        do
        {
            std::cout << "Column (0-9): ";
            std::cin >> pickedColumn;
            std::cout << std::endl;
        }while(pickedColumn < 0 || pickedColumn > 9);


        int rowNumber = int(pickedRow) - 97;
        if(rowNumber < 0) rowNumber = rowNumber + 32;
        if(gameSession->getShotAt(rowNumber, pickedColumn) < 1)
        {
            int hit = sendShotToServer(serverConnection, rowNumber, pickedColumn);
            if(hit == 4)
            {
                gameSession->setShotAt(pickedRow, pickedColumn, 2);
                displayBoards(gameSession);
                std::cout << "You have destroyed all ships of your opponent. This means you have won!" << std::endl;
                exit(0);
            }
            shotSuccessful = gameSession->setShotAt(pickedRow, pickedColumn, hit);
        }
    }

    return;
}

int main(int argc, char* argv[])
{
    char* hostAddress;
    char* hostPort;
    if (argc != 3)
    {
        std::cout << "Connecting to local server on port 1000. Use 'schiffe_versenken_client <host address> <host port>' if you want to specify something alse" << std::endl;
        hostAddress = "127.0.0.1";
        hostPort = "1000";
    }
    else
    {
        hostAddress = argv[1];
        hostPort = argv[2];
    }

    try
    {
        std::cout << "Welcome to schiffe versenken" << std::endl;
        std::cout << "You can end the game whenever you are asked to make a selection. Just enter x then" << std::endl;
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(hostAddress, hostPort);
        auto *gameSession = new session();
        auto *serverConnection = new client(io_context, endpoints, gameSession);

        std::thread t([&io_context](){ io_context.run(); });

        waitForMessage(serverConnection, defaultTimeout);

        setName(gameSession, serverConnection);

        waitForMessage(serverConnection, defaultTimeout);

        char line[message::max_body_length + 1];
        std::cout << std::endl;
        std::cout << "Do you want to join (j) into one of the games or to host a own game (h):" << std::endl;
        std::cin >> line;

        std::string decision = std::string(1, line[0]);
        std::vector<char> connectingOptions = {'j', 'J'};
        while(decision[0] == 'j' || decision[0] == 'J' || decision[0] == 'h' || decision[0] == 'H')
        {
            if(decision[0] == 'j' || decision[0] == 'J')
            {
                decision = joinGame(gameSession, serverConnection);
                if(decision == "joined")
                {
                    break;
                }
            }
            if(decision.at(0) == 'H' || decision.at(0) == 'h')
            {
                if(host(gameSession, serverConnection))
                {
                    break;
                }
                else
                {
                    std::cout << "Hosting failed, do you want to try again (h) or join annother player (j)";
                    std::cin >> line;
                    decision = std::string(1, line[0]);
                }
            }
        }
        if(decision[0] == 'x')
        {
            exit(0);
        }

        int status = setShips(gameSession, serverConnection);
        if(status == 0)
        {
            std::cout << "Could not set ships" << std::endl;
            exit(0);
        }
        else if(status == 2)
        {
            bool otherPlayerReady = false;
            while(!otherPlayerReady)
            {
                std::cout << "Waiting for your opponent to set his ships" << std::endl;
                if(waitForMessage(serverConnection, 5000, 123) != 0)
                {
                    otherPlayerReady = true;
                }
            }
        }
        std::cout << "All players have set the ships. The game can begin" << std::endl;



        bool gameRunning = true;
        int myTurn = false;
        if(gameSession->getWhoIAm() == 1) myTurn = true;
        while(gameRunning)
        {
            if(myTurn)
            {
                displayBoardShips(gameSession);
                shoot(serverConnection, gameSession);
                myTurn = false;
            }
            else
            {
                int receivedType;
                displayBoards(gameSession);
                std::cout << "Waiting for the other player to shoot" << std::endl;
                bool stillPickingHisShot = true;
                while(stillPickingHisShot)
                {
                    receivedType = waitForMessage(serverConnection, 1000, 134);
                    if (receivedType == 134)
                    {
                        stillPickingHisShot = false;
                    }
                    else if (receivedType == 135)
                    {
                        displayBoards(gameSession);
                        std::cout
                                << "You opponent has destroyed all your ships, the game is over now and and you have lost"
                                << std::endl;
                        exit(0);
                    }
                }
                int shotPosition = int(serverConnection->msgBody[0]);
                std::cout << "shot at " << shotPosition << std::endl;
                gameSession->setShotAt(shotPosition);
                myTurn = true;
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
