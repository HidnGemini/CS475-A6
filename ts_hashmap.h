#include <pthread.h>

// A hashmap entry stores the key, value
// and a pointer to the next entry
typedef struct ts_entry_t {
   int key;
   int value;
   struct ts_entry_t *next;
} ts_entry_t;

// A hashmap contains an array of pointers to entries,
// the capacity of the array, the size (number of entries stored), 
// the number of operations that it has run, a mutex for incrementing
// numOps, and an array of mutex to ensure each linked list are
// mutually exclusive.
typedef struct ts_hashmap_t {
   ts_entry_t **table;
   int numOps;
   int capacity;
   int size;
   pthread_mutex_t mutex;
   pthread_mutex_t *llMutex;
} ts_hashmap_t;

// function declarations
ts_hashmap_t *initmap(int);
int get(ts_hashmap_t*, int);
int put(ts_hashmap_t*, int, int);
int del(ts_hashmap_t*, int);
void printmap(ts_hashmap_t*);
void freeMap(ts_hashmap_t*);