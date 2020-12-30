#include <iostream>

#include "finite.hpp"

int
main(int argc, const char * argv[])
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

    return 0;
}
