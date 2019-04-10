#include "nyx/syntax/tokenizer.h"

#include <array>
#include <cctype>
#include <algorithm>


using namespace nyx::syntax;


Tokenizer::Tokenizer(std::shared_ptr<std::string> fileName):
  source(*fileName),
  file(fileName),
  line(nullptr),
  number(0),
  column(0) {
  readLine(); // prime the pump
}


std::shared_ptr<Token> Tokenizer::next() {
  // keep going as long as there is a line of text to process
  while(line) {
    const auto &str = *line;
    const auto  len = static_cast<int>(str.size());

    // skip any leading whitespace
    while(column < len && std::isspace(str[column])) {
      ++column;
    }

    // check to see if the line has been exhausted
    if(column >= len) {
      auto token = emitToken(Lexeme::EndOfLine, 0);
      readLine();
      return token; // always emit an end of line token even if missing in the file
    }

    switch(str[column]) {
      // check for single character tokens
      case '~':  return emitToken(Lexeme::BitwiseNot,  1); break;
      case ',':  return emitToken(Lexeme::Comma,       1); break;
      case '?':  return emitToken(Lexeme::Query,       1); break;
      case '{':  return emitToken(Lexeme::OpenCurly,   1); break;
      case '}':  return emitToken(Lexeme::CloseCurly,  1); break;
      case '[':  return emitToken(Lexeme::OpenSquare,  1); break;
      case ']':  return emitToken(Lexeme::CloseSquare, 1); break;
      case '(':  return emitToken(Lexeme::OpenParen,   1); break;
      case ')':  return emitToken(Lexeme::CloseParen,  1); break;

      // multicharacter tokens that start with a specific character
      case '#':  return parseComment(str, len);            break;
      case '@':  return parseKeyword(str, len);            break;
      case '"':
      case '\'': return parseString(str, len);             break;

      // ambiguous single/multi character tokens
      case '.': {
        int end = column + 1;
        if(end < len && std::isdigit(str[end])) {
          return parseNumber(str, len);
        }
        else {
          return emitToken(Lexeme::Dot, 1);
        }
      } break;
      case '-':
      case '+': {
        int end = column + 1;

        if(end < len && (std::isdigit(str[end]) || str[end] == '.')) {
          return parseNumber(str, len);
        }

        return parseOperator(str, len);
      } break;

      case '^':
      case '*':
      case '/':
      case '%':
      case '!':
      case '=':
      case '&':
      case '|':
      case '<':
      case '>':
        return parseOperator(str, len);
      break;
    }

    // check for an integer (hex/dec/octal/bin), floating point, or pattern
    if(std::isdigit(str[column])) {
      return parseNumber(str, len);
    }

    // check for identifiers
    if(str[column] == '_' || std::isalpha(str[column])) {
      return parseIdentifierOrLabel(str, len);
    }

    // matches nothing, return something invalid
    return emitToken(Lexeme::INVALID, 1);
  }

  // nothing to parse
  return nullptr;
}


std::shared_ptr<Token> Tokenizer::parseComment(const std::string &str, int len) {
  // checking for special comment markers
  if(len - column >= 3) {
    if(str[column + 1] == '+' && str[column + 2] == '+') {
      return emitToken(Lexeme::DocStart, len - column);
    }
    if(str[column + 1] == '-' && str[column + 2] == '-') {
      return emitToken(Lexeme::DocEnd, len - column);
    }
  }

  return emitToken(Lexeme::Comment, len - column);
}


std::shared_ptr<Token> Tokenizer::parseKeyword(const std::string &str, int len) {
  int end = column + 1;
  while(end < len && std::isalpha(str[end])) {
    ++end;
  }

  return emitToken(end - column);
}


std::shared_ptr<Token> Tokenizer::parseIdentifierOrLabel(const std::string &str, int len) {
  int end = column + 1;
  while(end < len && (str[end] == '_' || std::isalnum(str[end]) || str[end] == '-')) {
    ++end;
  }

  // check for labels
  if(end < len && str[end] == ':') {
    return emitToken(end - column + 1);
  }

  return emitToken(Lexeme::Identifier, end - column);
}


