#pragma once

#include "nyx/syntax/token.h"

#include <map>
#include <memory>
#include <vector>
#include <iostream>


namespace nyx {
  namespace syntax {


enum class AbstractElementType {
  Alias,
  AliasList,
  Code,
  Identifier,
  Import,
  ImportList,
  Module,
  Namespace,
  Pattern,
  PatternElement,
  PatternGroup,
  PatternList,
  RepeatedPattern,
  Rule,
  StorageElement,
  StorageList
};


class AbstractElement {
  protected:
    AbstractElement(AbstractElementType);

  public:
    virtual ~AbstractElement();

    inline AbstractElementType type() const {
      return kind;
    }

    virtual std::ostream &print(std::ostream &os) const = 0;
    virtual std::ostream &debug(std::ostream &os) const = 0;

    inline bool is(AbstractElementType type) const {
      return kind == type;
    }

  private:
    AbstractElementType kind;
};


std::ostream &operator<<(std::ostream &os, const AbstractElement &ce);
std::ostream &operator<<(std::ostream &os, const std::shared_ptr<const AbstractElement> &ce);


class AbstractMultiTokenElement: public AbstractElement {
  public:
    virtual ~AbstractMultiTokenElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    typedef std::vector<std::shared_ptr<Token>>::iterator iterator;
    typedef std::vector<std::shared_ptr<Token>>::const_iterator const_iterator;
    typedef std::vector<std::shared_ptr<Token>>::reverse_iterator reverse_iterator;
    typedef std::vector<std::shared_ptr<Token>>::const_reverse_iterator const_reverse_iterator;

    inline iterator               begin()         { tokens.begin();   }
    inline iterator               end()           { tokens.end();     }
    inline const_iterator         begin()   const { tokens.begin();   }
    inline const_iterator         end()     const { tokens.end();     }
    inline const_iterator         cbegin()  const { tokens.cbegin();  }
    inline const_iterator         cend()    const { tokens.cend();    }
    inline reverse_iterator       rbegin()        { tokens.rbegin();  }
    inline reverse_iterator       rend()          { tokens.rend();    }
    inline const_reverse_iterator rbegin()  const { tokens.rbegin();  }
    inline const_reverse_iterator rend()    const { tokens.rend();    }
    inline const_reverse_iterator crbegin() const { tokens.crbegin(); }
    inline const_reverse_iterator crend()   const { tokens.crend();   }

    inline auto size() const {
      return tokens.size();
    }

    inline bool isSimple() const {
      return size() == 1;
    }

    inline std::shared_ptr<Token> &operator[](int idx) {
      return tokens[idx];
    }

    void add(std::shared_ptr<Token>);

  protected:
    AbstractMultiTokenElement(AbstractElementType type, std::shared_ptr<Token> simple);
    AbstractMultiTokenElement(AbstractElementType type,
                              const std::vector<std::shared_ptr<Token>> &compound);
    AbstractMultiTokenElement(AbstractElementType type, iterator first, iterator end);

    std::vector<std::shared_ptr<Token>> tokens;
};


template<typename BASE = AbstractElement>
class AbstractCompoundElement: public AbstractElement {
  public:
    virtual ~AbstractCompoundElement() {}

    typedef typename std::vector<std::shared_ptr<BASE>>::iterator iterator;
    typedef typename std::vector<std::shared_ptr<BASE>>::const_iterator const_iterator;
    typedef typename std::vector<std::shared_ptr<BASE>>::reverse_iterator reverse_iterator;
    typedef typename std::vector<std::shared_ptr<BASE>>::const_reverse_iterator const_reverse_iterator;

