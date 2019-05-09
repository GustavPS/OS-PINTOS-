#ifndef _PLIST_H_
#define _PLIST_H_

#include "process.h" // Inkludera bara bool och tid_t
#include <string.h>
#include "threads/synch.h"


/* Place functions to handle a running process here (process list).
   
   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.
     
   - A function that print the entire content of the list in a nice,
     clean, readable format.
     
 */

struct pnode
{
  char* name;
  tid_t proc_id;
  tid_t parent_id;
  int exit_status;
  bool alive;
  bool parent_alive;
  struct semaphore sema;
  struct pnode* next;
};

void pnode_init(struct pnode*,tid_t, tid_t, const char*);
void pnode_copy(struct pnode*, struct pnode*); // Copy from the right to the left


struct plist
{
  struct pnode* first;
  struct lock plist_lock;
};

void         plist_init(struct plist*);
void         plist_destroy(struct plist*);
int          plist_insert(struct plist*, tid_t, tid_t, char*); // proc_id, parent_id, name
struct pnode* plist_find(struct plist*, int); // Find by proc_id
void         plist_remove(struct plist*, int, bool); // Remove by proc_id
void         plist_print(struct plist*);
void         plist_update_exit(struct plist*, tid_t, int);

#endif
