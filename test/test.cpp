#include "finite.hpp"
#include "regex.hpp"
#include "lexer.hpp"
#include "generator.hpp"
#include "code.hpp"

#include <sstream>
#include <iostream>

void test_finite();
void test_regex();
void test_lexer();
void test_grammar();

/******************************************************************************/
int
main(int argc, const char * argv[])
{
    //test_finite();
    //test_regex();
    //test_lexer();
    test_grammar();

    return 0;
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

/*******************************************************************************
 * Writes the source code for a parser.  The class reads in a user defined
 * grammar and outputs source code that can be compiled into another program to
 * parse an input string.  Action methods can be associated with every rule of
 * the grammar and are called when that pattern is found within the input.
 */
void
test_grammar()
{
    std::string test =
        "#include \"parser.hpp\"\n"
        "'num'<Expr>   [0-9]+   &scan_num;"
        ""
        "/* Comment */"
        "total<Expr>: add       &reduce_total"
        "    ;"
        "add<Value>: mul        &reduce_add"
        "    | add '+' mul      &reduce_add_mul"
        "    ;"
        "mul<Expr>: 'num'       &reduce_mul"
        "    | mul '*' 'num'    &reduce_mul_num"
        "    ;";
    
    std::stringstream in(test);
 
    Grammar generator;
    
    bool ok = generator.read_grammar(in);
    if (!ok) {
        std::cerr << "Unable to read grammar.\n";
        return;
    }
    generator.solve();
//    std::cout << "/*";
//    parser.print_grammar(std::cout);
//    parser.print_states(std::cout);
//    std::cout << "*/";
    //generator.write(std::cout);
    
    std::vector<State*> states;
    for (auto& s : generator.states) {
        states.push_back(s.get());
    }
    
    Code::write(generator, states, std::cout);
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
    
    std::unique_ptr<Regex> num = Regex::parse("[0-9]+", &number);
    std::unique_ptr<Regex> id  = Regex::parse("[a-z]([a-z]|[0-9])*", &identifier);
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
