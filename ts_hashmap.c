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
  pthread_mutex_init(&(map->mutex), NULL);

  // set up arrays
  map->table = (ts_entry_t**) malloc(capacity * sizeof(ts_entry_t*));
  map->llMutex = (pthread_mutex_t*) malloc(capacity * sizeof(pthread_mutex_t));
  
  // initialize locks for each linked list
  for (int i = 0; i < capacity; i++) {
    pthread_mutex_init(&(map->llMutex[i]), NULL);
  }

  return map;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key) {
  // iterate up
  pthread_mutex_lock(&(map->mutex));
  map->numOps++;
  pthread_mutex_unlock(&(map->mutex));

  // find index of this key and lock that lock
  int index = key % (map->capacity);
  pthread_mutex_lock(&(map->llMutex[index]));

  // check if this linked list exists yet
  if (map->table[index] == NULL) {
    pthread_mutex_unlock(&(map->llMutex[index]));
    return INT_MAX;
  }

  // get first entry in this linked list
  ts_entry_t *entry = map->table[index];

  // check all but the last entry
  do {
    if (entry->key == key) {
      pthread_mutex_unlock(&(map->llMutex[index]));
      return entry->value;
    }
    entry = entry->next;
  } while (entry->next != NULL);

  // check last entry
  if (entry->key == key) {
    pthread_mutex_unlock(&(map->llMutex[index]));
    return entry->value;
  }

  // we didn't find it! return back
  pthread_mutex_unlock(&(map->llMutex[index]));
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
  // iterate up
  pthread_mutex_lock(&(map->mutex));
  map->numOps++;
  pthread_mutex_unlock(&(map->mutex));
  
  // find index of this key and lock that lock
  int index = key % (map->capacity);
  pthread_mutex_lock(&(map->llMutex[index]));

  // check if this linked list exists
  if (map->table[index] == NULL) {
    // no entries in this linked list; create a new one!
    map->table[index] = (ts_entry_t*) malloc(sizeof(ts_entry_t));
    map->table[index]->key = key;
    map->table[index]->value = value;
    map->table[index]->next = NULL;

    // iterate up
    pthread_mutex_lock(&(map->mutex));
    map->size++;
    pthread_mutex_unlock(&(map->mutex));

    // unlock and return
    pthread_mutex_unlock(&(map->llMutex[index]));
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
      pthread_mutex_unlock(&(map->llMutex[index]));
      return oldVal;
    }

    // loop through all nodes in this chain looking for the key (or the last node)
    while ((entry->next) != NULL) {
      if (entry->key == key) {
        // there was an entry with the given input key! update it!
        int oldVal = entry->value;
        entry->value = value;
        pthread_mutex_unlock(&(map->llMutex[index]));
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

    // if we reach this point, we've made a new node, so return INT_MAX and increment size
    pthread_mutex_lock(&(map->mutex));
    map->size++;
    pthread_mutex_unlock(&(map->mutex));

    pthread_mutex_unlock(&(map->llMutex[index]));
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
  // iterate up
  pthread_mutex_lock(&(map->mutex));
  map->numOps++;
  pthread_mutex_unlock(&(map->mutex));

  // find index of this key and lock that lock
  int index = key % (map->capacity);
  pthread_mutex_lock(&(map->llMutex[index]));

  // check if table has an entry
  if (map->table[index] == NULL) {
    // if not, do nothing and just return INT_MAX
    pthread_mutex_unlock(&(map->llMutex[index]));
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

      // decrement size
      pthread_mutex_lock(&(map->mutex));
      map->size--;
      pthread_mutex_unlock(&(map->mutex));

      // return old value
      pthread_mutex_unlock(&(map->llMutex[index]));
      return oldVal;
    }

    while ((entry->next) != NULL) {
      if (entry->next->key == key) {
        // next entry has the given input key! MURDER it (violently and full of HATRED)!
        ts_entry_t *next = entry->next;
        entry->next = next->next;
        int oldVal = next->value;
        free(next);

        // decrement size
        pthread_mutex_lock(&(map->mutex));
        map->size--;
        pthread_mutex_unlock(&(map->mutex));

        // return old value
        pthread_mutex_unlock(&(map->llMutex[index]));
        return oldVal;
      }
      // try next entry
      entry = entry->next;
    }
  }

  // unlock and return
  pthread_mutex_unlock(&(map->llMutex[index]));
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
  pthread_mutex_destroy(&(map->mutex)); // free main lock

  for (int i = 0; i < map->capacity; i++) { // free linked lists and their locks
    pthread_mutex_destroy(&(map->llMutex[i]));
    if (map->table[i] != NULL) {
      ts_entry_t *entry = map->table[i];
      while(entry->next != NULL) {
        ts_entry_t *temp = entry->next;
        free(entry);
        entry = temp;
      }
      free(map->table[i]);
    }
    
    // free remaining data structures
    free(map->llMutex);
    free(map->table);
    free(map);
  }
}