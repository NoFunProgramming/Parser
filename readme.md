# Parser Generator for Building Compilers

This program generates parse tables for languages that are defined by a context
free grammar.  A calculator or a compiler can then be constructed by combining
these generated parse tables with user fuctions defined for each rule of the
grammar.

## Defining a Language
A common way to represent a context free grammar is in Backus-Naur Form or BNF.
The grammar is defined by two types of symbols: terminals and nonterminals.
In the language's definition the terminals are shown in quotes and are defined
by a regular expression.
```
    'num' [0-9]+;
```
The terminals can also have a user defined action and a type.  When their
pattern is matched in the input string, the program calls this action with the
matched string and expects the function to return a pointer to an object of the
provided type.
```
    'num'<Expr> [0-9]+ &scan_num;
    
    std::unique_ptr<Expr>
    scan_num(Table* table, const std::string& text)
    {
        return std::make_unique<Expr>(std::stoi(text));
    }
```
The nonterminals are defined as a sequence of symbols known as a production
rule.  These rules are written as a nonterminal followed by zero or more
symbols.  If there is more than one rule associated the same nonterminal, they
are separated by a vertical bar.  A semicolon indicates the end of the rules for
a given nonterminal.
```
    add: mul | add '+' mul;
```
During operation the program identifies terminals in the input string and adds
them to a stack.  Based on actions defined by the parse table, the program will
replace symbols on the top of the stack with a nonterminal and then call the
action named after the ampersand in the rule.  Each nonterminal has a type,
defined within angle brackets, and the program calls the function with an input
argument for each symbol of the production rule that has a type.
```
    add<int64> add '+' mul &reduce_add_mul;
    
    std::unique_ptr<Expr>
    reduce_add_mul(Table* table, unique_ptr<Expr>& E1, unique_ptr<Expr>& E2)
    {
        std::unique_ptr<Expr> result = std::move(E1);
        result->value += E2->value;
        return result;
    }
```
## Example Program

The source code includes an example program that defines the language and
functions to implement a calculator.  With the calculator's language as an
input, the parser program generates the parse table.  This parse table is then
compiled along with the user defined functions to build a calculator.

## Video Overviews

- [Part 1: Pattern Matching with Finite Automata](https://youtu.be/aI5OFpD1l9s)
- [Part 2: Converting Regular Expressions to Finite Automata](https://youtu.be/D6mSQTcs7jY)
- [Part 3: Regular Expressions to Finite Automata](https://youtu.be/dJ2icrMAM04)




