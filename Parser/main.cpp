/*******************************************************************************
 * Solves for the parse table of a grammar.  Reads the user defined grammar from
 * the standard input, then solves for all of the parse states.  After solving
 * for the parse states, writes the source code for the parse table to the
 * standard output.
 */

#include "grammar.hpp"
#include "code.hpp"

#include <iostream>

/******************************************************************************/
int
main(int argc, const char * argv[])
{
    Grammar grammar;
    bool ok = grammar.read_grammar(std::cin);
    if (!ok) {
        std::cerr << "Unable to read grammar.\n";
        return 1;
    }

    grammar.solve_states();
    Code::write(grammar, std::cout);

    return 0;
}
