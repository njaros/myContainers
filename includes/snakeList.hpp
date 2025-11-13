#ifndef SNAKE_LIST_HPP
#define SNAKE_LIST_HPP

class Node {
    Node();

    public:

    int Pi;
    Node *next;
    Node(const Node &o): Pi(o.Pi), next(o.next) {}
    Node(int n, Node *prev, Node *first): Pi(n), next(nullptr) {
        next = first;
        if (prev)
            prev->next = this;
    }
    ~Node() {}

    Node &operator=(const Node &o) {
        if (&o == this)
            return *this;
        Pi = o.Pi;
        next = o.next;
        return *this;
    }
};


/**
 * That's a list wich has his last element connected to the first element.
 * Only works for int atm.
 * Most of the times, that's more cleaver to use a vector with using as index idx % vector.size()
 * 
 * Upgrade to do : template the value_type, includes iterators and more method
*/
class SnakeList {

    Node *previous;

    public:

    Node *first;
    Node *current;

    SnakeList(): current(nullptr), first(nullptr), previous(nullptr) {};
    SnakeList(const SnakeList &o): current(o.current), first(o.first), previous(o.previous) {}
    ~SnakeList() {
        current = first;
        bool start = false;
        while (current && (current != first || !start)) {
            if (!start)
                start = true;
            Node *next = current->next;
            delete(current);
            current = next;
        }
    }
    SnakeList &operator=(const SnakeList &o) {
        if (&o == this)
            return *this;
        current = first;
        bool start = false;
        while (current && (current != first || !start)) {
            if (!start)
                start = true;
            Node *next = current->next;
            delete(current);
            current = next;
        }
        first = o.first;
        current = o.current;
        previous = o.previous;
        return *this;
    }

    void addElement(int nb) {
        if (!first) {
            first = new Node(nb, nullptr, nullptr);
            first->next = first;
            previous = first;
        }
        else {
            Node *newOne = new Node(nb, previous, first);
            previous = newOne;
        }
    }

    int getCurrent(void) {
        int nb;
        if (!current)
            current = first;
        nb = current->Pi;
        current = current->next;
        return nb;
    }

};

#endif