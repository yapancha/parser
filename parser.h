
#ifndef PARSER_H_
#define PARSER_H_

#include <iostream>
using std::istream;
using std::ostream;

#include <string>

using std::string;
using std::stoi;

#include <map>
using std::map;

#include "lexer.h"

// indicates if parse errors were present
extern bool hasParseErrors;
extern void error(int linenum, const string& message);

enum TypeForNode { INT_TYPE, STRING_TYPE, ERROR_TYPE, EMPTY_TYPE };

struct Value
{
    TypeForNode type;
    int intValue;
    string stringValue;

    static Value Integer(int v = 0)
    {
        return Value(v);
    }

    static Value String(string s = "")
    {
        return Value(s);
    }

    static Value Error(string error = "")
    {
        Value val(ERROR_TYPE);
        val.stringValue = error;
        return val;
    }

    static Value Empty()
    {
        return Value(EMPTY_TYPE);
    }

    Value() : type(EMPTY_TYPE) {}

private:
    Value(int v)
            : type(INT_TYPE),
              intValue(v)
    {
    }
    Value(string s)
            : type(STRING_TYPE),
              stringValue(s)
    {
    }
    Value(TypeForNode type)
            : type(type)
    {
    }
};

inline ostream& operator<<(ostream& os, const Value& v)
{
    if (v.type == INT_TYPE)
    {
        return os << v.intValue;
    }
    else if (v.type == STRING_TYPE)
    {
        return os << v.stringValue;
    }
    return os;
}

inline Value operator+(const Value& u, const Value& v)
{
    if (u.type == INT_TYPE && v.type == INT_TYPE)
    {
        return Value::Integer(u.intValue + v.intValue);
    }
    if (u.type == STRING_TYPE && v.type == STRING_TYPE)
    {
        return Value::String(u.stringValue + v.stringValue);
    }
    return Value::Error();
}

inline Value operator-(const Value& u, const Value& v)
{
    if (u.type == INT_TYPE && v.type == INT_TYPE)
    {
        return Value::Integer(u.intValue - v.intValue);
    }
    return Value::Error();
}

inline Value operator*(const Value& u, const Value& v)
{
    if (u.type == INT_TYPE && v.type == INT_TYPE)
    {
        return Value::Integer(u.intValue * v.intValue);
    }
    if (u.type == INT_TYPE && v.type == STRING_TYPE)
    {
        string accum = "";
        int count = u.intValue;
        while (count--)
        {
            accum += v.stringValue;
        }
        return Value::String(accum);
    }
    if (v.type == INT_TYPE && u.type == STRING_TYPE)
    {
        string accum = "";
        int count = v.intValue;
        while (count--)
        {
            accum += u.stringValue;
        }
        return Value::String(accum);
    }
    return Value::Error();
}

inline Value operator/(const Value& u, const Value& v)
{
    if (u.type == INT_TYPE && v.type == INT_TYPE)
    {
        if(v.intValue != 0)
        {
            return Value::Integer(u.intValue / v.intValue);
        }
        else
        {
            return Value::Error("DIVIDE BY ZERO");
        }
    }
    if (u.type == STRING_TYPE && v.type == STRING_TYPE)
    {
        size_t pos = u.stringValue.find(v.stringValue);
        if (pos == string::npos)
        {
            return u;
        }
        Value result = Value::String(u.stringValue);
        result.stringValue.replace(pos, v.stringValue.size(), "");
        return result;
    }
    return Value::Error();
}

extern map<string, Value> symbolTable;
extern map<string, TypeForNode> typeTable;

// forward declaration of visitor class
// ParseTree needs it, but visitor also needs classes depending on ParseTree
class ParseTreeVisitor;

class ParseTree {
    int			linenumber;
    ParseTree	*left;
    ParseTree	*right;

public:
    ParseTree(int n, ParseTree *l = 0, ParseTree *r = 0) : linenumber(n), left(l), right(r) {}
    virtual ~ParseTree() {}

    ParseTree* getLeft() const { return left; }
    ParseTree* getRight() const { return right; }
    int getLineNumber() const { return linenumber; }

