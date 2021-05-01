# Generating source code for a lexer

The Lexer class combines multiple regular expressions into a single
deterministic finite automaton (DFA).  After adding all expressions, call solve
and then write to generate the source code.

```cpp
    Accept num("number", 0);
    Accept id("identifier", 1);

    Lexer lexer;
    lexer.add_regex(&num, "[0-9]+");
    lexer.add_regex(&id, "[a-e]([a-e]|[0-9])*");

    lexer.solve();
    lexer.write(std::cout);
```

The output source code will define a structure for each state in DFA. This
structure contains a method that take a character and returns either the next
state in the DFA or a null pointer.  The null indicates that the pattern
matching is complete and the accepted value, if any, for the current state is
the type of token identified in the string.

## Video Overviews

- [Part 1: Pattern Matching with Finite Automata](https://youtu.be/aI5OFpD1l9s)
- [Part 2: Converting Regular Expressions to Finite Automata](https://youtu.be/D6mSQTcs7jY)
- [Part 3: Regular Expressions to Finite Automata](https://youtu.be/dJ2icrMAM04)

## Generating source code for the parse table

Solves for the parse table of a grammar.  User provided action methods for each
grammar rule and the parse table can later be combined to build a parser or a
compiler.

The program generates parse tables for context free grammars.  The most
common way to represent a context free grammar is in Backus-Naur Form or BNF.
The grammar is defined by two types of symbols: terminals and nonterminals. The
nonterminal themselves are define as a sequence of symbols, known as a
production rule. Terminals are shown in quotes and are defined by a pattern of
input characters.

Terminals are defined by regular expressions.  When running the generated source
code, the parser will identify these patterns in the input string and add the
matching terminal to the stack of the parser.  The parser will then check this
stack for sequences that match one of the rules.  If a match is found, that
rule's nonterminal will replace those symbols on the stack.
```
    'num' [0-9]+;
```
Similar to nonterminals, the terminals can also have a user defined action and a
type. When a pattern is match in the input string, the source calls the with the
matching string and expects the method to return a value of the provided type.
```
    'num'<int> [0-9]+ &read_int;
```
The input rules are written as a nonterminal followed by zero or more symbols.
If there is more than one rule associated the same nonterminal, they are
separated by a vertical bar.  A semicolon indicates the end of the rules for a
given nonterminal.
```
    add: mul | add '+' mul;
```
At the end of each rule can be optional user defined action, indicated by an
ampersand.  The action method is called by the generated source code when that
rule's sequence of symbols is found.  Each nonterminal can also have a type
which is defined within angle brackets.  The generated source code calls the
action method with a input argument for each rule's symbol that has a type.
```
    add<int64> add '+' mul &reduce_add_mul;
```
At runtime the program will read the grammar rules from the standard input and
write the generated source code for the parse table to the standard output.
```
'num'<Expr>   [0-9]+    &scan_num;

total<Expr>: add        &reduce_total;

add<Expr>: mul          &reduce_add
    | add '+' mul       &reduce_add_mul;
    
mul<Expr>: 'num'        &reduce_mul
    | mul '*' 'num'     &reduce_mul_num;
```
