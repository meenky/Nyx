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


#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
#define PRINT_ENUM(e) case Lexeme::e: return STRINGIFY(e); break

const char *nyx::syntax::toString(Lexeme lexeme) {
  switch(lexeme) {
    PRINT_ENUM(Assignment);
    PRINT_ENUM(BitwiseAnd);
    PRINT_ENUM(BitwiseNot);
    PRINT_ENUM(BitwiseOr);
    PRINT_ENUM(BitwiseXor);
    PRINT_ENUM(CloseAngle);
    PRINT_ENUM(CloseCurly);
    PRINT_ENUM(CloseParen);
    PRINT_ENUM(CloseSquare);
    PRINT_ENUM(Comma);
    PRINT_ENUM(Division);
    PRINT_ENUM(Dot);
    PRINT_ENUM(EndOfLine);
    PRINT_ENUM(LogicalNot);
    PRINT_ENUM(Minus);
    PRINT_ENUM(Modulo);
    PRINT_ENUM(OpenAngle);
    PRINT_ENUM(OpenCurly);
    PRINT_ENUM(OpenParen);
    PRINT_ENUM(OpenSquare);
    PRINT_ENUM(Plus);
    PRINT_ENUM(Query);
    PRINT_ENUM(Times);
    PRINT_ENUM(Alias);
    PRINT_ENUM(AndAssignment);
    PRINT_ENUM(BinaryLiteral);
    PRINT_ENUM(BinaryPattern);
    PRINT_ENUM(Bind);
    PRINT_ENUM(Comment);
    PRINT_ENUM(DecimalLiteral);
    PRINT_ENUM(Decode);
    PRINT_ENUM(DivAssignment);
    PRINT_ENUM(DocEnd);
    PRINT_ENUM(DocStart);
    PRINT_ENUM(Encode);
    PRINT_ENUM(Equality);
    PRINT_ENUM(FloatLiteral);
    PRINT_ENUM(GreaterThanOrEqual);
    PRINT_ENUM(HexadecimalLiteral);
    PRINT_ENUM(HexadecimalPattern);
    PRINT_ENUM(Identifier);
    PRINT_ENUM(Import);
    PRINT_ENUM(Inequality);
    PRINT_ENUM(LeftShift);
    PRINT_ENUM(LessThanOrEqual);
    PRINT_ENUM(LogicalAnd);
    PRINT_ENUM(LogicalOr);
    PRINT_ENUM(LogicalXor);
    PRINT_ENUM(Match);
    PRINT_ENUM(MinusAssignment);
    PRINT_ENUM(Module);
    PRINT_ENUM(ModuloAssignment);
    PRINT_ENUM(Namespace);
    PRINT_ENUM(OctalLiteral);
    PRINT_ENUM(OctalPattern);
    PRINT_ENUM(OrAssignment);
    PRINT_ENUM(Pattern);
    PRINT_ENUM(PlusAssignment);
    PRINT_ENUM(RightShift);
    PRINT_ENUM(Storage);
    PRINT_ENUM(StringLiteral);
    PRINT_ENUM(TimesAssignment);
    PRINT_ENUM(Validate);
    PRINT_ENUM(XorAssignment);
  }

  return "<INVALID>";
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


namespace nyx {
  namespace syntax {

std::ostream &operator<<(std::ostream &os, const Token &tok) {
  os << tok.text();

  return os;
}


std::ostream &operator<<(std::ostream &os, const std::shared_ptr<const Token> &tok) {
  if(tok) {
    os << *tok;
  }
  else {
    os << "(null)";
  }

  return os;
}


  }
}
