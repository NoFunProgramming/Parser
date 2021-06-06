#include "node.hpp"

/******************************************************************************/
Node::Node(size_t id):
    id(id),
    accept(nullptr) {}

void
Node::add_finite(Finite* finite) {
    items.insert(finite);
}

void
Node::add_finite(std::set<Finite*>& finites) {
    items.insert(finites.begin(), finites.end());
}

void
Node::add_next(int first, int last, Node* next) {
    nexts[Range(first, last)] = next;
}

Node*
Node::get_next(int c)
{
    for (auto next : nexts) {
        if (c >= next.first.first && c <= next.first.last) {
            return next.second;
        }
    }
    return nullptr;
}

void
Node::move(char c, std::set<Finite*>* found) {
    for (Finite* item : items) {
        item->move(c, found);
    }
}

/**
 * After folliwng outputs that contain the input character, add the targets of
 * empty transitions to the newly found set of states.
 */
void
Node::solve_closure()
{
    std::vector<Finite*> stack;
    stack.insert(stack.end(), items.begin(), items.end());
    
    while (stack.size() > 0) {
        Finite* check = stack.back();
        stack.pop_back();
        check->closure(&items, &stack);
    }
}

/**
 * Since the DFA states contain multiple finite states, determine the NFA state
 * with the lowest ranked accept to represent the pattern matched by the
 * current DFA state.
 */
void
Node::solve_accept()
{
    auto lowest = min_element(items.begin(), items.end(), Finite::lower_rank);
    if (lowest != items.end()) {
        accept = (*lowest)->accept;
    }
}

void
Node::replace(std::map<Node*, Node*> prime)
{
    for (auto& next : nexts) {
        if (prime.count(next.second) > 0) {
            next.second = prime[next.second];
        }
    }
}

void
Node::reduce()
{
    std::map<Node::Range, Node*> updated;

    auto itr = nexts.begin();
    while (itr != nexts.end()) {
        int first = itr->first.first;
        int last  = itr->first.last;
        Node* next = itr->second;
        itr++;

        bool matches = true;
        while (itr != nexts.end() && matches) {
            matches = itr->second == next && (last + 1) == itr->first.first;
            if (matches) {
                last = itr->first.last;
                itr++;
            }
        }
        updated[Node::Range(first, last)] = next;
    }
    nexts = updated;
}

bool
Node::lower(Node* left, Node* right)
{
    if (left->accept && right->accept) {
        return left->accept->rank < right->accept->rank;
    } else if (left->accept) {
        return true;
    } else {
        return false;
    }
}

/**
 * Writes the source code for a single state of the lexer.  The source code
 * defines a structure and a method for each state.  The method takes an input
 * character and returns either a new state in the DFA or a null pointer.
 */
void
Node::write(std::ostream& out)
{
    if (nexts.size() == 0) {
        return;
    }
    
    out << "int\n";
    out << "scanX" << id << "(int c) {\n";
    for (auto next : nexts) {
        out << "    if (";
        next.first.write(out);
        out << ") { return " << next.second->id << "; }\n";
    }
    out << "    return -1;\n";
    out << "}\n\n";
}

void
Node::write_struct(std::ostream& out)
{
    if (nexts.size() > 0) {
        out << "    {&scanX" << id;
    } else {
        out << "    {nullptr";
    }

    if (accept) {
        out << ", &term" << accept->rank;
        if (accept->scan.size() > 0) {
            out << ", &scan" << accept->rank << "";
        } else {
            out << ", nullptr";
        }
    } else {
        out << ", nullptr";
        out << ", nullptr";
    }
    out << "},\n";
}

/******************************************************************************/
Node::Range::Range(int first, int last):
    first(first), last(last) {}

bool
Node::Range::operator<(const Range& other) const {
    return last < other.first;
}

void
Node::Range::write(std::ostream& out) const
{
    if (first == last) {
        if (isprint(first) && first != '\'') {
            out << "c == '" << (char)first << "'";
        } else {
            out << "c == " << first << "";
        }
    } else {
        if (isprint(first) && isprint(last)
                && first != '\'' && last != '\'') {
            out << "(c >= '" << (char)first << "')";
            out << " && ";
            out << "(c <= '" << (char)last << "')";
        } else {
            out << "(c >= " << first << ")";
            out << " && ";
            out << "(c <= " << last << ")";
        }
    }
}

