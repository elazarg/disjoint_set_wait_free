#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef uintptr_t disj_elem;

/*
	Initially true iff u == v. This changes by unite().
	Although the operation is only a query, it does have (user-invisible) side effect, to improve the tree structure.
*/
bool same_set(disj_elem u, disj_elem v);


/* 
  Return value: like same_set(u, v)
  Effect: following calls to same_set(u, v) yield true.
*/
bool unite(disj_elem u, disj_elem v);

void init_arena(disj_elem* _arena, size_t size);
