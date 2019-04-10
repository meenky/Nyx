#pragma once

#include "nyx/syntax/token.h"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <fstream>


namespace nyx {
  namespace syntax {


class Tokenizer {
  public:
    Tokenizer(std::shared_ptr<std::string>  name);

    std::shared_ptr<Token> next();

    operator bool() const {
      return static_cast<bool>(source);
    }

    bool good() const {
      return source.good();
    }

    bool eof() const {
      return source.eof();
    }

    bool fail() const {
      return source.fail();
    }

    bool bad() const {
      return source.bad();
    }


  private:
    static const std::map<std::string, Lexeme> KEYWORDS;

    std::ifstream                source;
    std::shared_ptr<std::string> file;
    std::shared_ptr<std::string> line;
    int                          number;
    int                          column;

    bool                   readLine();
    std::shared_ptr<Token> parseComment(const std::string &str, int len);
    std::shared_ptr<Token> parseKeyword(const std::string &str, int len);
    std::shared_ptr<Token> parseIdentifierOrLabel(const std::string &str, int len);
    std::shared_ptr<Token> parseString(const std::string &str, int len);
    std::shared_ptr<Token> parseOperator(const std::string &str, int len);
    std::shared_ptr<Token> parseNumber(const std::string &str, int len);
    std::shared_ptr<Token> emitToken(int length);
    std::shared_ptr<Token> emitToken(Lexeme lexeme, int length);
};


  }
}

