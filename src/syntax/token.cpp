#include "nyx/syntax/token.h"

using namespace nyx::syntax;


Token::Token(
  const std::string &v,           // text value from the file
  std::shared_ptr<std::string> f, // file name
  std::shared_ptr<std::string> o, // origin line from the file
  int l,                          // line number
  int c,                          // column in the line where the token starts
  Lexeme x                        // the lexeme type of the token
  ):
  value(v),
  file(f),
  line(o),
  number(l),
  column(c),
  kind(x) {
}


Token::Token(
  std::string &&v,                // text value from the file
  std::shared_ptr<std::string> f, // file name
  std::shared_ptr<std::string> o, // origin line from the file
  int l,                          // line number
  int c,                          // column in the line where the token starts
  Lexeme x                        // the lexeme type of the token
  ):
  value(v),
  file(f),
  line(o),
  number(l),
  column(c),
  kind(x) {
}


bool nyx::syntax::isNumeric(Lexeme lexeme, bool pattern) {
  switch(lexeme) {
    case Lexeme::OctalPattern:
    case Lexeme::BinaryPattern:
    case Lexeme::HexadecimalPattern:
      return pattern;
    break;

    case Lexeme::FloatLiteral:
    case Lexeme::OctalLiteral:
    case Lexeme::BinaryLiteral:
    case Lexeme::DecimalLiteral:
    case Lexeme::HexadecimalLiteral:
      return true;
    break;

    default:
      return false;
    break;
  }
}


bool nyx::syntax::isOperator(Lexeme lexeme) {
  switch(lexeme) {
    case Lexeme::Assignment:
    case Lexeme::BitwiseAnd:
    case Lexeme::BitwiseNot:
    case Lexeme::BitwiseOr:
    case Lexeme::BitwiseXor:
    case Lexeme::CloseAngle:
    case Lexeme::Division:
    case Lexeme::LogicalNot:
    case Lexeme::Minus:
    case Lexeme::Modulo:
    case Lexeme::OpenAngle:
    case Lexeme::Plus:
    case Lexeme::Times:
    case Lexeme::AndAssignment:
    case Lexeme::DivAssignment:
    case Lexeme::Equality:
    case Lexeme::GreaterThanOrEqual:
    case Lexeme::Identifier:
    case Lexeme::Inequality:
    case Lexeme::LeftShift:
    case Lexeme::LessThanOrEqual:
    case Lexeme::LogicalAnd:
    case Lexeme::LogicalOr:
    case Lexeme::LogicalXor:
    case Lexeme::MinusAssignment:
    case Lexeme::ModuloAssignment:
    case Lexeme::OrAssignment:
    case Lexeme::PlusAssignment:
    case Lexeme::RightShift:
    case Lexeme::TimesAssignment:
    case Lexeme::XorAssignment:
      return true;
    break;

    default:
      return false;
    break;
  }
}