    inline iterator               begin()         { elements.begin();   }
    inline iterator               end()           { elements.end();     }
    inline const_iterator         begin()   const { elements.begin();   }
    inline const_iterator         end()     const { elements.end();     }
    inline const_iterator         cbegin()  const { elements.cbegin();  }
    inline const_iterator         cend()    const { elements.cend();    }
    inline reverse_iterator       rbegin()        { elements.rbegin();  }
    inline reverse_iterator       rend()          { elements.rend();    }
    inline const_reverse_iterator rbegin()  const { elements.rbegin();  }
    inline const_reverse_iterator rend()    const { elements.rend();    }
    inline const_reverse_iterator crbegin() const { elements.crbegin(); }
    inline const_reverse_iterator crend()   const { elements.crend();   }

    inline auto size() {
      return elements.size();
    }

    inline bool isSimple() {
      return size() == 1;
    }

    inline bool isEmpty() {
      return size() == 0;
    }

    inline std::shared_ptr<AbstractElement> &operator[](int idx) {
      return elements[idx];
    }

    inline std::shared_ptr<const AbstractElement> &operator[](int idx) const {
      return elements[idx];
    }

    virtual std::ostream &print(std::ostream &os) const {
      auto iter = begin(), end = this->end();

      while(iter != end) {
        if(*iter) {
          (*iter)->print(os);
        }
        else {
          os << "(null)";
        }

        if(++iter != end) {
          os << ' ';
        }
      }

      return os;
    }


    virtual std::ostream &debug(std::ostream &os) const {
      for(auto &element : elements) {
        if(element) {
          element->debug(os);
        }
        else {
          os << "(null)";
        }
      }

      return os;
    }

  protected:
    AbstractCompoundElement(AbstractElementType type):
      AbstractElement(type),
      elements() {
    }

    AbstractCompoundElement(AbstractElementType type, std::shared_ptr<BASE> simple):
      AbstractElement(type),
      elements() {
      elements.emplace_back(simple);
    }

    AbstractCompoundElement(AbstractElementType type,
                            const std::vector<std::shared_ptr<BASE>> &compound):
      AbstractElement(type),
      elements(compound) {
    }

    AbstractCompoundElement(AbstractElementType type, iterator start, iterator end):
      AbstractElement(type),
      elements(start, end) {
    }

    std::vector<std::shared_ptr<BASE>> elements;
};


class AbstractIdentifierElement: public AbstractMultiTokenElement {
  public:
    AbstractIdentifierElement(std::shared_ptr<Token> simple);
    AbstractIdentifierElement(const std::vector<std::shared_ptr<Token>> &compound);
    AbstractIdentifierElement(iterator first, iterator end);

    virtual ~AbstractIdentifierElement();

    virtual std::ostream &print(std::ostream &os) const;

    inline bool isCompound() {
      return size() > 1;
    }

    std::string toString() const;
};


class AbstractImportElement: public AbstractElement {
  public:
    static std::shared_ptr<AbstractImportElement>
    importModule(std::shared_ptr<AbstractIdentifierElement> name);

    static std::shared_ptr<AbstractImportElement>
    importModule(std::shared_ptr<AbstractIdentifierElement> name,
                 std::shared_ptr<AbstractIdentifierElement> alias);

    static std::shared_ptr<AbstractImportElement>
    importElement(std::shared_ptr<AbstractIdentifierElement> name,
                  std::shared_ptr<AbstractIdentifierElement> module);

    static std::shared_ptr<AbstractImportElement>
    importElement(std::shared_ptr<AbstractIdentifierElement> name,
                  std::shared_ptr<AbstractIdentifierElement> module,
                  std::shared_ptr<AbstractIdentifierElement> alias);

    virtual ~AbstractImportElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    inline auto module() {
      return module_ptr;
    }

    inline auto alias() {
      return alias_ptr;
    }

    inline auto element() {
      return element_ptr;
    }


    inline bool hasAlias() {
      return static_cast<bool>(alias_ptr);
    }

