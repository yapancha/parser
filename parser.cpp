#include <string>
using std::string;

#include "parser.h"

class ParserToken
{
private:
    Token	tok;
    bool	pushedBack;

public:
    ParserToken() : pushedBack(false) {}

    Token getToken(istream *in)
    {
        if (pushedBack)
        {
            pushedBack = false;
            return tok;
        }
        return ::getToken(in);
    }

    void pushbackToken(const Token& t)
    {
        if (pushedBack)
        {
            // error
        }
        tok = t;
        pushedBack = true;
    }
} ParserToken;

// helper methods that print specific type of error message and set parse error flag
void syntaxError(int line, string text)
{
    error(line, "Syntax error " + text);
    hasParseErrors = true;
}

// check if a token is identifier
// returns new instance of Identifier if true,
// or signals error and return null if false
Identifier* checkIdentifier(const Token& id)
{
    switch (id.GetTokenType())
    {
        case T_ID:
            return new Identifier(id);
        default:
            syntaxError(id.GetLinenum(), "identifier expected");
            break;
    }
    return 0;
}

// Prog ::= StmtList
ParseTree* Prog(istream* in)
{
    return StmtList(in);
}

// StmtList ::=  { Stmt T_SC } { StmtList }
ParseTree* StmtList(istream* in)
{
    ParseTree *stmt = Stmt(in);
    if (stmt != 0)
    {
        Token semicolon = ParserToken.getToken(in);
        switch(semicolon.GetTokenType())
        {
            case T_SC:
                break;
            default:
                syntaxError(semicolon.GetLinenum(), "semicolon required");
                return 0;
        }
        return new StatementList(stmt, StmtList(in));
    }
    return 0;
}

// Stmt ::=  Decl | Set | Print
ParseTree* Stmt(istream* in)
{
    // look ahead and see what token is next
    Token token = ParserToken.getToken(in);
    ParserToken.pushbackToken(token);

    ParseTree *stmt = 0;
    // check if token matches one of the possible choices
    switch (token.GetTokenType())
    {
        case T_INT:
        case T_STRING:
            stmt = Decl(in);
            break;
        case T_SET:
            stmt = Set(in);
            break;
        case T_PRINT:
        case T_PRINTLN:
            stmt = Print(in);
            break;
        case T_DONE:
            return 0;
        default:
            syntaxError(token.GetLinenum(), "statement expected");
            return 0;
    }
    return stmt;
}

// Decl ::= T_INT T_ID | T_STRING T_ID
// when this function is called, next token is for sure either T_INT or T_STRING, so no need to check
ParseTree *	Decl(istream* in)
{
    Token declarationType = ParserToken.getToken(in);
    Token id = ParserToken.getToken(in);

    if (Identifier *identifier = checkIdentifier(id))
    {
        return new VariableDeclaration(declarationType, identifier);
    }
    else
    {
        syntaxError(id.GetLinenum(), "declaration expected");
    }
    return 0;
}

// Set ::= T_SET T_ID Expr
// when this functions is called, next token is T_SET for sure
ParseTree *	Set(istream* in)
{
    Token set = ParserToken.getToken(in);
    Token id = ParserToken.getToken(in);
    if (Identifier *identifier = checkIdentifier(id))
    {
        ParseTree *expr = Expr(in);
        if (expr != 0)
        {
            return new VariableAssignment(set, identifier, expr);
        }
        else
        {
            syntaxError(id.GetLinenum(), "expression required");
        }
    }
    return 0;
}

// Set ::= T_PRINT Expr | T_PRINTLN Expr
// again, when this function is called, we for sure have T_PRINT or T_PRINTLN token next
ParseTree* Print(istream* in)
{
    Token keyword = ParserToken.getToken(in);
    ParseTree *expr = Expr(in);
    if (expr != 0)
    {
        return new PrintCommand(keyword, expr);
    }
    else
    {
        syntaxError(keyword.GetLinenum(), "expression required");
    }
    return 0;
}

// Expr ::= Term { (T_PLUS | T_MINUS) Expr }
ParseTree* Expr(istream* in)
{
    ParseTree *t1 = Term(in);
    if (t1 != 0)
    {
        for(;;)
        {
            Token op = ParserToken.getToken(in);
            if( op != T_PLUS && op != T_MINUS )
            {
                ParserToken.pushbackToken(op);
                return t1;
            }

            ParseTree *t2 = Term(in);
            if( t2 == 0 )
            {
                syntaxError(op.GetLinenum(), "expression required after + or - operator");
                return 0;
            }

            // combine t1 and t2 together
            if( op == T_PLUS )
            {
                t1 = new Addition(op.GetLinenum(), t1, t2);
            }
            else
            {
                t1 = new Subtraction(op.GetLinenum(), t1, t2);
            }
        }
    }
    return 0;
}

// Term ::= Primary { (T_STAR | T_SLASH) Term}
// same algorithm as above, as the rule has exactly the same structure,
// only differs in symbols used
ParseTree*	Term(istream* in)
{
    ParseTree *t1 = Primary(in);
    if (t1 != 0)
    {
        for (;;)
        {
            Token op = ParserToken.getToken(in);
            if (op != T_STAR && op != T_SLASH)
            {
                ParserToken.pushbackToken(op);
                return t1;
            }

            ParseTree *t2 = Primary(in);
            if (t2 == 0)
            {
                syntaxError(op.GetLinenum(), "term required after * or / operator");
                return 0;
            }

            if (op == T_STAR)
            {
                t1 = new Multiplication(op.GetLinenum(), t1, t2);
            }
            else
            {
                t1 = new Division(op.GetLinenum(), t1, t2);
            }
        }
    }
    else
    {

    }
    return 0;
}

// Primary ::= T_ICONST | T_SCONST | T_ID | T_LPAREN Expr T_RPAREN
// straight-forward, depending on next token, we either signal error,
// or descend into appropriate parsing function
// Additionaly, in last production we consume ( and )
ParseTree*	Primary(istream* in)
{
    Token firstToken = ParserToken.getToken(in);
    switch(firstToken.GetTokenType())
    {
        case T_ICONST:
            return new IntegerConstant(firstToken);
        case T_SCONST:
            return new StringConstant(firstToken);
        case T_ID:
            return new Identifier(firstToken);
        case T_LPAREN:
        {
            ParseTree *expr = Expr(in);
            Token lastToken = ParserToken.getToken(in);
            switch (lastToken.GetTokenType())
            {
                case T_RPAREN:
                    return expr;
                default:
                    syntaxError(lastToken.GetLinenum(), "right paren expected");
                    break;
            }
            break;
        }
        default:
            syntaxError(firstToken.GetLinenum(), "primary expected");
            break;
    }
    return 0;
}

