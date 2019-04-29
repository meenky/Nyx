#pragma once

#include "nyx/syntax/token.h"

#include <memory>
#include <vector>
#include <iostream>


namespace nyx {
  namespace syntax {


enum class ConcreteElementType {
  Alias,
  Bound,
  Comment,
  Decode,
  Documentation,
  Encode,
  Identifier,
  Import,
  List,
  Match,
  Module,
  Namespace,
  Pattern,
  Repetition,
  Root,
  Rule,
  SExpr,
  Storage,
  Token,
  Validate
};


const char *stringify(ConcreteElementType cet);


class ConcreteElement {
  protected:
    ConcreteElement(ConcreteElementType);

  public:
    virtual ~ConcreteElement();

    inline ConcreteElementType type() const {
      return kind;
    }

    virtual int line() const = 0;
    virtual int column() const = 0;
    virtual const std::string &file() const = 0;
    virtual std::ostream &print(std::ostream &os) const = 0;
    virtual std::ostream &debug(std::ostream &os) const = 0;

    virtual std::vector<std::shared_ptr<ConcreteElement>>::size_type size() const;

    virtual bool is(Lexeme) const = 0;

    inline bool is(ConcreteElementType type) const {
      return kind == type;
    }

    inline bool is(ConcreteElementType type, Lexeme lexeme) const {
      return is(type) && is(lexeme);
    }

  private:
    ConcreteElementType kind;
};


std::ostream &operator<<(std::ostream &os, const ConcreteElement &ce);
std::ostream &operator<<(std::ostream &os, const std::shared_ptr<const ConcreteElement> &ce);


class ConcreteTokenElement: public ConcreteElement {
  public:
    ConcreteTokenElement(std::shared_ptr<Token> token);
    virtual ~ConcreteTokenElement();

    inline auto token() {
      return value;
    }

    inline auto token() const {
      return value;
    }

    virtual int line() const;
    virtual int column() const;
    virtual const std::string &file() const;
    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    virtual bool is(Lexeme) const;

  private:
    std::shared_ptr<Token> value;
};


class ConcreteCompoundElement: public ConcreteElement {
  public:
    virtual ~ConcreteCompoundElement();

    typedef std::vector<std::shared_ptr<ConcreteElement>>::size_type              size_type;
    typedef std::vector<std::shared_ptr<ConcreteElement>>::iterator               iterator;
    typedef std::vector<std::shared_ptr<ConcreteElement>>::const_iterator         const_iterator;
    typedef std::vector<std::shared_ptr<ConcreteElement>>::reverse_iterator       reverse_iterator;
    typedef std::vector<std::shared_ptr<ConcreteElement>>::const_reverse_iterator const_reverse_iterator;

    inline iterator               begin()         { children.begin();   }
    inline iterator               end()           { children.end();     }
    inline const_iterator         begin()   const { children.begin();   }
    inline const_iterator         end()     const { children.end();     }
    inline const_iterator         cbegin()  const { children.cbegin();  }
    inline const_iterator         cend()    const { children.cend();    }
    inline reverse_iterator       rbegin()        { children.rbegin();  }
    inline reverse_iterator       rend()          { children.rend();    }
    inline const_reverse_iterator rbegin()  const { children.rbegin();  }
    inline const_reverse_iterator rend()    const { children.rend();    }
    inline const_reverse_iterator crbegin() const { children.crbegin(); }
    inline const_reverse_iterator crend()   const { children.crend();   }

    virtual size_type size() const;

    auto &operator[](size_type idx) {
      return children[idx];
    }

    auto &operator[](size_type idx) const {
      return children[idx];
    }

    virtual int line() const;
    virtual int column() const;
    virtual const std::string &file() const;

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    virtual bool is(Lexeme) const;

  protected:
    ConcreteCompoundElement(ConcreteElementType, const std::vector<std::shared_ptr<ConcreteElement>> &);

    static std::vector<std::shared_ptr<ConcreteElement>> &
    getPeerChildren(ConcreteCompoundElement *);

    static const std::vector<std::shared_ptr<ConcreteElement>> &
    getPeerChildren(const ConcreteCompoundElement *);

    std::vector<std::shared_ptr<ConcreteElement>> children;
};


class ConcreteIdentifierElement: public ConcreteCompoundElement {
  public:
    ConcreteIdentifierElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteIdentifierElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteSexprElement: public ConcreteCompoundElement {
  public:
    ConcreteSexprElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteSexprElement();
};


class ConcreteAliasElement: public ConcreteCompoundElement {
  public:
    ConcreteAliasElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteAliasElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteBoundElement: public ConcreteCompoundElement {
  public:
    ConcreteBoundElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteBoundElement();

    //virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteCommentElement: public ConcreteCompoundElement {
  public:
    ConcreteCommentElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteCommentElement();

    virtual std::ostream &print(std::ostream &os) const;

  protected:
    ConcreteCommentElement(ConcreteElementType, const std::vector<std::shared_ptr<ConcreteElement>> &);
};


class ConcreteDecodeElement: public ConcreteCompoundElement {
  public:
    ConcreteDecodeElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteDecodeElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteDocumentationElement: public ConcreteCommentElement {
  public:
    ConcreteDocumentationElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteDocumentationElement();
};


class ConcreteEncodeElement: public ConcreteCompoundElement {
  public:
    ConcreteEncodeElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteEncodeElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteImportElement: public ConcreteCompoundElement {
  public:
    ConcreteImportElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteImportElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteListElement: public ConcreteCompoundElement {
  public:
    ConcreteListElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteListElement();

    //virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteMatchElement: public ConcreteCompoundElement {
  public:
    ConcreteMatchElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteMatchElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteModuleElement: public ConcreteCompoundElement {
  public:
    ConcreteModuleElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteModuleElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteNamespaceElement: public ConcreteCompoundElement {
  public:
    ConcreteNamespaceElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteNamespaceElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcretePatternElement: public ConcreteCompoundElement {
  public:
    ConcretePatternElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcretePatternElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteRepetitionElement: public ConcreteCompoundElement {
  public:
    ConcreteRepetitionElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteRepetitionElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteRuleElement: public ConcreteCompoundElement {
  public:
    ConcreteRuleElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteRuleElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteStorageElement: public ConcreteCompoundElement {
  public:
    ConcreteStorageElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteStorageElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteValidateElement: public ConcreteCompoundElement {
  public:
    ConcreteValidateElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteValidateElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class ConcreteSyntaxTree: public ConcreteCompoundElement {
  public:
    ConcreteSyntaxTree(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteSyntaxTree();

    virtual std::ostream &print(std::ostream &os) const;
};


  }
}

