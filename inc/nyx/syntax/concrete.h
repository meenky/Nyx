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


class ConcreteElement {
  protected:
    ConcreteElement(ConcreteElementType);

  public:
    virtual ~ConcreteElement();

    inline ConcreteElementType type() const {
      return kind;
    }

    virtual std::ostream &print(std::ostream &os) const = 0;
    virtual std::ostream &debug(std::ostream &os) const = 0;

    virtual bool isValid() const = 0;
    virtual bool is(Lexeme) const;

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

    inline std::shared_ptr<Token> token() {
      return value;
    }

    inline std::shared_ptr<const Token> token() const {
      return value;
    }

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    virtual bool isValid() const;
    virtual bool is(Lexeme) const;

  private:
    std::shared_ptr<Token> value;
};


class ConcreteCompoundElement: public ConcreteElement {
  public:
    virtual ~ConcreteCompoundElement();

    typedef std::vector<std::shared_ptr<ConcreteElement>>::iterator iterator;
    typedef std::vector<std::shared_ptr<ConcreteElement>>::const_iterator const_iterator;
    typedef std::vector<std::shared_ptr<ConcreteElement>>::reverse_iterator reverse_iterator;
    typedef std::vector<std::shared_ptr<ConcreteElement>>::const_reverse_iterator const_reverse_iterator;

    inline iterator               begin()         { children.begin(); }
    inline iterator               end()           { children.end();   }
    inline const_iterator         begin()   const { children.begin(); }
    inline const_iterator         end()     const { children.end();   }
    inline const_iterator         cbegin()  const { children.begin(); }
    inline const_iterator         cend()    const { children.end();   }
    inline reverse_iterator       rbegin()        { children.begin(); }
    inline reverse_iterator       rend()          { children.end();   }
    inline const_reverse_iterator rbegin()  const { children.begin(); }
    inline const_reverse_iterator rend()    const { children.end();   }
    inline const_reverse_iterator crbegin() const { children.begin(); }
    inline const_reverse_iterator crend()   const { children.end();   }

    inline std::vector<std::shared_ptr<ConcreteElement>>::size_type size() const {
      return children.size();
    }

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    virtual bool isValid() const;
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

    virtual bool isValid() const;
};


class ConcreteSexprElement: public ConcreteCompoundElement {
  public:
    ConcreteSexprElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteSexprElement();

    virtual bool isValid() const;
};


class ConcreteAliasElement: public ConcreteCompoundElement {
  public:
    ConcreteAliasElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteAliasElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteBoundElement: public ConcreteCompoundElement {
  public:
    ConcreteBoundElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteBoundElement();

    //virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteCommentElement: public ConcreteCompoundElement {
  public:
    ConcreteCommentElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteCommentElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;

  protected:
    ConcreteCommentElement(ConcreteElementType, const std::vector<std::shared_ptr<ConcreteElement>> &);
};


class ConcreteDecodeElement: public ConcreteCompoundElement {
  public:
    ConcreteDecodeElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteDecodeElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteDocumentationElement: public ConcreteCommentElement {
  public:
    ConcreteDocumentationElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteDocumentationElement();

    virtual bool isValid() const;
};


class ConcreteEncodeElement: public ConcreteCompoundElement {
  public:
    ConcreteEncodeElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteEncodeElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteImportElement: public ConcreteCompoundElement {
  public:
    ConcreteImportElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteImportElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteListElement: public ConcreteCompoundElement {
  public:
    ConcreteListElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteListElement();

    //virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteMatchElement: public ConcreteCompoundElement {
  public:
    ConcreteMatchElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteMatchElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteModuleElement: public ConcreteCompoundElement {
  public:
    ConcreteModuleElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteModuleElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteNamespaceElement: public ConcreteCompoundElement {
  public:
    ConcreteNamespaceElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteNamespaceElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcretePatternElement: public ConcreteCompoundElement {
  public:
    ConcretePatternElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcretePatternElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteRepetitionElement: public ConcreteCompoundElement {
  public:
    ConcreteRepetitionElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteRepetitionElement();

    //virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteRuleElement: public ConcreteCompoundElement {
  public:
    ConcreteRuleElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteRuleElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteStorageElement: public ConcreteCompoundElement {
  public:
    ConcreteStorageElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteStorageElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteValidateElement: public ConcreteCompoundElement {
  public:
    ConcreteValidateElement(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteValidateElement();

    virtual std::ostream &print(std::ostream &os) const;

    virtual bool isValid() const;
};


class ConcreteSyntaxTree: public ConcreteCompoundElement {
  public:
    ConcreteSyntaxTree(const std::vector<std::shared_ptr<ConcreteElement>> &);
    virtual ~ConcreteSyntaxTree();

    virtual std::ostream &print(std::ostream &os) const;
};


  }
}

