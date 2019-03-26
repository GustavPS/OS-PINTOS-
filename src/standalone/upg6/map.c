#include "map.h"

void map_init(struct map* m) {
    m->current_size = 0;
    for(int i = 0; i < MAP_SIZE; i++) {
        m->content[i] = NULL;
    }
}

key_t map_insert(struct map* m, value_t v) {
    if(m->current_size >= MAP_SIZE) {
        PANIC();
    }

    // Loopa igenom tills första NULL värde hittas, ersätt det med det nya värdet
    value_t *value = &m->content[0];
    key_t count = 0;
    while(*value != NULL) {
        value++;
        count++;
    }
    *value = v;
    m->current_size++;
    return count;
}

value_t map_find(struct map* m, key_t k) {
    if(k > MAP_SIZE) {
        PANIC();
    }
    return m->content[k];
}

value_t map_remove(struct map* m, key_t k) {
    if(k > MAP_SIZE) {
        PANIC();
    }

    value_t value = m->content[k];
    if(value != NULL) {
        m->current_size--;
        m->content[k] = NULL;
    }
    return value;
}

void map_for_each(struct map* m,
        void (*exec)(key_t, value_t, int),
        int aux) {
    value_t *value = &m->content[0];
    int done = 0; // Hur många värden vi processerat.
    key_t key = 0; // Key:n för nuvarande värde.
    int size = m->current_size; // Storleken av map:en

    // Kör exec() på alla där value inte är NULL (på alla värden)
    while(done < size) {
        if(*value != NULL) {
            exec(key, *value, aux);
            done++;
        }
        value++;
        key++;
    }
}

void map_remove_if(struct map* m,
        bool (*cond)(key_t, value_t, int),
        int aux) {
    value_t *value = &m->content[0];
    int done = 0; // Hur många värden vi processerat.
    key_t key = 0; // Key:n för nuvarande värde.
    int size = m->current_size; // Storleken av map:en

    // Gå igenom alla värden och ta bort det ifall cond(key, *value, aux) == true
    while(done < size) {
        if(*value != NULL) {
            if(cond(key, *value, aux)) {
                map_remove(m, key);
            }
            done++;
        }
        value++;
        key++;
    }
}