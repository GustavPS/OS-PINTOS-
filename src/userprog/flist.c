#include "flist.h"
#include "filesys/filesys.h"

void node_init(struct node* n, value_t v, key_t k) {
  n->value = v;
  n->key = k;
  n->next = NULL;
}


void map_init(struct map* m) {
  m->first = NULL;
}

void map_destroy(struct map* m) {
  struct node* current = m->first;
  struct node* next = m->first;
  while(next != NULL)
  {
    current = next;
    next = current->next;
    filesys_close(current->value);
    free(current);
  }
}


key_t map_insert(struct map* m, value_t v) {
  // Loopa igenom tills första NULL värde hittas, ersätt det med det nya värdet
  int key = 2;
  struct node* current = m->first;
  struct node* prevNode;
  while(current != NULL) {
    prevNode = current;
    current = current->next;
    key++;
  }

  struct node* newValue = (struct node*) malloc(sizeof(struct node));
  node_init(newValue, v, key);

  if(key == 2) {
    m->first = newValue;
  } else {
    prevNode->next = newValue;
  }

  current = newValue;
  return key;
}

value_t map_find(struct map* m, key_t k) {
  struct node* current = m->first;

  while(current != NULL && current->key != k) {
    current = current->next;
  }

  if(current == NULL) {
    return NULL;
  } else {
    return current->value;
  }
}

value_t map_remove(struct map* m, key_t k) {
  struct node* current = m->first;
  struct node* prevNode = m->first;
  int count = 0;
  if(current == NULL) // If list is empty, return NULL.
  {
    return NULL;
  }

  while(current->key != k && current->next != NULL) {
    prevNode = current;
    current = current->next;
    count++;
  }

  if(k == current->key) {
    value_t value = current->value;
    if(count == 0) // Special case, if we removed the only item, point maps first to NULL.
    {
      m->first = NULL;
    }
    else
    {
      prevNode->next = current->next;
    }
    free(current);
    return value;
  } else {
    return NULL;
  }
}

void map_for_each(struct map* m,
                  void (*exec)(key_t, value_t, int),
                  int aux) {
  struct node* current = m->first;

  while(current != NULL) {
    if(current->value != NULL) {
      exec(current->key, current->value, aux);
    }
    current = current->next;
  }
}

void map_remove_if(struct map* m,
                   bool (*cond)(key_t, value_t, int),
                   int aux) {
  struct node* current = m->first;

  while(current != NULL) {
    if(current->value != NULL) {
      if(cond(current->key, current->value, aux)) {
        map_remove(m, current->key);
      }
    }
    current = current->next;
  }
}
