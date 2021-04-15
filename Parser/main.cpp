/*******************************************************************************
 * Writes a language parser.  The parser reads in grammar rules and outputs
 * source code that parses a regular language.  The grammar is defined by two
 * types of symbols: terminals and nonterminals.  The nonterminal themselves
 * are define as a sequence of symbols, known as a production rule. Terminals
 * are shown in quotes and are defined by a pattern of input characters.
 *
 * Terminals are defined by regular expressions.  When running the generated
 * source code, the parser will identify these patterns in the input string and
 * add the matching terminal to the stack of the parser.  The parser will then
 * check this stack for sequences that match one of the rules.  If a match is
 * found, that rule's nonterminal will replace those symbols on the stack.
 *
 *      'num' [0-9]+;
 *
 * Similar to nonterminals, the terminals can also have a user defined action
 * and a type. When a pattern is match in the input string, the source calls the
 * with the matching string and expects the method to return a value of the
 * provided type.
 *
 *    'num'<int> [0-9]+ &read_int;
 *
 * The input rules are written as a nonterminal followed by zero or more
 * symbols.  If there is more than one rule associated the same nonterminal,
 * they are separated by a vertical bar.  A semicolon indicates the end of the
 * rules for a given nonterminal.
 *
 *      add: mul | add '+' mul;
 *
 * At the end of each rule can be optional user defined action, indicated by an
 * ampersand.  The action method is called by the generated source code when
 * that rule's sequence of symbols is found.  Each nonterminal can also have a
 * type which is defined within angle brackets.  The generated source code calls
 * the action method with a input argument for each rule's symbol that has a
 * type.
 *
 *      add<int64> add '+' mul &reduce_add_mul;
 *
 * At runtime program will read the grammar rules from the standard input and
 * write the generated source code to the standard output.
 */

#include "generator.hpp"
#include "code.hpp"

#include <iostream>

/******************************************************************************/
int
main(int argc, const char * argv[])
{
    Generator generator;

    bool ok = generator.read_grammar(std::cin);
    if (!ok) {
        std::cerr << "Unable to read grammar.\n";
        return 1;
    }

    generator.solve();
    generator.write(std::cout);

    return 0;
}