    inline bool hasElement() {
      return static_cast<bool>(element_ptr);
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> module_ptr;
    std::shared_ptr<AbstractIdentifierElement> alias_ptr;
    std::shared_ptr<AbstractIdentifierElement> element_ptr;

    AbstractImportElement(std::shared_ptr<AbstractIdentifierElement> module,
                          std::shared_ptr<AbstractIdentifierElement> alias,
                          std::shared_ptr<AbstractIdentifierElement> element);
};


class AbstractImportList: public AbstractCompoundElement<AbstractImportElement> {
  public:
    AbstractImportList();
    AbstractImportList(std::shared_ptr<AbstractImportElement> simple);
    AbstractImportList(const std::vector<std::shared_ptr<AbstractImportElement>> &multi);
    AbstractImportList(iterator first, iterator end);

    virtual ~AbstractImportList();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractImportElement>);
};


class AbstractAliasElement: public AbstractElement {
  public:
    AbstractAliasElement(std::shared_ptr<AbstractIdentifierElement> original,
                         std::shared_ptr<AbstractIdentifierElement> alias);
    virtual ~AbstractAliasElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    inline std::shared_ptr<AbstractIdentifierElement> original() {
      return original_ptr;
    }


    inline std::shared_ptr<AbstractIdentifierElement> alias() {
      return alias_ptr;
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> original_ptr;
    std::shared_ptr<AbstractIdentifierElement> alias_ptr;
};


class AbstractAliasList: public AbstractCompoundElement<AbstractAliasElement> {
  public:
    AbstractAliasList();
    AbstractAliasList(std::shared_ptr<AbstractAliasElement> simple);
    AbstractAliasList(const std::vector<std::shared_ptr<AbstractAliasElement>> &multi);
    AbstractAliasList(iterator first, iterator end);
    virtual ~AbstractAliasList();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractAliasElement>);
};


class AbstractPatternElement: public AbstractElement {
  public:
    AbstractPatternElement(std::shared_ptr<Token> token,
                           std::shared_ptr<Token> lower = nullptr,
                           std::shared_ptr<Token> upper = nullptr,
                           std::shared_ptr<Token> bind  = nullptr);
    virtual ~AbstractPatternElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    inline bool hasMinimum() const {
      return static_cast<bool>(min);
    }

    inline bool hasMaximum() const {
      return static_cast<bool>(max);
    }

    inline bool hasBinding() const {
      return static_cast<bool>(ident);
    }

    auto pattern() {
      return elem;
    }

    auto minimum() {
      return min;
    }

    auto maximum() {
      return max;
    }

    auto binding() {
      return ident;
    }

  protected:
    std::shared_ptr<Token> elem;
    std::shared_ptr<Token> min;
    std::shared_ptr<Token> max;
    std::shared_ptr<Token> ident;
};


class AbstractPatternGroup: public AbstractCompoundElement<AbstractPatternElement> {
  public:
    AbstractPatternGroup(std::shared_ptr<AbstractPatternElement> simple,
                         std::shared_ptr<Token>                  lower = nullptr,
                         std::shared_ptr<Token>                  upper = nullptr,
                         std::shared_ptr<Token>                  bind  = nullptr);
    virtual ~AbstractPatternGroup();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractPatternElement>);

    inline bool hasMinimum() const {
      return static_cast<bool>(min);
    }

    inline bool hasMaximum() const {
      return static_cast<bool>(max);
    }

    inline bool hasBinding() const {
      return static_cast<bool>(ident);
    }

    auto minimum() {
      return min;
    }

    auto maximum() {
      return max;
    }

    auto binding() {
      return ident;
    }

  protected:
    std::shared_ptr<Token>                  min;
    std::shared_ptr<Token>                  max;
    std::shared_ptr<Token>                  ident;
};


class AbstractPatternAlternates: public AbstractCompoundElement<AbstractPatternGroup> {
  public:
    AbstractPatternAlternates(std::shared_ptr<AbstractPatternGroup> simple);
    virtual ~AbstractPatternAlternates();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractPatternGroup>);
};


