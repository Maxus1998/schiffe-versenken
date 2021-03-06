//This Client is NOT part of the actual application. It is merely used to help test the server functionality

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

class client
{
public:
  client(boost::asio::io_context& io_context,
      const tcp::resolver::results_type& endpoints)
    : io_context_(io_context),
      socket_(io_context)
  {
    do_connect(endpoints);
  }

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
                      std::cout << "Number of Moves: " << int(reinterpret_cast<const unsigned char&>(read_msg_.body()[0]));
                      std::cout << std::endl;
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
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.data(), message::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          std::cout << int(read_msg_.data()[0]) << std::endl;
          if (int(reinterpret_cast<const unsigned char&>(read_msg_.data()[0])) == 0b10001000) {
              do_read_oneMoreByte();
          }
          else if (!ec && read_msg_.decode_header())
          {
            if (read_msg_.body_length() > 0) {
                do_read_body();
            } else {
                if (read_msg_.data()[0] == - 1) {
                    std::cout << "error";
                }
                do_read_header();
            }
          }
          else
          {
            socket_.close();
          }
        });
  }

  void do_read_body(int x = 0)
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.body() + x, read_msg_.body_length() - x),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            if (int(reinterpret_cast<const unsigned char&>(read_msg_.data()[0]) == 0b10001000)) {
                std::cout << "Player: ";
                std::cout << int(read_msg_.body()[1]) << std::endl;
                std::cout << "Ships:" << std::endl;
                for (int i = 2; i < 22; i = i + 2) {
                    std::cout << int(read_msg_.body()[i]) << ((read_msg_.body()[i + 1] >>  2 ) & 1) << ((read_msg_.body()[i + 1] >>  1 ) & 1) << ((read_msg_.body()[i + 1] >>  0 ) & 1) << std::endl;
                }
                std::cout << "Shots:" << std::endl;
                for (int i = 22; i < 22 + int(reinterpret_cast<const unsigned char&>(read_msg_.body()[00])) * 2; i = i + 2) {
                    std::cout << int(read_msg_.body()[i]) << ((read_msg_.body()[i + 1] >> 0 ) & 1) << ((read_msg_.body()[i + 1] >> 1 ) & 1) << ((read_msg_.body()[i + 1] >> 2 ) & 1) << std::endl;
                }
            }else {
                //std::cout.write(read_msg_.body(), read_msg_.body_length());
                std::cout << int(read_msg_.body()[0]) << ((read_msg_.body()[1] >> 0 ) & 1) << ((read_msg_.body()[1] >> 1 ) & 1) << ((read_msg_.body()[1] >> 2 ) & 1);
                std::cout << "\n";
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
  boost::asio::io_context& io_context_;
  tcp::socket socket_;
  message read_msg_;
  message_queue write_msgs_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: client <host> <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    client *c = new client(io_context, endpoints);

    std::thread t([&io_context](){ io_context.run(); });
    while(true) {
      char header;
      int input1;
      int input2;
      char line[message::max_body_length + 1] = "";
      std::cout << "Please choose Header" << std::endl;
      std::cin >> input1;
      if (input1 == 100) {
        break;
      }
      std::cout << std::endl;

      switch(input1) {
          case 0: header = 0b00000000;
                  break;
          case 1: header = 0b00000001;
                  break;
          case 2: header = 0b00000010;
                  break;
          case 3: header = 0b00000011;
                  break;
          case 4: header = 0b00000100;
                  break;
          case 5: header = 0b00000101;
                  break;
          case 6: header = 0b00000110;
                  break;
          case 127: header = 0b01111111;
                    break;
          case 128: header = 0b10000000;
                    break;
          case 129: header = 0b10000001;
                    break;
          case 130: header = 0b10000010;
                    break;
          case 131: header = 0b10000011;
                    break;
          case 132: header = 0b10000100;
                    break;
          case 133: header = 0b10000101;
                    break;
          case 134: header = 0b10000110;
                    break;
          case 135: header = 0b10000111;
                    break;
          case 136: header = 0b10001000;
                    break;
          case 137: header = 0b10001001;
                    break;
          case 138: header = 0b10001010;
                    break;
          case 139: header = 0b10001011;
                    break;
          case 140: header = 0b10001100;
                    break;
          case 141: header = 0b10001101;
                    break;
          case 255: header = 0b11111111;
                    break;
      }
      message msg;
      msg.encode_header(header);
      if (input1 != 0 && input1 != 1 && input1 != 6 && input1 != 127) {
          std::cout << "Please choose Body" << std::endl;
          if (input1 == 2 || input1 == 5) {
              std::cin >> line;
              std::cout << std::endl;
          } else {
              char x = 0;
              std::cin >> input2;
              line[0] = input2;
              if (input1 == 3) {
                  if (input2 == 0) {
                      x |= 0 << 0;
                      x |= 0 << 1; //battleship
                  } else if (input2 >= 10 && input2 < 30) {
                      x |= 1 << 0;
                      x |= 0 << 1; //cruiser
                  } else if (input2 >= 30 && input2 < 60) {
                      x |= 0 << 0;
                      x |= 1 << 1; //destroyer
                  } else {
                      x |= 1 << 0;
                      x |= 1 << 1; //submarine
                  }
                  x |= 1 << 2; //horizontal
                  line[1] = x;
                  /*std::cout << "choose ship" << std::endl;
                  std::cin >> input2;
                  switch (input2) {
                      case 0: x |= 0 << 0;
                              x |= 0 << 1; //battleship
                              break;
                      case 1: x |= 1 << 0;
                              x |= 0 << 1; //cruiser
                              break;
                      case 2: x |= 0 << 0;
                              x |= 1 << 1; //destroyer
                              break;
                      case 3: x |= 1 << 0;
                              x |= 1 << 1; //submarine
                  }
                  std::cout << "horizontal or vertical?" << std::endl;
                  std::cin >> input2;
                  if (input2 == 1) {
                      x |= 1 << 2; //horizontal
                  } else {
                      x |= 0 << 2; //vertical
                  }
                  line[1] = x;*/
              }
          }
          std::memcpy(msg.body(), line, msg.body_length());
      }
      c -> write(msg);
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    c -> close();
    t.join();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}