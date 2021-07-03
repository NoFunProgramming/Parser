/*******************************************************************************
 * Solves for the parse table of a grammar.  Reads the user defined grammar from
 * the standard input, then solves for all of the parse states.  After solving
 * for the parse states, writes the source code for the parse table to the
 * standard output.
 */

#include "grammar.hpp"
#include "display.hpp"
#include "code.hpp"

#include <iostream>
#include <fstream>

/******************************************************************************/
int
main(int argc, const char * argv[])
{
    Grammar grammar;
    
    if (argc > 1) {
        std::fstream in;
        in.open(argv[1]);
        if (!in) {
            std::cerr << "Unable to read input file.\n";
            return 1;
        }
        bool ok = grammar.read_grammar(in);
        if (!ok) {
            std::cerr << "Unable to read grammar.\n";
            return 1;
        }
        
        //    std::cout << "/*\n";
        //    grammar.print_grammar(std::cout);
        //    //grammar.print_states(std::cout);
        //    std::cout << "*/\n";
            
            Display::print(grammar.lexer, std::cout);
            //Display::print(grammar, std::cout);
            //Display::print(grammar, std::cout);
            
            //Code::write(grammar.lexer, std::cout);
            //Code::write(grammar, std::cout);

    }
    else {
        bool ok = grammar.read_grammar(std::cin);
        if (!ok) {
            std::cerr << "Unable to read grammar.\n";
            return 1;
        }
    }    

    grammar.solve_states();
    Code::write(grammar, std::cout);

    return 0;
}
