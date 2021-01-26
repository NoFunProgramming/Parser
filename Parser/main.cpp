#include "finite.hpp"
#include "regex.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <iostream>
#include <sstream>

/******************************************************************************/
void
test_grammar()
{
    Parser parser;
    
    string test =
        "'a'"
        ""
        "E: T EP;"
        "EP: 'a' E EP | ;"
        //"T: F TP;"
        //"TP: 'm' F TP | ;"
        //"F: 'l' E 'r' | 'id';"
    ;
    
    std::stringstream in(test);
    
    parser.read_grammar(in);
    parser.solve();
    //parser.print_grammar(std::cout);
    parser.write(std::cout);
}

/*******************************************************************************
 * Writes the source code for a lexer.  This class combines multiple regular
 * expressions into a single deterministic finite automaton (DFA).  After adding
 * all expressions, call solve and then write to generate the source code.
 *
 * The output source code will define a structure for each state in DFA. This
 * structure contains a method that take a character and returns either the next
 * state in the DFA or a null pointer.  The null indicates that the pattern
 * matching is complete and the accepted value, if any, for the current state is
 * the type of token identified in the string.
 */
void
test_lexer()
{
    Accept num("number", 0);
    Accept id("identifier", 1);

    Lexer lexer;
    lexer.add(&num, "[0-9]+");
    lexer.add(&id, "[a-e]([a-e]|[0-9])*");

    lexer.solve();
    lexer.write(std::cout);
}

/*******************************************************************************
 * Converts regular expressions into finite automata for finding patterns in
 * strings.  After building, the Regex object will contain a non-deterministic
 * finite automaton (NFA).  Define a start state and connect it to the first
 * state of each NFA.  Calling scan from this new start will return the
 * accepted pattern, defined by a regular expession, found in the string.
 */
void
test_regex()
{
    Accept number("number", 0);
    Accept identifier("identifier", 1);
    
    unique_ptr<Regex> num = Regex::parse("[0-9]+", &number);
    unique_ptr<Regex> id  = Regex::parse("[a-z]([a-z]|[0-9])*", &identifier);
    if (!num || !id) {
        std::cerr << "Unable to parse expression.\n";
        return;
    }

    Finite start;
    start.add_epsilon(num->get_start());
    start.add_epsilon(id->get_start());
    
    std::stringstream in("test var3 12");
    
    while (in.peek() != EOF) {
        in >> std::ws;
        Accept* accept = start.scan(&in);
        if (accept) {
            std::cout << "Found a " << accept->name << ".\n";
        } else {
            std::cout << "Did not match expression.\n";
            break;
        }
    }
    
}

/*******************************************************************************
 * Defines an automaton to match tokens in a string.  Outputs from each state
 * determine the next active states based on the characters read from the
 * stream.  After connecting the states scan the input from the start state,
 * which moves between states while reading, to match words and numbers.
 */
void
test_finite()
{
    Accept word("word", 0);
    Accept number("number", 1);
    
    Finite start;
    Finite state_word(&word);
    Finite state_number(&number);
    Finite state_sign;
    
    start.add_out('a', 'z', &state_word);
    start.add_out('-', &state_sign);
    start.add_epsilon(&state_sign);
    
    state_word.add_out('a', 'z', &state_word);
    state_sign.add_out('0', '9', &state_number);
    state_number.add_out('0', '9', &state_number);
    
    std::stringstream in("hello 123 -456 world");
    
    while (in.peek() != EOF) {
        in >> std::ws;
        Accept* accept = start.scan(&in);
        if (accept) {
            std::cout << "Found a " << accept->name << ".\n";
        }
    }
}

/******************************************************************************/
int
main(int argc, const char * argv[])
{
    //test_finite();
    //test_regex();
    //test_lexer();
    test_grammar();
}

