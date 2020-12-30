#include "finite.hpp"

Accept::Accept(const string& name, size_t rank):
    name(name),
    rank(rank){}

Finite::Finite():
    accept(nullptr){}

Finite::Finite(Accept* accept):
    accept(accept){}

Accept*
Finite::scan(std::istream* in)
{
    return nullptr;
}
