//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class chat_message
{
public:
  enum { header_length = 1 };
  enum { max_body_length = 512 };

  chat_message()
    : body_length_(0)
  {
  }

  const char* data() const
  {
    return data_;
  }

  char* data()
  {
    return data_;
  }

  std::size_t length() const
  {
    return header_length + body_length_;
  }

  const char* body() const
  {
    return data_ + header_length;
  }

  char* body()
  {
    return data_ + header_length;
  }

  std::size_t body_length() const
  {
    return body_length_;
  }

  void body_length(std::size_t new_length)
  {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_body_length;
  }

    void setBodyLength(char &header, size_t &variableLength)
    {
        switch(header) {
            case 0: body_length(0);
                    break;
            case 1: body_length(0);
                    break;
            case 2: body_length(16);
                    break;
            case 3: body_length(2);
                    break;
            case 4: body_length(1);
                    break;
            case 5: body_length(16);
                    break;
            case 127: body_length(0);
                    break;
            case 128: body_length(0);
                    break;
            case 129: body_length(16);
                    break;
            case 130: body_length(0);
                    break;
            case 131: body_length(0);
                    break;
            case 132: body_length(1);
                    break;
            case 133: body_length(0);
                    break;
            case 134: body_length(2);
                    break;
            case 135: body_length(2);
                    break;
            case 136: body_length(21 + variableLength);
                    break;
            case 255: body_length(0);
                    break;
            default: body_length(0);
        }

    }

  bool decode_header(size_t variableLength = 0)
  {
    char header = data_[0];
    setBodyLength(header, variableLength);
    if (body_length_ > max_body_length)
    {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  void encode_header(char header, size_t variableLength = 0)
  {
    setBodyLength(header, variableLength);
    std::memcpy(data_, &header, header_length);
  }

private:
  char data_[header_length + max_body_length];
  std::size_t body_length_;
};

#endif // CHAT_MESSAGE_HPP