class AbstractPatternList: public AbstractCompoundElement<AbstractPatternAlternates> {
  public:
    AbstractPatternList(std::shared_ptr<AbstractPatternAlternates> simple);
    virtual ~AbstractPatternList();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractPatternAlternates>);
};


class AbstractStorageElement: public AbstractElement {
  public:
    AbstractStorageElement(std::shared_ptr<AbstractIdentifierElement> name);
    AbstractStorageElement(std::shared_ptr<AbstractIdentifierElement> name,
                           std::shared_ptr<AbstractIdentifierElement> type);
    virtual ~AbstractStorageElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    auto identifier() {
      return ident;
    }

    auto type() {
      return kind;
    }

    inline bool hasType() {
      return static_cast<bool>(kind);
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> ident;
    std::shared_ptr<AbstractIdentifierElement> kind;
};


class AbstractStorageList: public AbstractCompoundElement<AbstractStorageElement> {
  public:
    AbstractStorageList(std::shared_ptr<AbstractStorageElement> simple);
    virtual ~AbstractStorageList();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractStorageElement>);
};


class AbstractSExpr {
  public:
    AbstractSExpr();
    AbstractSExpr(std::shared_ptr<Token>);
    AbstractSExpr(std::shared_ptr<AbstractSExpr>);

    std::ostream &print(std::ostream &os) const;
    std::ostream &debug(std::ostream &os) const;

    void setNext(std::shared_ptr<AbstractSExpr> next) {
      sexpr_next = next;
    }

    auto next() {
      return sexpr_next;
    }

    auto next() const {
      return sexpr_next;
    }

    auto token() {
      return tok_val;
    }

    auto token() const {
      return tok_val;
    }

    auto sexpr() {
      return sexpr_val;
    }

    auto sexpr() const {
      return sexpr_val;
    }

    bool isToken() const {
      static_cast<bool>(tok_val);
    }

    bool isSexpr() const {
      static_cast<bool>(sexpr_val);
    }

    bool isEmpty() const {
      return !isToken() && !isSexpr();
    }

  protected:
    std::shared_ptr<Token> tok_val;
    std::shared_ptr<AbstractSExpr> sexpr_val;
    std::shared_ptr<AbstractSExpr> sexpr_next;
};


class AbstractCodeSnippet: public AbstractElement {
  public:
    AbstractCodeSnippet(std::shared_ptr<AbstractSExpr>);
    virtual ~AbstractCodeSnippet();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    auto sexpr() {
      return expr;
    }

  protected:
    std::shared_ptr<AbstractSExpr> expr;
};


class AbstractRuleElement: public AbstractElement {
  public:
    AbstractRuleElement(
      std::shared_ptr<AbstractIdentifierElement>,
      std::shared_ptr<AbstractPatternList>,
      std::shared_ptr<AbstractStorageList> = nullptr,
      std::shared_ptr<AbstractCodeSnippet> = nullptr,
      std::shared_ptr<AbstractCodeSnippet> = nullptr,
      std::shared_ptr<AbstractCodeSnippet> = nullptr
    );
    virtual ~AbstractRuleElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    auto identifier() {
      return ident;
    }

    auto pattern() {
      return pat;
    }

    auto storage() {
      return store;
    }

    auto validation() {
      return validate;
    }

    auto encode() {
      return enc;
    }

    auto decode() {
      return dec;
    }


    inline bool hasStorage() {
      return static_cast<bool>(store);
    }

    inline bool hasValidation() {
      return static_cast<bool>(validate);
    }

    inline bool hasEncode() {
      return static_cast<bool>(enc);
    }

