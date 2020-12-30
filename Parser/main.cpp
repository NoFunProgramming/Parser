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

    std::cout << "Hello, World!\n";
    return 0;
}
