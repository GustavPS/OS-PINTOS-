/* do not forget the guard against multiple includes */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAP_SIZE 128
#define PANIC() exit(1)

typedef char* value_t;
typedef int key_t;

struct map
{
    value_t content[MAP_SIZE];
    int current_size;
};

void map_init(struct map*);
key_t map_insert(struct map*, value_t);
value_t map_find(struct map*, key_t);
value_t map_remove(struct map*, key_t);
void map_for_each(struct map*, void (*exec)(key_t, value_t, int), int);
void map_remove_if(struct map*, bool(*cond)(key_t, value_t, int), int);