    inline bool hasDecode() {
      return static_cast<bool>(dec);
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> ident;
    std::shared_ptr<AbstractPatternList>       pat;
    std::shared_ptr<AbstractStorageList>       store;
    std::shared_ptr<AbstractCodeSnippet>       validate;
    std::shared_ptr<AbstractCodeSnippet>       enc;
    std::shared_ptr<AbstractCodeSnippet>       dec;
};


class AbstractNamespaceElement: public AbstractElement {
  public:
    AbstractNamespaceElement(std::shared_ptr<AbstractIdentifierElement>, bool module = false);
    virtual ~AbstractNamespaceElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    typedef std::map<const std::string, std::shared_ptr<AbstractRuleElement>>::iterator iterator;
    typedef std::map<const std::string, std::shared_ptr<AbstractRuleElement>>::const_iterator const_iterator;
    typedef std::map<const std::string, std::shared_ptr<AbstractRuleElement>>::reverse_iterator reverse_iterator;
    typedef std::map<const std::string, std::shared_ptr<AbstractRuleElement>>::const_reverse_iterator const_reverse_iterator;

    inline iterator               begin()         { elements.begin();   }
    inline iterator               end()           { elements.end();     }
    inline const_iterator         begin()   const { elements.begin();   }
    inline const_iterator         end()     const { elements.end();     }
    inline const_iterator         cbegin()  const { elements.cbegin();  }
    inline const_iterator         cend()    const { elements.cend();    }
    inline reverse_iterator       rbegin()        { elements.rbegin();  }
    inline reverse_iterator       rend()          { elements.rend();    }
    inline const_reverse_iterator rbegin()  const { elements.rbegin();  }
    inline const_reverse_iterator rend()    const { elements.rend();    }
    inline const_reverse_iterator crbegin() const { elements.crbegin(); }
    inline const_reverse_iterator crend()   const { elements.crend();   }

    inline auto identifier() {
      return ident;
    }

    inline auto size() {
      return elements.size();
    }

    inline bool isEmpty() {
      return size() == 0;
    }

    inline std::shared_ptr<AbstractRuleElement> &operator[](const std::string &key) {
      return elements[key];
    }

    void add(std::shared_ptr<AbstractRuleElement>);

  protected:
    std::shared_ptr<AbstractIdentifierElement> ident;
    std::map<const std::string, std::shared_ptr<AbstractRuleElement>> elements;
};


class AbstractSyntaxTree {
  public:
    AbstractSyntaxTree();

    std::ostream &print(std::ostream &os) const;
    std::ostream &debug(std::ostream &os) const;

    typedef std::vector<std::shared_ptr<AbstractNamespaceElement>>::iterator iterator;
    typedef std::vector<std::shared_ptr<AbstractNamespaceElement>>::const_iterator const_iterator;
    typedef std::vector<std::shared_ptr<AbstractNamespaceElement>>::reverse_iterator reverse_iterator;
    typedef std::vector<std::shared_ptr<AbstractNamespaceElement>>::const_reverse_iterator const_reverse_iterator;

    inline iterator               begin()         { spaces.begin();   }
    inline iterator               end()           { spaces.end();     }
    inline const_iterator         begin()   const { spaces.begin();   }
    inline const_iterator         end()     const { spaces.end();     }
    inline const_iterator         cbegin()  const { spaces.cbegin();  }
    inline const_iterator         cend()    const { spaces.cend();    }
    inline reverse_iterator       rbegin()        { spaces.rbegin();  }
    inline reverse_iterator       rend()          { spaces.rend();    }
    inline const_reverse_iterator rbegin()  const { spaces.rbegin();  }
    inline const_reverse_iterator rend()    const { spaces.rend();    }
    inline const_reverse_iterator crbegin() const { spaces.crbegin(); }
    inline const_reverse_iterator crend()   const { spaces.crend();   }

    std::shared_ptr<AbstractNamespaceElement> currentNamespace();
    std::shared_ptr<AbstractNamespaceElement>
    addNamespace(std::shared_ptr<AbstractIdentifierElement>, bool emit);

  protected:
    std::vector<std::shared_ptr<AbstractNamespaceElement>> spaces;
};


  }
}