std::shared_ptr<Token> Tokenizer::parseString(const std::string &str, int len) {
  for(int end = column + 1; end < len; ++end) {
    if(str[end] == str[column]) {
      return emitToken(Lexeme::StringLiteral, end - column + 1);
    }
  }

  // unterminated string literal?
  return emitToken(Lexeme::INVALID, len - column);
}


std::shared_ptr<Token> Tokenizer::parseOperator(const std::string &str, int len) {
  switch(str[column]) {
    // stand alone/doubled up/_ equals
    case '+':
    case '-':
    case '&':
    case '|':
    case '^':
    case '<':
    case '>': {
      int end = column + 1;
      if(end < len && (str[end] == str[column] || str[end] == '=')) {
        ++end;
      }

      return emitToken(end - column);
    } break;

    // stand alone/_ equals
    case '!':
    case '*':
    case '/':
    case '%': {
      int end = column + 1;
      if(end < len && str[end] == '=') {
        ++end;
      }

      return emitToken(end - column);
    } break;

    case '=': {
      int end = column + 1;
      if(end < len && (str[end] == '=' || str[end] == '>')) {
        ++end;
      }

      return emitToken(end - column);
    } break;
  }

  return emitToken(Lexeme::INVALID, 1);
}


enum class NumberState {
  ERROR,
  Start,
  Sign,
  Zero,
  Decimal,
  Octal,
  OctalPattern,
  BinaryStart,
  Binary,
  BinaryPattern,
  HexadecimalStart,
  Hexadecimal,
  HexadecimalPattern,
  FractionStart,
  Fraction,
  ExponentStart,
  ExponentSign,
  Exponent
};


static const std::array<std::string::value_type, 24> DELIMITERS{
  '!', '#', '/', '%', '*', '-', '+', '(', ')', '[', ']', '{', '}',
  ',', '~', '=', '<', '>', '&', '|', '^', '@', '"', '\''
};


template<typename T, std::array<int, 0>::size_type N>
static bool contains(const std::array<T, N> &arr, const T &val) {
  return std::find(std::begin(arr), std::end(arr), val) != std::end(arr);
}


