#ifndef finite_hpp
#define finite_hpp

#include <iostream>
#include <string>
#include <vector>
#include <set>
using std::string;
using std::vector;
using std::set;

/******************************************************************************/
class Accept {
  public:
    Accept(const string& name, size_t rank);
    string name;
    size_t rank;
};

/******************************************************************************/
class Finite {
  public:
    Finite();
    Finite(Accept* accept);

    Accept* scan(std::istream* in);
    
private:
  Accept* accept;
};

#endif
