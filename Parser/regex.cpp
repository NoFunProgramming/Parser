#include "regex.hpp"

using std::make_unique;

Regex::Regex() :
    start(nullptr) {}

unique_ptr<Regex>
Regex::parse(const string& in, Accept* accept)
{
    unique_ptr<Regex> result(make_unique<Regex>());

    return result;
}