    virtual TypeForNode GetType() const { return ERROR_TYPE; }
    virtual int GetIntValue() const { throw "no integer value"; }
    virtual string GetStringValue() const { throw "no string value"; }

    virtual Value Evaluate() const = 0;

    // accept visitor
    // derived classes override this method to perform tree traversal
    virtual void accept(ParseTreeVisitor *visitor) const = 0;
};

// forward declaration of all the classes that ParseTreeVisitor needs
// it's required, because all of these classes depend on methods of ParseTreeVisitor
class StatementList;
class Addition;
class Subtraction;
class Multiplication;
class Division;
class PrintCommand;
class VariableAssignment;
class VariableDeclaration;
class Identifier;
class IntegerConstant;
class StringConstant;

// implementation of visitor pattern
// when tree node accepts a visitor, it calls type-appropriate version of beginVisit/endVisit methods
class ParseTreeVisitor
{
protected:
    // default methods, in case the deriving visitor doesn't care about the types
    virtual bool beginVisitNode(const ParseTree *tree)
    {
        return true;
    }
    virtual void endVisitNode(const ParseTree *tree) {}

public:
    virtual ~ParseTreeVisitor() {}

    // for each ParseTree type, there is a pair beginVisit/endVisit
    virtual bool beginVisit(const StatementList *stmts)
    {
        return beginVisitNode((ParseTree*)stmts);
    }
    virtual bool beginVisit(const Addition *add)
    {
        return beginVisitNode((ParseTree*)add);
    }
    virtual bool beginVisit(const Subtraction *sub)
    {
        return beginVisitNode((ParseTree*)sub);
    }
    virtual bool beginVisit(const Multiplication *mul)
    {
        return beginVisitNode((ParseTree*)mul);
    }
    virtual bool beginVisit(const Division *div)
    {
        return beginVisitNode((ParseTree*)div);
    }
    virtual bool beginVisit(const PrintCommand *print)
    {
        return beginVisitNode((ParseTree*)print);
    }
    virtual bool beginVisit(const VariableAssignment *varAssign)
    {
        return beginVisitNode((ParseTree*)varAssign);
    }
    virtual bool beginVisit(const VariableDeclaration *varDecl)
    {
        return beginVisitNode((ParseTree*)varDecl);
    }
    virtual bool beginVisit(const Identifier *id)
    {
        return beginVisitNode((ParseTree*)id);
    }
    virtual bool beginVisit(const IntegerConstant *intConst)
    {
        return beginVisitNode((ParseTree*)intConst);
    }
    virtual bool beginVisit(const StringConstant *strConst)
    {
        return beginVisitNode((ParseTree*)strConst);
    }
    virtual void endVisit(const StatementList *stmts)
    {
        endVisitNode((ParseTree*)stmts);
    }
    virtual void endVisit(const Addition *add)
    {
        endVisitNode((ParseTree*)add);
    }
    virtual void endVisit(const Subtraction *sub)
    {
        endVisitNode((ParseTree*)sub);
    }
    virtual void endVisit(const Multiplication *mul)
    {
        endVisitNode((ParseTree*)mul);
    }
    virtual void endVisit(const Division *div)
    {
        endVisitNode((ParseTree*)div);
    }
    virtual void endVisit(const PrintCommand *print)
    {
        endVisitNode((ParseTree*)print);
    }
    virtual void endVisit(const VariableAssignment *varAssign)
    {
        endVisitNode((ParseTree*)varAssign);
    }
    virtual void endVisit(const VariableDeclaration *varDecl)
    {
        endVisitNode((ParseTree*)varDecl);
    }
    virtual void endVisit(const Identifier *id)
    {
        endVisitNode((ParseTree*)id);
    }
    virtual void endVisit(const IntegerConstant *intConst)
    {
        endVisitNode((ParseTree*)intConst);
    }
    virtual void endVisit(const StringConstant *strConst)
    {
        endVisitNode((ParseTree*)strConst);
    }
};

