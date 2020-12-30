#ifndef regex_hpp
#define regex_hpp

#include "finite.hpp"

#include <string>
#include <iostream>
using std::string;
using std::istream;
using std::unique_ptr;

class Regex
{
  public:
    Regex();
    static unique_ptr<Regex> parse(const string& in, Accept* accept);

    Finite* get_start();

  private:
    Finite* start;
};

#endif
