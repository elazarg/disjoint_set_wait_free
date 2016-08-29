/*
A wait-free implementation of Disjoint-Set data structure (aka Union-Find)

Direct translation of

    A Randomized Concurrent Algorithm for Disjoint Set Union
    Siddharta V. Jayanti, Robert E. Tarjan
    PODC`16, July 25-28, 2016, Chicago, IL, USA
    http://www.cis.upenn.edu/~sanjeev/papers/soda14_disjoint_set_union.pdf

C implementation by Elazar Gershuni
*/

/*
The algorithm uses addresses as identity of nodes, and takes advantage on the natural ordering of pointers.

The correctness, linerizability and termination depends on keeping `n->p >= &n`.

The asymptotic efficiency (inverse Ackerman with some parameters; constant for all practical purposes) depends on the order between addresses of nodes be independent from the calls to the API.
This is most likely the case in practice. Allocating the nodes from pools using random() can help to make sure it is.
*/

// TODO: Verify that the side-effect of failed compare_exchange is not harmful
//       (Might even be useful)

//
#include <stdlib.h>
#include <stdatomic.h>
#include "disjoint_set.h"

typedef struct node {
    _Atomic (struct node*) p;
} node;

//strategy: naive
__attribute__ ((unused))
static node* find(node* u) {
    node* p = u->p;
    if (u == p)
        return u;
    return find(p);
}

//strategy: split
static node* find_split(node* u) {
    node* p = u->p;
    node* pp = p->p;
    if (p == pp)
        return p;
    atomic_compare_exchange_weak(&u->p, &p, pp);
    return find_split(p);
}

//strategy: halve
static node* find_halve(node* u) {
    node* p = u->p;
    node* pp = p->p;
    if (p == pp)
        return p;
    atomic_compare_exchange_weak(&u->p, &p, pp);
    return find_halve(u->p);
}

static node* find_const(node* u) {
    node* p = u->p;
    if (u == p)
        return u;
    return find_const(p);
}

static bool _same_set(node* _u, node* _v) {
    node* u = find_const(_u);
    node* v = find_const(_v);
    if (u == v)
        return true;
    if (u == u->p)
        return false;

    // alternatively/additionally:
    // (can be ommitted)
    if (v < u && v == v->p)
        return false;

    return _same_set(u, v);
}

static bool _unite(node* _u, node* _v) {
    //we use these just so the compiler will not complain they are not in use
    node* u = find_split(_u);
    node* v = find_halve(_v);
    if (u == v)
        return true;
    if (u < v)
        if (atomic_compare_exchange_strong(&u->p, &u, v))
            return false;
    if (v < u)
        if (atomic_compare_exchange_strong(&v->p, &v, u))
            return false;
    return _unite(u, v);
}

__attribute__ ((unused))
static bool same_set_early_recognition(node* _u, node* _v) {
    if (_u == _v)
        return true;
    node* u = (_u < _v) ? _u : _v;
    node* v = (_u < _v) ? _v : _u;
    
    if (u == u->p)
        return false;

    node* z = u->p;
    atomic_compare_exchange_weak(&u->p, &z, z->p);
    return same_set_early_recognition(z, v);
}

__attribute__ ((unused))
static bool unite_early_linking(node* _u, node* _v) {
    if (_u == _v)
        return true;
    node* u = (_u < _v) ? _u : _v;
    node* v = (_u < _v) ? _v : _u;

    // It might be possible to use weaker memory order in the failure case
    // it does not seem that we need any guarantees at all. so memory_order_relaxed?
    if (atomic_compare_exchange_strong(&u->p, &u, v))
        return false;

    node* z = u->p;
    atomic_compare_exchange_weak(&u->p, &z, z->p);
    return same_set_early_recognition(z, v);
}

bool same_set(disj_elem u, disj_elem v) { 
	return _same_set((node*)u, (node*)v); 
}
bool unite(disj_elem u, disj_elem v) { 
	return _unite((node*)u, (node*)v); 
}

void init_arena(disj_elem* _arena, size_t size) {
    node* arena = (node*)_arena;
    for (size_t i=0; i < size; i++) {
        arena[i].p = &arena[i];
    };
}
