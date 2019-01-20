//
// server.cpp
// ~~~~~~~~~~~~~~~
//
// This has been made using the official boost example for a chat server application.
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <string>
#include <boost/asio.hpp>
#include "message.hpp"
#include "Schiffe_versenken.hpp"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<message> message_queue;
class game_room;

//----------------------------------------------------------------------

class participant
{
public:
  virtual ~participant() {}
  virtual void deliver(const message& msg) = 0;
  std::string name_ = "";
  std::shared_ptr<game_room> game_room_;
  bool isInAGame_ = false;
  bool player;
};

typedef std::shared_ptr<participant> participant_ptr;

//----------------------------------------------------------------------

class room
{
public:
    void join(participant_ptr participant)
    {
        participants_.insert(participant);
    }

    void leave(participant_ptr participant)
    {
        participants_.erase(participant);
    }

    virtual void deliver(const message& msg, participant_ptr participant) = 0;

    void sendErrorMessage(participant_ptr participant) {
        message error;
        error.encode_header(0b11111111);
        participant -> deliver(error);
    }

    std::set<participant_ptr> participants_;
    std::set<participant_ptr> searchingParticipants_;
};

//----------------------------------------------------------------------

class game_room : public room {
public:
    Schiffe_versenken game;
    bool shipsSet = false;
    char player1Ships[20];
    char* shipPointer1 = player1Ships;
    char player2Ships[20];
    char* shipPointer2 = player2Ships;
    size_t numberOfMoves = 0;
    char moves[400];

    void disconnectPlayer(participant_ptr participant) {
        message confirm;
        confirm.encode_header(0b10001100);
        participant -> deliver(confirm);
        if (!shipsSet) {
            game.deleteShips(!participant -> player);
        }
    }

    int setShip(participant_ptr participant, const char space, const char shipInfo) {
        int shipType;
        bool horizontal;
        if ((shipInfo >> 0) & 1) {
            if ((shipInfo >> 1) & 1) {
                shipType = 2;
                std::cout << "submarine";
            } else {
                shipType = 4;
                std::cout << "cruiser";
            }
        } else {
            if ((shipInfo >> 1) & 1) {
                shipType = 3;
                std::cout << "destroyer";
            } else {
                shipType = 5;
                std::cout << "battleship";
            }
        }
        if ((shipInfo >> 2) & 1) {
            horizontal = true;
        } else {
            horizontal = false;
        }
        return game.setShip(int(space), horizontal, participant -> player, shipType);
    }

    bool sendSetShips(participant_ptr &participant, int rc) {
        bool valid = true;
        message confirm;
        switch(rc) {
            case 0: sendErrorMessage(participant);
                    valid = false;
                    break;
            case 1: confirm.encode_header(0b10000011);
                    participant -> deliver(confirm);
                    break;
            case 2: confirm.encode_header(0b10000100);
                    char x;
                    if (participant -> player) {
                        x |= 1 << 0;
                    } else {
                        x |= 0 << 0;
                    }
                    std::memcpy(confirm.body(), &x, confirm.body_length());
                    for (participant_ptr y : participant -> game_room_ -> participants_) {
                        y->deliver(confirm);
                    }
                    break;
            case 3: confirm.encode_header(0b10000101);
                    for (participant_ptr y : participant -> game_room_ -> participants_) {
                        y->deliver(confirm);
                    }
                    shipsSet = true;
                    break;
        }
        return valid;
    }

