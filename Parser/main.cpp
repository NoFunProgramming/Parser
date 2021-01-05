/**
 * Identify words and numbers from an input stream using finite automata.
 *
 */

#include "finite.hpp"
#include "regex.hpp"

#include <iostream>
#include <sstream>

void
test_regex()
{
    Accept number("number", 0);
    Accept identifier("identifier", 1);
    
    unique_ptr<Regex> num = Regex::parse("[0-9]+", &number);
    unique_ptr<Regex> id  = Regex::parse("[a-z]([a-z]|[0-9])*", &identifier);
    if (!id) {
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













/**
 * This program defines an automata with four states.  Two of the states are
 * accepting and mark the match of a word or number.  In addition to the start,
 * another state is for reading an optional negative sign in front of numbers.
 * Outputs from each state determine the next active states based on the
 * characters read from the stream.  After connecting the states scan the input
 * from the start state, which moves between states while reading, to match
 * words and numbers.
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

int
main(int argc, const char * argv[])
{
    test_regex();
}

