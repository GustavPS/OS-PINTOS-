#include <stddef.h>

#include "plist.h"

void pnode_init(struct pnode* pn, tid_t proc_id, tid_t parent_id, const char* name)
{
  pn->proc_id = proc_id;
  pn->parent_id = parent_id;
  pn->name = malloc(strlen(name)+1);
  strlcpy(pn->name, name, strlen(name)+1);
  pn->exit_status = -1;
  pn->alive = true;
  pn->parent_alive = true;
  pn->next = NULL;
}

void pnode_copy(struct pnode* to, struct pnode* from)
{
  to->proc_id = from->proc_id;
  to->parent_id = from->parent_id;
  to->name = from->name;
  to->exit_status = from->exit_status;
  to->alive = from->alive;
  to->parent_alive = from->parent_alive;
  to->next = from->next;
}

void plist_init(struct plist* pl)
{
  pl->first = NULL;
}

int plist_insert(struct plist* pl, tid_t proc_id, tid_t parent_id, char* name)
{
  struct pnode* current = pl->first;
  struct pnode* prevNode;
  int count = 0;
  while(current != NULL) {
    prevNode = current;
    current = current->next;
    count++;
  }

  struct pnode* newValue = (struct pnode*) malloc(sizeof(struct pnode));
  pnode_init(newValue, proc_id, parent_id, name);

  if (count == 0)
  {
    pl->first = newValue;
  }
  else
  {
    prevNode->next = newValue;
  }

  return proc_id;
}

struct pnode plist_find(struct plist* pl, int proc_id)
{
  struct pnode* current = pl->first;

  while (current != NULL && current->proc_id != proc_id)
  {
    current = current->next;
  }

  struct pnode returnNode;
  if (current == NULL)
  {
    memset(&returnNode, 0, sizeof(struct pnode)); // Set everything to "null"
  }
  else
  {
    pnode_copy(&returnNode, current);
    returnNode.next = NULL;
  }
  return returnNode;
}

void plist_remove(struct plist* pl, int proc_id)
{
  struct pnode* current = pl->first;
  struct pnode* prevNode = pl->first;
  int count = 0;
  bool should_remove = false;

  if (current != NULL) { // Fortsätt bara om listan inte är tom.
    while(current != NULL) {
      should_remove = false;

      // 1. Gå igenom alla barn, sätt parent alive = false, om barnet har alive = false ta bort barnet
      if (current->parent_id == proc_id) {
        current->parent_alive = false;

        if(!current->alive) { // Ta bort barnet om den inte lever
          if (count == 0) { // Special case, om vi tar bort första itemet
            pl->first = current->next;
          } else {
            prevNode->next = current->next;
          }
          should_remove = true;
        }
      // 2. Sätt egen alive = false, om parent alive == false, ta bort oss själva
      } else if (current->proc_id == proc_id) {
        current->alive = false;
        if (!current->parent_alive) {
          if (count == 0) { // Special case, om vi tar bort första itemet
            pl->first = current->next;
          } else {
            prevNode->next = current->next;
          }
          should_remove = true;
        }
      }

      struct pnode* nextNode = current->next;
      if(should_remove) {
        free(current->name);
        free(current);
        current = nextNode;
      } else {
        prevNode = current;
        current = nextNode;
        count++;
      }
    }
  }
}

void plist_print(struct plist* pl)
{
  struct pnode* current = pl->first;
  printf("-----------------------------------------\n");
  while(current != NULL) {
    printf("Thread: %s id: %d\n", current->name, current->proc_id);
    printf(" - proc_id: %d\n", current->proc_id);
    printf(" - parent_id: %d\n", current->parent_id);
    printf(" - exit_status: %d\n", current->exit_status);
    printf(" - alive: %d\n", current->alive);
    printf(" - parent_alive: %d\n", current->parent_alive);
    printf("-----------------------------------------\n\n");
    current = current->next;
  }
}
