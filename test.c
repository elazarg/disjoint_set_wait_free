#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "disjoint_set.h"

typedef int T;

void shuffle(T* array, size_t n) {
    srand(time(NULL));
    for (size_t i=0; i < n-1; i++) {
        size_t j = i + rand() / (RAND_MAX / (n-1)) + 1;
        T temp = array[j];
        array[j] = array[i];
        array[i] = temp;
    }
}

typedef struct Q {
    disj_elem elem;
    int data;
} Q;

const char* check(bool actual, bool expected) {
	return actual == expected ? "PASS" : "FAIL";
}


bool SAMESET(Q* arr, size_t a, size_t b, bool expected) {
	bool actual = same_set(arr[a].elem, arr[b].elem);
	printf("%s: same_set(%zu, %zu): %s\n", check(actual, expected), a, b, actual?"true":"false");
	return actual == expected;
}

bool UNITE(Q* arr, size_t a, size_t b, bool before_expected) {
	SAMESET(arr, a, b, before_expected);
	bool actual = unite(arr[a].elem, arr[b].elem);
    printf("%s:   unite(%zu, %zu): %s\n", check(actual, before_expected), a, b, actual?"true":"false");
    return (actual == before_expected) & SAMESET(arr, a, b, true);
}

typedef bool command(Q*, size_t, size_t, bool);

typedef struct test {
	command* cmd;
	int a, b;
	bool expected;
} test;

int main() {
    #define WORLD_SIZE 1000
    int map[WORLD_SIZE];
    for (int i=0; i < WORLD_SIZE; i++) {
        map[i] = i;
    }
    shuffle(map, WORLD_SIZE);
    disj_elem arena[WORLD_SIZE]; 
    init_arena(arena, WORLD_SIZE);
    Q arr[10];
    for (int i=0; i < 10; i++) {
        arr[i].elem = arena[map[i]];
        arr[i].data = i;
    }
	test suite[] = {
		{SAMESET, 0, 0, true}, 
		{SAMESET, 0, 1, false}, 
		{UNITE,   1, 0, false}, 
		{SAMESET, 0, 1, true}, 
		{SAMESET, 0, 2, false}, 
		{UNITE,   2, 3, false}, 
		{SAMESET, 3, 2, true}, 
		{SAMESET, 0, 2, false}, 
		{SAMESET, 0, 3, false}, 
		{SAMESET, 1, 2, false}, 
		{SAMESET, 1, 3, false}, 
		{UNITE,   3, 0, false}, 
		{SAMESET, 3, 2, true}, 
		{SAMESET, 0, 2, true}, 
		{SAMESET, 0, 3, true}, 
		{SAMESET, 1, 2, true}, 
		{SAMESET, 1, 3, true},
	};
	int passed = 0, total = 0;
	for (size_t i=0; i < sizeof(suite) / sizeof(suite[0]); i++) {
		test t = suite[i];
		if (t.cmd(arr, t.a, t.b, t.expected))
			passed++;
		total++;
	}
	printf("Passed %d out of %d\n", passed, total);
    return 0;
}
