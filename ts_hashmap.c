#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_hashmap.h"

/**
 * Creates a new thread-safe hashmap. 
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity) {
  // malloc ts_hashmap_t structure memory
  ts_hashmap_t *map = (ts_hashmap_t*) malloc(sizeof(ts_hashmap_t));

  // set initial values
  map->capacity = capacity;
  map->size = 0;
  map->numOps = 0;

  map->table = (ts_entry_t**) malloc(capacity * sizeof(ts_entry_t*));

  return map;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key) {
  map->numOps++;
  int index = key % (map->capacity);

  ts_entry_t *entry = map->table[index];

  if (map->table[index] == NULL) {
    return INT_MAX;
  }

  // check all but the last entry
  do {
    if (entry->key == key) {
      return entry->value;
    }
    entry = entry->next;
  } while (entry->next != NULL);

  // check last entry
  if (entry->key == key) {
    return entry->value;
  }

  return INT_MAX;
}

/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value) {
  map->numOps++;
  int index = key % (map->capacity);

  if (map->table[index] == NULL) {
    // no entries in this linked list
    map->table[index] = (ts_entry_t*) malloc(sizeof(ts_entry_t));
    map->table[index]->key = key;
    map->table[index]->value = value;
    map->table[index]->next = NULL;
    map->size++;
    return INT_MAX;
  } else {
    // there are already entries. Are any of their keys the input key?
    // get first entry
    ts_entry_t *entry = map->table[index];

    // check if the first node is our key
    if (entry->key == key) {
      // this is an entry with the given input key! update it!
      int oldVal = entry->value;
      entry->value = value;
      return oldVal;
    }

    // loop through all nodes in this chain looking for the key (or the last node)
    while ((entry->next) != NULL) {
      if (entry->key == key) {
        // there was an entry with the given input key! update it!
        int oldVal = entry->value;
        entry->value = value;
        return oldVal;
      }
      entry = entry->next;
    }

    // if we break out of the while loop, make a new node
    ts_entry_t *newEntry = (ts_entry_t*) malloc(sizeof(ts_entry_t));
    newEntry->key = key;
    newEntry->value = value;

    // set newEntry pointer to entry's next field
    entry->next = newEntry;

    // if we reach this point, we've made a new node, so return INT_MAX (and increment size)
    map->size++;
    return INT_MAX;
  }
}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key) {
  map->numOps++;
  int index = key % (map->capacity);

  // check if table has an entry
  if (map->table[index] == NULL) {
    // if not, do nothing and just return INT_MAX
    return INT_MAX;
  } else {
    // otherwise, we must look for our key
    ts_entry_t *entry = map->table[index];

    // check if the first node is our key
    if (entry->key == key) {
      // this is an entry with the given input key! MURDER it (violently and full of HATRED)!
      map->table[index] = entry->next;
      int oldVal = entry->value;
      free(entry);
      map->size--;
      return oldVal;
    }

    while ((entry->next) != NULL) {
      if (entry->next->key == key) {
        // next entry has the given input key! update it!
        ts_entry_t *next = entry->next;
        entry->next = next->next;
        int oldVal = next->value;
        free(next);
        map->size--;
        return oldVal;
      }
      entry = entry->next;
    }
    
  }
  return INT_MAX;
}


/**
 * Prints the contents of the map (given)
 */
void printmap(ts_hashmap_t *map) {
  for (int i = 0; i < map->capacity; i++) {
    printf("[%d] -> ", i);
    ts_entry_t *entry = map->table[i];
    while (entry != NULL) {
      printf("(%d,%d)", entry->key, entry->value);
      if (entry->next != NULL)
        printf(" -> ");
      entry = entry->next;
    }
    printf("\n");
  }
}

/**
 * Free up the space allocated for hashmap
 * @param map a pointer to the map
 */
void freeMap(ts_hashmap_t *map) {
  // TODO: iterate through each list, free up all nodes
  // TODO: free the hash table
  // TODO: destroy locks
}