// these are straight-forward
// all types of syntax constructs are represented here
// note that only minimum of methods required for this particular assignment is supported,
// e.g. semantic check only cares about variable names, so we don't implement type-returning methods, etc.
class StatementList : public ParseTree {
public:
    StatementList(ParseTree *first, ParseTree *rest) : ParseTree(0, first, rest) {}

    virtual Value Evaluate() const
    {
        if (getLeft() != NULL)
        {
            if (getLeft()->Evaluate().type != EMPTY_TYPE)
                return Value::Error();
        }
        if (getRight() != NULL)
        {
            return getRight()->Evaluate();
        }
        return Value::Empty();
    }

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        if (visitor->beginVisit(this))
        {
            if (getLeft() != 0)
            {
                getLeft()->accept(visitor);
            }
            if (getRight() != 0)
            {
                getRight()->accept(visitor);
            }
        }
        visitor->endVisit(this);
    }
};

class Addition : public ParseTree {
public:
    Addition(int line, ParseTree *op1, ParseTree *op2) : ParseTree(line, op1, op2) {}

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        if (visitor->beginVisit(this))
        {
            getLeft()->accept(visitor);
            getRight()->accept(visitor);
        }
        visitor->endVisit(this);
    }

    virtual Value Evaluate() const
    {
        return getLeft()->Evaluate() + getRight()->Evaluate();
    }

    virtual TypeForNode GetType() const
    {
        if (getLeft()->GetType() == getRight()->GetType())
        {
            return getLeft()->GetType();
        }
        return ERROR_TYPE;
    }
};

class Subtraction : public ParseTree {
public:
    Subtraction(int line, ParseTree *op1, ParseTree *op2) : ParseTree(line, op1, op2) {}

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        if (visitor->beginVisit(this))
        {
            getLeft()->accept(visitor);
            getRight()->accept(visitor);
        }
        visitor->endVisit(this);
    }

    virtual Value Evaluate() const
    {
        return getLeft()->Evaluate() - getRight()->Evaluate();
    }

    virtual TypeForNode GetType() const
    {
        if (getLeft()->GetType() == INT_TYPE && getRight()->GetType() == INT_TYPE)
        {
            return INT_TYPE;
        }
        return ERROR_TYPE;
    }
};

class Multiplication : public ParseTree {
public:
    Multiplication(int line, ParseTree *op1, ParseTree *op2) : ParseTree(line, op1, op2) {}

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        if (visitor->beginVisit(this))
        {
            getLeft()->accept(visitor);
            getRight()->accept(visitor);
        }
        visitor->endVisit(this);
    }

    virtual Value Evaluate() const
    {
        return getLeft()->Evaluate() * getRight()->Evaluate();
    }

    virtual TypeForNode GetType() const
    {
        if (getLeft()->GetType() == INT_TYPE)
        {
            return getRight()->GetType();
        }
        if (getLeft()->GetType() == STRING_TYPE && getRight()->GetType() == INT_TYPE)
        {
            return STRING_TYPE;
        }
        return ERROR_TYPE;
    }
};

class Division : public ParseTree {
public:
    Division(int line, ParseTree *op1, ParseTree *op2) : ParseTree(line, op1, op2) {}

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        if (visitor->beginVisit(this))
        {
            getLeft()->accept(visitor);
            getRight()->accept(visitor);
        }
        visitor->endVisit(this);
    }

    virtual Value Evaluate() const
    {
        Value val = getLeft()->Evaluate() / getRight()->Evaluate();
        if (val.type == ERROR_TYPE && val.stringValue.size() > 0)
        {
            error(getLineNumber(), val.stringValue);
        }
        return val;
    }

    virtual TypeForNode GetType() const
    {
        if (getLeft()->GetType() == getRight()->GetType())
        {
            return getLeft()->GetType();
        }
        return ERROR_TYPE;
    }
};

class IntegerConstant : public ParseTree
{
    int	value;
public:
    IntegerConstant(const Token& tok) : ParseTree(tok.GetLinenum()) {
        value = stoi( tok.GetLexeme() );
    }

    virtual TypeForNode GetType() const { return INT_TYPE; }
    virtual int GetIntValue() const { return value; }

