#include <iostream>
#include <fstream>
#include <string>
#include <set>

using namespace std;

#include "parser.h"

int lineNumber = 0;
//bool shouldTrace = false;
bool hasParseErrors = false;
string *theInputFileName = 0;

map<string, Value> symbolTable;
map<string, TypeForNode> typeTable;

// Print the parse error to standard output
// If the input is file (as indicated by non-null theInputFileName pointer),
// prepend the error message with "filename:"
void error(int linenum, const string& message)
{
    if (theInputFileName)
    {
        cout << *theInputFileName << ":";
    }
    cout << linenum+1 << ":" << message << endl;
}

// SemanticCheck implemented as tree visitor
// We visit only relevant nodes, i.e. VariableDeclaration and Identifier nodes
// We keep a set of all names declared so far
class SemanticCheck : public ParseTreeVisitor
{
    bool hasErrors;

public:
    SemanticCheck()
            : hasErrors(false)
    {
    }

    // After full traversal, we can use this function to see if there were any semantic errors
    bool isErrorFree()
    {
        return !hasErrors;
    }

    // Semantic rule #4 - check if variable wasn't declared before
    virtual bool beginVisit(const VariableDeclaration *varDecl)
    {
        Identifier *identifier = varDecl->getIdentifier();
        if (typeTable.find(identifier->getName()) != typeTable.end())
        {
            // variable was declared before
            error(varDecl->getLineNumber(), "variable " + identifier->getName() + " was already declared");
            hasErrors = true;
        }
        else
        {
            typeTable[identifier->getName()] = varDecl->GetType();
        }
        // We return false here, because we don't want to go into VariableDeclaration children,
        // as it only has one, Identifier, and we already handled that above
        return false;
    }

    virtual bool beginVisit(const VariableAssignment *varAssign)
    {
        varAssign->getIdentifier()->accept(this);
        varAssign->getLeft()->accept(this);
        if (typeTable.find(varAssign->getIdentifier()->getName()) != typeTable.end() &&
            varAssign->getLeft()->GetType() != typeTable[varAssign->getIdentifier()->getName()])
        {
            error(varAssign->getLeft()->getLineNumber(), "type error");
            hasErrors = true;
        }
        return false;
    }

    virtual bool beginVisit(const PrintCommand *printCmd)
    {
        printCmd->getLeft()->accept(this);
        return false;
    }

    // Semantic rule #2 - check if variable name was declared before use
    // All identifiers that are not children of VariableDeclaration node
    // must be uses of variable name, in expressions, assignments, etc.
    virtual bool beginVisit(const Identifier *identifier)
    {
        // we simply check if variable wasn't declared, if it was then it's name is in the set
        if (typeTable.find(identifier->getName()) == typeTable.end())
        {
            // wasn't declared
            error(identifier->getLineNumber(), "variable " + identifier->getName() + " is used before being declared");
            hasErrors = true;
        }
        return false;
    }

    virtual bool beginVisit(const Addition *add)
    {
        return beginVisitOperation(add);
    }

    virtual bool beginVisit(const Subtraction *sub)
    {
        return beginVisitOperation(sub);
    }

    virtual bool beginVisit(const Multiplication *mul)
    {
        return beginVisitOperation(mul);
    }

    virtual bool beginVisit(const Division *dvsn)
    {
        return beginVisitOperation(dvsn);
    }

    bool beginVisitOperation(const ParseTree *op)
    {
        op->getLeft()->accept(this);
        op->getRight()->accept(this);
        if (op->GetType() == ERROR_TYPE)
        {
            error(op->getLineNumber(), "type error");
        }
        return false;
    }
};

int main(int argc, char *argv[])
{
    int arg = 1;
    // Check for arguments
    // filename for input file, if any
    while (arg < argc)
    {
        string curArg = argv[arg];
        if (theInputFileName == 0)
        {
            theInputFileName = new string(curArg);
        }
        else
        {
            cout << "TOO MANY FILES" << endl;
            return 1;
        }
        ++arg;
    }

    ParseTree *tree = 0;
    // Read from standard input if no file name was provided
    if (theInputFileName == 0)
    {
        tree = Prog( &cin );
    }
    else
    {
        // Read from file if name was provided
        ifstream f;
        f.open(*theInputFileName);
        if (f.fail())
        {
            cout << *theInputFileName << " FILE NOT FOUND" << endl;
            return 1;
        }
        tree = Prog (&f);
        f.close();
    }
    if( tree == 0 || hasParseErrors)
    {
        // Parse finished and there were errors
        // They were printed in-the-fly, so we can finish here
        return 1;
    }
    // Semantic check is performed by creating SemanticCheck object and accepting it by the tree
    SemanticCheck semanticCheck;
    tree->accept(&semanticCheck);
    if (semanticCheck.isErrorFree())
    {
        tree->Evaluate();
    }
    return 0;
}


