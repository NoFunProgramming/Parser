#include "finite.hpp"

using std::make_unique;

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

Finite::Out::Out(char first, char last, Finite* next):
    next    (next),
    epsilon (false),
    first   (first),
    last    (last){}

Finite::Out::Out(Finite* next):
    next    (next),
    epsilon (true),
    first   ('\0'),
    last    ('\0'){}

bool
Finite::Out::is_epsilon() {
    return epsilon;
}

bool
Finite::Out::in_range(char c) {
    if (epsilon) {
        return false;
    } else {
        return (c >= first && c <= last);
    }
}

Finite::Out*
Finite::add_out(char c, Finite* next) {
    outs.emplace_back(make_unique<Out>(c, c, next));
    return outs.back().get();
}

Finite::Out*
Finite::add_out(char first, char last, Finite* next) {
    outs.emplace_back(make_unique<Out>(first, last, next));
    return outs.back().get();
}

Finite::Out*
Finite::add_epsilon(Finite* next) {
    outs.emplace_back(make_unique<Out>(next));
    return outs.back().get();
}