    bool makeMoveAndSend(participant_ptr participant, char space) {
        bool valid = false;
        int rc = game.makeMove(space, participant -> player);
        message confirm;
        bool hit;
        bool sunk;
        bool gameOver;
        char body[2];
        switch(rc) {
            case 0: sendErrorMessage(participant);
                    break;
            case 1: hit = false;
                    sunk = false;
                    gameOver = false;
                    break;
            case 2: hit = true;
                    sunk = false;
                    gameOver = false;
                    break;
            case 3: hit = true;
                    sunk = true;
                    gameOver = false;
                    break;
            case 4: hit = true;
                    sunk = true;
                    gameOver = true;
                    break;
        }
        if (rc != 0) {
            valid = true;
            if (!gameOver) {
                confirm.encode_header(0b10000110);
            } else {
                confirm.encode_header(0b10000111);
            }
            body[0] = space;
            char x = 0;
            if (participant->player) {
                std::cout << "player";
                x |= 1 << 0;
            } else {
                x |= 0 << 0;
            }
            if (hit) {
                std::cout << "hit";
                x |= 1 << 1;
            } else {
                x |= 0 << 1;
            }
            if (sunk) {
                std::cout << "sunk";
                x |= 1 << 2;
            } else {
                x |= 0 << 2;
            }
            body[1] = x;
            std::memcpy(confirm.body(), body, confirm.body_length());
            participant -> game_room_ -> moves[participant -> game_room_ -> numberOfMoves * 2] = body[0];
            participant -> game_room_ -> moves[(participant -> game_room_ -> numberOfMoves * 2) + 1] = body[1];
            participant -> game_room_ -> numberOfMoves = participant -> game_room_ -> numberOfMoves + 1;
            participant_ptr otherPlayer;
            for (participant_ptr x : participant -> game_room_ -> participants_) {
                x -> deliver(confirm);
                if (x -> name_ != participant -> name_) {
                    otherPlayer = x;
                }
            }
            if (gameOver) {
                leave(participant);
                participant->isInAGame_ = false;
                leave(otherPlayer);
                otherPlayer->isInAGame_ = false;
            }
        }
        return valid;
    }

    void deliver (const message& msg, participant_ptr participant) override{
        switch(int(reinterpret_cast<const unsigned char&>(msg.data()[0]))) {
            case 3:
                std::cout << "Set Ship";
                if (sendSetShips(participant, setShip(participant, msg.body()[0], msg.body()[1]))) {
                    if (participant -> player) {
                        *(participant -> game_room_ -> shipPointer1) = msg.body()[0];
                        *(participant -> game_room_ -> shipPointer1 + 1) = msg.body()[1];
                        participant -> game_room_ -> shipPointer1 = participant -> game_room_ -> shipPointer1 + 2;
                    } else {
                        *(participant -> game_room_ -> shipPointer2) = msg.body()[0];
                        *(participant -> game_room_ -> shipPointer2 + 1) = msg.body()[1];
                        participant -> game_room_ -> shipPointer2 = participant -> game_room_ -> shipPointer2 + 2;
                    }
                }
                break;
            case 4:
                std::cout << "Shoot at " << int(msg.body()[0]) << "!";
                makeMoveAndSend(participant, msg.body()[0]);
                break;
            case 127:
                std::cout << "Error!";
                break;
            default:
                sendErrorMessage(participant);
        }
    }
};

//----------------------------------------------------------------------

class waiting_room : public room {
public:

    bool setName(participant_ptr &participant, const char name[]) {
        if (participant -> name_.empty()) {
            for (participant_ptr x : participants_) {
                if (x -> name_ == name) {
                    return false;
                }
            }
            participant -> name_ = name;
            return true;
        } else {
            return false;
        }
    }

    bool createGame(participant_ptr participant) {
        if (participant -> name_.empty()) {
            return false;
        }
        for (participant_ptr x : searchingParticipants_) {
            if (x -> name_ == participant -> name_) {
                return false;
            }
        }
        searchingParticipants_.insert(participant);
        participant -> game_room_ = std::make_shared<game_room>();
        participant -> game_room_ -> join(participant);
        participant -> game_room_ -> game.initialize();
        participant -> player = true;
        return true;
    }

    int stopSearching(participant_ptr participant) {
        return searchingParticipants_.erase(participant);
    }

    void sendPlayers(participant_ptr participant) {
        if (searchingParticipants_.empty()) {
            message confirm;
            confirm.encode_header(0b10001011);
            participant -> deliver(confirm);
        } else {
            for (participant_ptr x : searchingParticipants_) {
                std::cout << x -> name_;
                message confirm;
                confirm.encode_header(0b10000001);
                char body[16] = "";
                std::memcpy(body, x -> name_.c_str(), x -> name_.length() + 1);
                std::memcpy(confirm.body(), body, confirm.body_length());
                participant -> deliver(confirm);
            }
        }
    }

