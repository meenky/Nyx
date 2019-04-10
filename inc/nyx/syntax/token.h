#pragma once


#include <memory>
#include <string>


namespace nyx {
  namespace syntax {


enum class Lexeme: int {
  INVALID     = -1, // not really a lexeme. something broke the parser
  Assignment  = '=',
  BitwiseAnd  = '&',
  BitwiseNot  = '~',
  BitwiseOr   = '|',
  BitwiseXor  = '^',
  CloseAngle  = '>',
  CloseCurly  = '}',
  CloseParen  = ')',
  CloseSquare = ']',
  Comma       = ',',
  Division    = '/',
  Dot         = '.',
  EndOfLine   = '\n',
  LogicalNot  = '!',
  Minus       = '-',
  Modulo      = '%',
  OpenAngle   = '<',
  OpenCurly   = '{',
  OpenParen   = '(',
  OpenSquare  = '[',
  Plus        = '+',
  Query       = '?',
  Times       = '*',
  Alias       = 256,
  AndAssignment,
  BinaryLiteral,
  BinaryPattern,
  Bind,
  Comment,
  DecimalLiteral,
  Decode,
  DivAssignment,
  DocEnd,
  DocStart,
  Encode,
  Equality,
  FloatLiteral,
  GreaterThanOrEqual,
  HexadecimalLiteral,
  HexadecimalPattern,
  Identifier,
  Import,
  Inequality,
  LeftShift,
  LessThanOrEqual,
  LogicalAnd,
  LogicalOr,
  LogicalXor,
  Match,
  MinusAssignment,
  Module,
  ModuloAssignment,
  Namespace,
  OctalLiteral,
  OctalPattern,
  OrAssignment,
  Pattern,
  PlusAssignment,
  RightShift,
  Storage,
  StringLiteral,
  TimesAssignment,
  Validate,
  XorAssignment,
};


bool isNumeric(Lexeme lexeme, bool pattern = false);
bool isOperator(Lexeme lexeme);


class Token {
  public:
    Token(
      const std::string &value,
      std::shared_ptr<std::string> file,
      std::shared_ptr<std::string> source,
      int line,
      int column,
      Lexeme lexeme
    );

    Token(
      std::string &&value,
      std::shared_ptr<std::string> file,
      std::shared_ptr<std::string> source,
      int line,
      int column,
      Lexeme lexeme
    );

    inline int lineNumber() const noexcept {
      return number;
    }

    inline int columnNumber() const noexcept {
      return column;
    }

    inline const std::string &fileName() const {
      return *file;
    }

    inline const std::string &fullLine() const {
      return *line;
    }

    inline const std::string &text() const noexcept {
      return value;
    }

    inline Lexeme lexeme() const noexcept {
      return kind;
    }

    inline bool is(Lexeme lexeme) const noexcept {
      return kind == lexeme;
    }

    inline operator const std::string() const noexcept {
      return text();
    }

  private:
    std::string                  value;
    std::shared_ptr<std::string> file;
    std::shared_ptr<std::string> line;
    int                          number;
    int                          column;
    Lexeme                       kind;
};

  }
}

