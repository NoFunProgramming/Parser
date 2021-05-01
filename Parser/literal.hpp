/*******************************************************************************
 * Builds a finite automata (NFA) that matches an input string against a
 * sequence of characters.
 */
#ifndef literal_hpp
#define literal_hpp

#include "finite.hpp"

/*******************************************************************************
 * NFA for matching a sequence of characters.  Each character in the pattern
 * must be a printable character.  Escape sequences that start with a forward
 * slash allow non-printable characters in the matched sequence.
 */
class Literal
{
  public:
    /** Returns the NFA if the pattern is valid, otherwise null. */
    static std::unique_ptr<Literal>
    build(const std::string& pattern, Accept* accept);
    
    Literal();

    /** After building, call start's scan method to check for a match. */
    Finite* start;

  private:
    std::vector<std::unique_ptr<Finite>> states;
    Finite* add_state();
    Finite* add_state(Accept* accept);

    Finite* parse_term(std::istream& series, Accept* accept);
};

#endif
