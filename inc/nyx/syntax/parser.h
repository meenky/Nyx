#pragma once

#include "nyx/syntax/abstract.h"
#include "nyx/syntax/concrete.h"
#include "nyx/syntax/tokenizer.h"

#include <memory>


namespace nyx {
  namespace syntax {


class Parser {
  public:
    Parser() = delete;

    static std::unique_ptr<ConcreteSyntaxTree>
    concreteParse(Tokenizer &tokenizer);

    static std::unique_ptr<AbstractSyntaxTree>
    abstractParse(ConcreteSyntaxTree &concrete);

    static std::unique_ptr<AbstractSyntaxTree>
    parse(Tokenizer &tokenizer);
};


  }
}