std::shared_ptr<Token> Tokenizer::parseNumber(const std::string &str, int len) {
  NumberState state = NumberState::Start;

  // state machine to validate that the input is some form of valid number
  for(int end = column; end < len; ++end) {
    switch(state) {
      case NumberState::Start:
        if(str[end] == '+' || str[end] == '-') {
          state = NumberState::Sign;
        }
        else if(str[end] == '0') {
          state = NumberState::Zero;
        }
        else if('1' <= str[end] && str[end] <= '9') {
          state = NumberState::Decimal;
        }
        else {
          goto error;
        }
      break;

      case NumberState::Sign:
        if(str[end] == '0') {
          state = NumberState::Zero;
        }
        else if('1' <= str[end] && str[end] <= '9') {
          state = NumberState::Decimal;
        }
        else {
          goto error;
        }
      break;

      case NumberState::Zero:
        if(str[end] == 'B' || str[end] == 'b') {
          state = NumberState::BinaryStart;
        }
        else if(str[end] == 'X' || str[end] == 'x') {
          state = NumberState::HexadecimalStart;
        }
        else if('0' <= str[end] && str[end] <= '7') {
          state = NumberState::Octal;
        }
        else if(str[end] == '*') {
          state = NumberState::OctalPattern;
        }
        else if(str[end] == '.') {
          state = NumberState::FractionStart;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::DecimalLiteral, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::Decimal:
        if('0' <= str[end] && str[end] <= '9') {
          state = NumberState::Decimal;
        }
        else if(str[end] == '.') {
          state = NumberState::FractionStart;
        }
        else if(str[end] == 'E' || str[end] == 'e') {
          state = NumberState::Exponent;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::DecimalLiteral, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::Octal:
        if('0' <= str[end] && str[end] <= '7') {
          state = NumberState::Octal;
        }
        else if(str[end] == '*') {
          state = NumberState::OctalPattern;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::OctalLiteral, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::OctalPattern:
        if(str[end] == '*' || ('0' <= str[end] && str[end] <= '7')) {
          state = NumberState::Octal;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::OctalPattern, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::BinaryStart:
        if(str[end] == '0' || str[end] == '1') {
          state = NumberState::Binary;
        }
        else if(str[end] == '*') {
          state = NumberState::BinaryPattern;
        }
        else {
          goto error;
        }
      break;

      case NumberState::Binary:
        if(str[end] == '0' || str[end] == '1') {
          state = NumberState::Binary;
        }
        else if(str[end] == '*') {
          state = NumberState::BinaryPattern;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::BinaryLiteral, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::BinaryPattern:
        if(str[end] == '0' || str[end] == '1' || str[end] == '*') {
          state = NumberState::BinaryPattern;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::BinaryPattern, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::HexadecimalStart:
        if(('0' <= str[end] && str[end] <= '9') ||
           ('A' <= str[end] && str[end] <= 'F') ||
           ('a' <= str[end] && str[end] <= 'f')) {
          state = NumberState::Hexadecimal;
        }
        else if(str[end] == '*') {
          state = NumberState::HexadecimalPattern;
        }
        else {
          goto error;
        }
      break;

      case NumberState::Hexadecimal:
        if(('0' <= str[end] && str[end] <= '9') ||
           ('A' <= str[end] && str[end] <= 'F') ||
           ('a' <= str[end] && str[end] <= 'f')) {
          state = NumberState::Hexadecimal;
        }
        else if(str[end] == '*') {
          state = NumberState::HexadecimalPattern;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::HexadecimalLiteral, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::HexadecimalPattern:
        if(('0' <= str[end] && str[end] <= '9') ||
           ('A' <= str[end] && str[end] <= 'F') ||
           ('a' <= str[end] && str[end] <= 'f') ||
           str[end] == '*') {
          state = NumberState::HexadecimalPattern;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::HexadecimalPattern, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::FractionStart:
        if('0' <= str[end] && str[end] <= '9') {
          state = NumberState::Fraction;
        }
        else {
          goto error;
        }
      break;

      case NumberState::Fraction:
        if('0' <= str[end] && str[end] <= '9') {
          state = NumberState::Fraction;
        }
        else if(str[end] == 'E' || str[end] == 'e') {
          state = NumberState::Exponent;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::FloatLiteral, end - column);
        }
        else {
          goto error;
        }
      break;

      case NumberState::ExponentStart:
        if(str[end] == '-' || str[end] == '+') {
          state = NumberState::ExponentSign;
        }
        else if('0' <= str[end] && str[end] <= '9') {
          state = NumberState::Exponent;
        }
        else {
          goto error;
        }
      break;

      case NumberState::ExponentSign:
        if('0' <= str[end] && str[end] <= '9') {
          state = NumberState::Exponent;
        }
        else {
          goto error;
        }
      break;

      case NumberState::Exponent:
        if('0' <= str[end] && str[end] <= '9') {
          state = NumberState::Exponent;
        }
        else if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::FloatLiteral, end - column);
        }
        else {
          goto error;
        }
      break;

      error:
      case NumberState::ERROR:
        state = NumberState::ERROR;
        if(std::isspace(str[end]) || contains(DELIMITERS, str[end])) {
          return emitToken(Lexeme::INVALID, end - column);
        }
      break;
    }
  }

  // ran out of input, emit a token
  switch(state) {
    case NumberState::Zero:
    case NumberState::Decimal:
      return emitToken(Lexeme::DecimalLiteral, len - column);
    break;

    case NumberState::Octal:
      return emitToken(Lexeme::OctalLiteral, len - column);
    break;

    case NumberState::OctalPattern:
      return emitToken(Lexeme::OctalPattern, len - column);
    break;

    case NumberState::Binary:
      return emitToken(Lexeme::BinaryLiteral, len - column);
    break;

    case NumberState::BinaryPattern:
      return emitToken(Lexeme::BinaryPattern, len - column);
    break;

    case NumberState::Hexadecimal:
      return emitToken(Lexeme::HexadecimalLiteral, len - column);
    break;

    case NumberState::HexadecimalPattern:
      return emitToken(Lexeme::HexadecimalPattern, len - column);
    break;

    case NumberState::Fraction:
    case NumberState::Exponent:
      return emitToken(Lexeme::FloatLiteral, len - column);
    break;

    // all the failure states
    case NumberState::ERROR:
    case NumberState::Start:
    case NumberState::Sign:
    case NumberState::BinaryStart:
    case NumberState::HexadecimalStart:
    case NumberState::FractionStart:
    case NumberState::ExponentStart:
    case NumberState::ExponentSign:
      goto failthrough;
    break;
  }

failthrough:
  return emitToken(Lexeme::INVALID, len - column);
}


bool Tokenizer::readLine() {
  // ensure file stream is in good shape
  if(source) {
    // make a new string
    line = std::make_shared<std::string>();

    if(std::getline(source, *line)) {
      column = 0;
      ++number;
      return true;
    }

    // no line to process
    line.reset();
  }

  return false;
}


const std::map<std::string, Lexeme> Tokenizer::KEYWORDS = {
 { "@alias",     Lexeme::Alias              },
 { "&=",         Lexeme::AndAssignment      },
 { "=",          Lexeme::Assignment         },
 { "=>",         Lexeme::Bind               },
 { "&",          Lexeme::BitwiseAnd         },
 { "~",          Lexeme::BitwiseNot         },
 { "|",          Lexeme::BitwiseOr          },
 { "^",          Lexeme::BitwiseXor         },
 { "decode:",    Lexeme::Decode             },
 { "/=",         Lexeme::DivAssignment      },
 { "/",          Lexeme::Division           },
 { "encode:",    Lexeme::Encode             },
 { "==",         Lexeme::Equality           },
 { ">",          Lexeme::CloseAngle         },
 { ">=",         Lexeme::GreaterThanOrEqual },
 { "@import",    Lexeme::Import             },
 { "!=",         Lexeme::Inequality         },
 { "<<",         Lexeme::LeftShift          },
 { "<",          Lexeme::OpenAngle          },
 { "<=",         Lexeme::LessThanOrEqual    },
 { "&&",         Lexeme::LogicalAnd         },
 { "!",          Lexeme::LogicalNot         },
 { "||",         Lexeme::LogicalOr          },
 { "^^",         Lexeme::LogicalXor         },
 { "@match",     Lexeme::Match              },
 { "-",          Lexeme::Minus              },
 { "-=",         Lexeme::MinusAssignment    },
 { "@module",    Lexeme::Module             },
 { "%",          Lexeme::Modulo             },
 { "%=",         Lexeme::ModuloAssignment   },
 { "@namespace", Lexeme::Namespace          },
 { "|=",         Lexeme::OrAssignment       },
 { "pattern:",   Lexeme::Pattern            },
 { "+",          Lexeme::Plus               },
 { "+=",         Lexeme::PlusAssignment     },
 { ">>",         Lexeme::RightShift         },
 { "storage:",   Lexeme::Storage            },
 { "*",          Lexeme::Times              },
 { "*=",         Lexeme::TimesAssignment    },
 { "validate:",  Lexeme::Validate           },
 { "^=",         Lexeme::XorAssignment      },
};


std::shared_ptr<Token> Tokenizer::emitToken(int length) {
  auto str = line->substr(column, length);
  auto iter = KEYWORDS.find(str);
  auto token = std::make_shared<Token>(
                 str,
                 file,
                 line,
                 number,
                 column,
                 iter != KEYWORDS.end() ? iter->second : Lexeme::INVALID
               );

  column += length;

  return token;
}


std::shared_ptr<Token> Tokenizer::emitToken(Lexeme lexeme, int length) {
  auto token = std::make_shared<Token>(
                 line->substr(column, length),
                 file,
                 line,
                 number,
                 column,
                 lexeme
               );

  column += length;

  return token;
}