    virtual Value Evaluate() const
    {
        return Value::Integer(value);
    }

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        visitor->beginVisit(this);
        visitor->endVisit(this);
    }
};

class StringConstant : public ParseTree
{
    string value;
public:
    StringConstant(const Token& token)
            : ParseTree(token.GetLinenum())
    {
        value = token.GetLexeme().substr(1, token.GetLexeme().size() -2);
    }

    virtual TypeForNode GetType() const { return STRING_TYPE; }
    virtual string GetStringValue() const { return value; }

    virtual Value Evaluate() const
    {
        return Value::String(value);
    }

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        visitor->beginVisit(this);
        visitor->endVisit(this);
    }
};

class Identifier : public ParseTree {
    string identifier;
public:
    Identifier(const Token& id)
            : ParseTree(id.GetLinenum()),
              identifier(id.GetLexeme())
    {
    }

    string getName() const
    {
        return identifier;
    }

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        visitor->beginVisit(this);
        visitor->endVisit(this);
    }

    virtual Value Evaluate() const
    {
        return symbolTable[identifier];
    }

    virtual TypeForNode GetType() const
    {
        if (typeTable.find(identifier) != typeTable.end())
        {
            return typeTable[identifier];
        }
        return ERROR_TYPE;
    }
};

class VariableDeclaration : public ParseTree
{
    const TypeForNode type;
    Identifier *identifier;
public:
    VariableDeclaration(const Token& keyword, Identifier *identifier)
            : ParseTree(keyword.GetLinenum()),
              identifier(identifier),
              type(keyword.GetTokenType() == T_INT ? INT_TYPE : STRING_TYPE)
    {
    }

    virtual Value Evaluate() const
    {
        symbolTable[identifier->getName()] = type == INT_TYPE ? Value::Integer() : Value::String();
        return Value::Empty();
    }

    virtual TypeForNode GetType() const
    {
        return type;
    }

    Identifier* getIdentifier() const
    {
        return identifier;
    }

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        visitor->beginVisit(this);
        visitor->endVisit(this);
    }
};

class VariableAssignment : public ParseTree
{
    Identifier *identifier;
public:
    VariableAssignment(const Token& keyword, Identifier *identifier, ParseTree *expr)
            : ParseTree(keyword.GetLinenum(), expr),
              identifier(identifier)
    {
    }

    virtual Value Evaluate() const
    {
        Value val = getLeft()->Evaluate();
        if (val.type != ERROR_TYPE)
        {
            symbolTable[identifier->getName()] = val;
            return Value::Empty();
        }
        return Value::Error();
    }

    Identifier* getIdentifier() const
    {
        return identifier;
    }

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        if (visitor->beginVisit(this))
        {
            getLeft()->accept(visitor);
        }
        visitor->endVisit(this);
    }
};

class PrintCommand : public ParseTree
{
    const TokenType tokenType;
public:
    PrintCommand(const Token& keyword, ParseTree *expr)
            : ParseTree(keyword.GetLinenum(), expr),
              tokenType(keyword.GetTokenType())
    {
    }

    virtual Value Evaluate() const
    {
        Value val = getLeft()->Evaluate();
        if (val.type != ERROR_TYPE)
        {
            std::cout << val;
            if (IsNewline())
            {
                std::cout << std::endl;
            }
            return Value::Empty();
        }
        return Value::Error();
    }

    bool IsNewline() const
    {
        return tokenType == T_PRINTLN;
    }

    virtual void accept(ParseTreeVisitor *visitor) const
    {
        if (visitor->beginVisit(this))
        {
            getLeft()->accept(visitor);
        }
        visitor->endVisit(this);
    }
};

extern ParseTree *	Prog(istream* in);
extern ParseTree *	StmtList(istream* in);
extern ParseTree *	Stmt(istream* in);
extern ParseTree *	Decl(istream* in);
extern ParseTree *	Set(istream* in);
extern ParseTree *	Print(istream* in);
extern ParseTree *	Expr(istream* in);
extern ParseTree *	Term(istream* in);
extern ParseTree *	Primary(istream* in);


#endif /* PARSER_H_ */
