#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"
#define DBG(format, ...) printf(format "\n", ##__VA_ARGS__)

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:
   
   int sys_read_arg_count = argc[ SYS_READ ];
   
   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1, 
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

// Returns fp or NULL
struct file* get_file(struct thread* t, int fd)
{
  return map_find(&t->open_file_table, fd);
}



// FUNCTIONS FOR SYSCALLS
void halt()
{
  DBG("# Rad %d i filen %s SYS_HALT interupt", __LINE__, __FILE__);
  power_off();
}

void exit(int32_t* esp)
{
  int exit_code = *(esp+1);
  struct thread* current = thread_current();
  DBG("# SYS_EXIT med koden %d på tråden %s med ID: %d", exit_code, current->name, current->tid);
  thread_exit();
}

int read(int32_t* esp)
{
  int fd          = *(esp+1);
  char *buffer    = (char*) *(esp+2);
  int length      = *(esp+3);
  int count       = 0;

  switch (fd)
  {
    case (STDIN_FILENO): // Läsa från tangentbordet
    {
      char c;
      while(count < length)
      {
        c = input_getc(); // Hämta en karaktär från tangentbordet
        if(c == '\r') c = '\n';
        putbuf(&c, 1);    // Skriv karaktären till terminalen
        *buffer++ = c;      // Ändra värdet i buffern
        count++;
      }
      return count;
    }
    default:
    {
      struct file* fp = get_file(thread_current(), fd);
      if(fp == NULL) return -1;
      return file_read(fp, buffer, length);
    }
  }
}

int write(int32_t* esp)
{
  int fd          = *(esp+1);
  char *buffer    = (char*) *(esp+2);
  int length      = *(esp+3);

  switch(fd)
  {
    case (STDOUT_FILENO):
    {
      putbuf(buffer, length);
      return length;
    }
    default:
    {
      // För filer
      struct file* fp = get_file(thread_current(), fd);
      if(fp == NULL) return -1;
      return file_write(fp, buffer, length);
    }
  }
}

int open(int32_t* esp)
{
  char*  file_name = (char*) *(esp+1);
  struct file* fp  = filesys_open(file_name);
  int    fd        = -1;



  if(fp != NULL) {
    struct thread* current = thread_current();
    fd = map_insert(&current->open_file_table, fp);
  }
  return fd;
}

bool create(int32_t* esp)
{
  char* file_name       = (char*) *(esp+1);
  unsigned initial_size = *(esp+2);

  return filesys_create(file_name, initial_size);
}

void close(int32_t* esp)
{
  struct thread* t = thread_current();
  int fd           = *(esp+1);
  struct file* f   = map_remove(&t->open_file_table, fd);

  file_close(f);
}

bool remove(int32_t* esp)
{
  char* file_name = (char*) *(esp+1);

  return filesys_remove(file_name);
}

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;

  // *ESP ger vilken interupt som ska köras.

  switch (*esp)
  {
    case (SYS_HALT):
    {
      halt();
      break;
    }
    case (SYS_EXIT):
    {
      exit(esp);
      break;
    }
    case (SYS_READ):
    {
      f->eax = read(esp);
      break;
    }
    case (SYS_WRITE):
    {
      f->eax = write(esp);
      break;
    }
    case (SYS_OPEN):
    {
      f->eax = open(esp);
      break;
    }
    case (SYS_CREATE):
    {
      f->eax = create(esp);
      break;
    }
    case (SYS_CLOSE):
    {
      close(esp);
      break;
    }
    case (SYS_REMOVE):
    {
      f->eax = remove(esp);
      break;
    }
    default:
    {
      printf ("Executed an unknown system call!\n");

      printf ("ESP: %d\n", *esp);
      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);
      
      thread_exit ();
    }
  }
}