    void joinGame(const participant_ptr participant, const char name[]) {
        bool rc = false;
        participant_ptr joinedPlayer;
        for (const participant_ptr x: searchingParticipants_) {
            if (x -> name_ == name && participant -> name_ != name && !participant -> name_.empty()) {
                participant -> game_room_ = x -> game_room_;
                participant -> game_room_ -> join(participant);
                rc = true;
                joinedPlayer = x;
            }
        }
        if (rc) {
            stopSearching(joinedPlayer);
            if (!joinedPlayer -> isInAGame_) {
                participant -> isInAGame_ = true;
                joinedPlayer -> isInAGame_ = true;
                participant -> player = false;
                message confirm;
                confirm.encode_header(0b10000010);
                char body1[17] = "";
                char player;
                if (joinedPlayer -> player) {
                    player = 1;
                } else {
                    player = 0;
                }
                std::memcpy(body1, participant -> name_.c_str(), participant -> name_.length() + 1);
                body1[16] = player;
                std::memcpy(confirm.body(), body1, confirm.body_length());
                joinedPlayer -> deliver(confirm);
                char body2[17];
                if (participant -> player) {
                    player = 1;
                } else {
                    player = 0;
                }
                std::memcpy(body2, joinedPlayer -> name_.c_str(), joinedPlayer -> name_.length() + 1);
                body2[16] = player;
                std::memcpy(confirm.body(), body2, confirm.body_length());
                participant -> deliver(confirm);
            } else {
                participant -> isInAGame_ = true;
                participant -> player = !joinedPlayer -> player;
                message reconnect;
                reconnect.encode_header(0b10001101);
                char body1[16] = "";
                std::memcpy(body1, participant -> name_.c_str(), participant -> name_.size() + 1);
                std::memcpy(reconnect.body(), body1, reconnect.body_length());
                joinedPlayer -> deliver(reconnect);
                if (!joinedPlayer -> game_room_ -> shipsSet) {
                    message confirm;
                    confirm.encode_header(0b10000010);
                    char body2[17] = "";
                    char player;
                    if (participant -> player) {
                        player = 1;
                    } else {
                        player = 0;
                    }
                    std::memcpy(body2, joinedPlayer -> name_.c_str(), joinedPlayer -> name_.length() + 1);
                    body2[16] = player;
                    std::memcpy(confirm.body(), body2, confirm.body_length());
                    participant -> deliver(confirm);
                } else {
                    message initialize;
                    initialize.encode_header(0b10001000, participant -> game_room_ -> numberOfMoves * 2);
                    char body[421] = "";
                    char moves = participant -> game_room_ -> numberOfMoves;
                    std::cout << int(moves);
                    body[0] = moves;
                    char* ships;
                    if (participant -> player) {
                        body[1] = 1;
                        ships = participant -> game_room_ -> player1Ships;
                    } else {
                        body[1] = 0;
                        ships = participant -> game_room_ -> player2Ships;
                    }
                    std::memcpy(body + 2, ships, 20);
                    std::memcpy(body + 22, participant -> game_room_ -> moves, participant -> game_room_ -> numberOfMoves * 2);
                    std::memcpy(initialize.body(), body, initialize.body_length());
                    participant -> deliver(initialize);
                }
            }
        } else {
            sendErrorMessage(participant);
        }
    }

