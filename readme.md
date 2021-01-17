# Generating source code for a lexer
The Lexer class combines multiple regular expressions into a single deterministic finite automaton 
(DFA).  After adding all expressions, call solve and then write to generate the source code.

```cpp
    Accept num("number", 0);
    Accept id("identifier", 1);

    Lexer lexer;
    lexer.add(&num, "[0-9]+");
    lexer.add(&id, "[a-e]([a-e]|[0-9])*");

    lexer.solve();
    lexer.write(std::cout);
```

The output source code will define a structure for each state in DFA. This structure contains a 
method that take a character and returns either the next state in the DFA or a null pointer.  The 
null indicates that the pattern matching is complete and the accepted value, if any, for the 
current state is the type of token identified in the string.

## Video Overviews

- [Part 1: Pattern Matching with Finite Automata](https://youtu.be/aI5OFpD1l9s)
- [Part 2: Converting Regular Expressions to Finite Automata](https://youtu.be/D6mSQTcs7jY)
- [Part 3: Regular Expressions to Finite Automata](https://youtu.be/dJ2icrMAM04)
