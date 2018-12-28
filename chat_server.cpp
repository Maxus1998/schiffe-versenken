//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#include "chat_message.hpp"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant
{
public:
  virtual ~chat_participant() {}
  virtual void deliver(const chat_message& msg) = 0;
  std::string name_ = "";
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class room
{
public:
    void join(chat_participant_ptr participant)
    {
        participants_.insert(participant);
    }

    void leave(chat_participant_ptr participant)
    {
        participants_.erase(participant);
    }

    virtual void deliver(const chat_message& msg, chat_participant_ptr participant) = 0;

    void sendErrorMessage(chat_participant_ptr participant) {
        chat_message error;
        error.encode_header(0b11111111);
        participant -> deliver(error);
    }

    std::set<chat_participant_ptr> participants_;
    std::set<chat_participant_ptr> searchingParticipants_;
};
//----------------------------------------------------------------------

class waitingRoom : public room {
public:

    bool setName(chat_participant_ptr &participant, const char name[]) {
        if (participant -> name_.empty()) {
            for (chat_participant_ptr x : participants_) {
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

    bool createGame(chat_participant_ptr &participant) {
        for (chat_participant_ptr x : searchingParticipants_) {
            if (x -> name_ == participant -> name_) {
                return false;
            }
        }
        searchingParticipants_.insert(participant);
        return true;
    }

    int deleteGame(chat_participant_ptr &participant) {
        return searchingParticipants_.erase(participant);
    }

    void deliver(const chat_message& msg, chat_participant_ptr participant) override{
        switch(int(reinterpret_cast<const unsigned char&>(msg.data()[0]))) {
            case 0:
                std::cout << "Create game!";
                if (createGame(participant)) {
                    chat_message confirm;
                    confirm.encode_header(0b10000000);
                    participant -> deliver(confirm);
                } else {
                    sendErrorMessage(participant);
                }
                break;
            case 1:
                std::cout << "Request players!";
                break;
            case 2:
                std::cout << "Join player " << msg.body() << "!";
                break;
            case 5:
                std::cout << "Send name " << msg.body() << "!";
                if (setName(participant, msg.body())) {
                    chat_message confirm;
                    confirm.encode_header(0b10001001);
                    participant -> deliver(confirm);
                } else {
                    sendErrorMessage(participant);
                }
                break;
            case 6:
                std::cout << "Delete game!";
                if (deleteGame(participant)) {
                    chat_message confirm;
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

class gameRoom : public room {
public:
    void deliver (const chat_message& msg, chat_participant_ptr participant) override{
        switch(int(reinterpret_cast<const unsigned char&>(msg.data()[0]))) {
            case 3:
                std::cout << "Set Ship at " << int(msg.body()[0]) << " " << int(msg.body()[1]);
                break;
            case 4:
                std::cout << "Shoot at " << int(msg.body()[0]) << "!";
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

class chat_session
  : public chat_participant,
    public std::enable_shared_from_this<chat_session>
{
public:
    chat_session(tcp::socket socket, waitingRoom& room)
        : socket_(std::move(socket)),
        room_(room)
    {
    }

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }

    void deliver(const chat_message& msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

    bool isInAGame_ = false;
    gameRoom gameRoom_;
private:
    void do_read_header()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.data(), chat_message::header_length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec && read_msg_.decode_header())
                {
                    if (int(reinterpret_cast<unsigned char&>(read_msg_.data()[0])) >= 128) {
                        chat_message error;
                        error.encode_header(0b11111111);
                        deliver(error);
                    } else {
                        switch(int(reinterpret_cast<unsigned char&>(read_msg_.data()[0]))) {
                            case 0:
                            case 1:
                            case 6:
                            case 127:
                                if (isInAGame_) {
                                    gameRoom_.deliver(read_msg_, shared_from_this());
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
                        gameRoom_.leave(shared_from_this());
                    }
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
                      gameRoom_.deliver(read_msg_, shared_from_this());
                  } else {
                      room_.deliver(read_msg_, shared_from_this());
                  }
                  do_read_header();
              }
              else
              {
                  room_.leave(shared_from_this());
                  if (isInAGame_) {
                      gameRoom_.leave(shared_from_this());
                  }
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
                      gameRoom_.leave(shared_from_this());
                  }
              }
          });
  }

  tcp::socket socket_;
  waitingRoom& room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class chat_server
{
public:
  chat_server(boost::asio::io_context& io_context,
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
            std::make_shared<chat_session>(std::move(socket), room_)->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  waitingRoom room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_context io_context;

    std::list<chat_server> servers;
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