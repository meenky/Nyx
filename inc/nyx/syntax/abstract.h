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
  Import,
  ImportList,
  Module,
  Namespace,
  Rule
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

    inline auto size() {
      return tokens.size();
    }

    inline bool isSimple() {
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
    virtual ~AbstractCompoundElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

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

  protected:
    AbstractCompoundElement(AbstractElementType type, std::shared_ptr<BASE> simple);
    AbstractCompoundElement(AbstractElementType type,
                            const std::vector<std::shared_ptr<BASE>> &compound);

    void append(std::shared_ptr<BASE> &);

    std::vector<std::shared_ptr<BASE>> elements;
};


class AbstractIdentifierElement: public AbstractMultiTokenElement {
  public:
    AbstractIdentifierElement(std::shared_ptr<Token> simple);
    AbstractIdentifierElement(const std::vector<std::shared_ptr<Token>> &compound);
    AbstractIdentifierElement(iterator first, iterator end);

    virtual ~AbstractIdentifierElement();

    virtual std::ostream &print(std::ostream &os) const;
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

    std::shared_ptr<AbstractIdentifierElement> module();
    std::shared_ptr<AbstractIdentifierElement> alias();
    std::shared_ptr<AbstractIdentifierElement> element();

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
    virtual ~AbstractAliasList();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractAliasElement>);
};


class AbstractPatternElement: public AbstractMultiTokenElement {
  public:
    AbstractPatternElement(std::shared_ptr<Token> simple);
    AbstractPatternElement(const std::vector<std::shared_ptr<Token>> &compound);
    AbstractPatternElement(iterator first, iterator end);
    virtual ~AbstractPatternElement();

    virtual std::ostream &print(std::ostream &os) const;
};


class AbstractPatternList: public AbstractCompoundElement<AbstractPatternElement> {
  public:
    AbstractPatternList();
    AbstractPatternList(std::shared_ptr<AbstractPatternElement> simple);
    AbstractPatternList(const std::vector<std::shared_ptr<AbstractPatternElement>> &multi);
    virtual ~AbstractPatternList();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractPatternElement>);
};


class AbstractStorageElement: public AbstractElement {
  public:
    AbstractStorageElement(std::shared_ptr<AbstractIdentifierElement> name);
    AbstractStorageElement(std::shared_ptr<AbstractIdentifierElement> name,
                           std::shared_ptr<AbstractIdentifierElement> type);
    virtual ~AbstractStorageElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    std::shared_ptr<AbstractIdentifierElement> identifier();
    std::shared_ptr<AbstractIdentifierElement> type();

    inline bool hasType() {
      return static_cast<bool>(type_ptr);
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> ident;
    std::shared_ptr<AbstractIdentifierElement> type_ptr;
};


class AbstractStorageList: public AbstractCompoundElement<AbstractStorageElement> {
  public:
    AbstractStorageList();
    AbstractStorageList(std::shared_ptr<AbstractStorageElement> simple);
    AbstractStorageList(const std::vector<std::shared_ptr<AbstractStorageElement>> &multi);
    virtual ~AbstractStorageList();

    virtual std::ostream &print(std::ostream &os) const;

    void add(std::shared_ptr<AbstractStorageElement>);
};


class AbstractCodeSnippet: public AbstractElement {
 
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

    std::shared_ptr<AbstractIdentifierElement> identifier();
    std::shared_ptr<AbstractPatternList>       pattern();
    std::shared_ptr<AbstractStorageList>       storage();
    std::shared_ptr<AbstractCodeSnippet>       validation();
    std::shared_ptr<AbstractCodeSnippet>       encode();
    std::shared_ptr<AbstractCodeSnippet>       decode();

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
    std::shared_ptr<AbstractPatternElement>    pat;
    std::shared_ptr<AbstractStorageElement>    store;
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

    typedef std::vector<std::shared_ptr<AbstractRuleElement>>::iterator iterator;
    typedef std::vector<std::shared_ptr<AbstractRuleElement>>::const_iterator const_iterator;
    typedef std::vector<std::shared_ptr<AbstractRuleElement>>::reverse_iterator reverse_iterator;
    typedef std::vector<std::shared_ptr<AbstractRuleElement>>::const_reverse_iterator const_reverse_iterator;

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

  protected:
    std::shared_ptr<AbstractIdentifierElement> ident;
    std::map<const std::string, std::shared_ptr<AbstractRuleElement>> elements;
};


class AbstractSyntaxTree {
  public:
    AbstractSyntaxTree();
    ~AbstractSyntaxTree();

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