    void deliver(const message& msg, participant_ptr participant) override{
        switch(int(reinterpret_cast<const unsigned char&>(msg.data()[0]))) {
            case 0:
                std::cout << "Create game!";
                if (createGame(participant)) {
                    message confirm;
                    confirm.encode_header(0b10000000);
                    participant -> deliver(confirm);
                } else {
                    sendErrorMessage(participant);
                }
                break;
            case 1:
                std::cout << "Request players!";
                if (!participant -> name_.empty()) {
                    sendPlayers(participant);
                } else {
                    sendErrorMessage(participant);
                }
                break;
            case 2:
                std::cout << "Join player " << msg.body() << "!";
                joinGame(participant, msg.body());
                break;
            case 5:
                std::cout << "Send name " << msg.body() << "!";
                if (setName(participant, msg.body())) {
                    message confirm;
                    confirm.encode_header(0b10001001);
                    participant -> deliver(confirm);
                } else {
                    sendErrorMessage(participant);
                }
                break;
            case 6:
                std::cout << "Delete game!";
                if (stopSearching(participant)) {
                    message confirm;
                    confirm.encode_header(0b10001010);
                    participant -> deliver(confirm);
                } else {
                    sendErrorMessage(participant);
                }
                break;
            case 127:
                std::cout << "Error!";
                break;
            default:
                sendErrorMessage(participant);
        }
    }
};

//----------------------------------------------------------------------

class session
  : public participant,
    public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket, waiting_room& room)
        : socket_(std::move(socket)),
        room_(room)
    {
    }

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }

    void deliver(const message& msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:
    void do_read_header()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.data(), message::header_length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec && read_msg_.decode_header())
                {
                    if (int(reinterpret_cast<unsigned char&>(read_msg_.data()[0])) >= 128) {
                        message error;
                        error.encode_header(0b11111111);
                        deliver(error);
                    } else {
                        switch(int(reinterpret_cast<unsigned char&>(read_msg_.data()[0]))) {
                            case 0:
                            case 1:
                            case 6:
                            case 127:
                                if (isInAGame_) {
                                    game_room_ -> deliver(read_msg_, shared_from_this());
                                } else {
                                    room_.deliver(read_msg_, shared_from_this());
                                }
                                do_read_header();
                                break;
                            default:
                                do_read_body();
                        }
                    }
                }
                else
                {
                    room_.leave(shared_from_this());
                    if (isInAGame_) {
                        game_room_ -> leave(shared_from_this());
                        if (!game_room_ -> participants_.empty()) {
                            game_room_->disconnectPlayer(*game_room_->participants_.begin());
                            room_.searchingParticipants_.insert(*game_room_ -> participants_.begin());
                        }
                    }
                    room_.stopSearching(shared_from_this());
                }
            });
  }

  void do_read_body()
  {
      auto self(shared_from_this());
      boost::asio::async_read(socket_,
          boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
          [this, self](boost::system::error_code ec, std::size_t /*length*/)
          {
              if (!ec)
              {
                  if (isInAGame_) {
                      game_room_ -> deliver(read_msg_, shared_from_this());
                  } else {
                      room_.deliver(read_msg_, shared_from_this());
                  }
                  do_read_header();
              }
              else
              {
                  room_.leave(shared_from_this());
                  if (isInAGame_) {
                      game_room_ -> leave(shared_from_this());
                      if (!game_room_ -> participants_.empty()) {
                          game_room_->disconnectPlayer(*game_room_->participants_.begin());
                          room_.searchingParticipants_.insert(*game_room_ -> participants_.begin());
                      }
                  }
                  room_.stopSearching(shared_from_this());
              }
          });
  }

  void do_write()
  {
      auto self(shared_from_this());
      boost::asio::async_write(socket_,
          boost::asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
          [this, self](boost::system::error_code ec, std::size_t /*length*/)
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
                  room_.leave(shared_from_this());
                  if (isInAGame_) {
                      game_room_ -> leave(shared_from_this());
                      if (!game_room_ -> participants_.empty()) {
                          game_room_->disconnectPlayer(*game_room_->participants_.begin());
                          room_.searchingParticipants_.insert(*game_room_ -> participants_.begin());
                      }
                  }
                  room_.stopSearching(shared_from_this());
              }
          });
  }

  tcp::socket socket_;
  waiting_room& room_;
  message read_msg_;
  message_queue write_msgs_;
};

//----------------------------------------------------------------------

class server
{
public:
  server(boost::asio::io_context& io_context,
      const tcp::endpoint& endpoint)
    : acceptor_(io_context, endpoint)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket), room_)->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  waiting_room room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_context io_context;

    std::list<server> servers;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_context, endpoint);
    }

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}