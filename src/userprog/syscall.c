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
#include "devices/timer.h"
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

/* Verify all addresses from and including 'start' up to but excluding
 * (start+length). */
bool verify_fix_length(void* start, int length)
{
  bool result = true;

  int start_po = pg_no(start);
  int end_po = pg_no(((char*)start) + length - 1);
  char* curr = pg_round_down((void*)start);
  for (int i = start_po; i <= end_po; i++) {
    if (pagedir_get_page(thread_current()->pagedir, curr) == NULL) {
      result = false;
      break;
    }
    curr = curr + PGSIZE;
  }
  return result;
}

/* Verify all addresses from and including 'start' up to and including
 * the address first containg a null-character ('\0'). (The way
 * C-strings are stored.)
 */
bool verify_variable_length(char* start)
{
  void* first_addr;
  char* curr = start;
  bool found_end = false;
  while (!found_end) {
    first_addr = pg_round_down((void*)curr); // Hämta första adressen i pagen som curr ligger i
    if (pagedir_get_page(thread_current()->pagedir, first_addr) == NULL) { // Om den fysiska adressem är ogiltig returnera false
      return false;
    }
    int start_page = pg_no(curr); // Hämta pagensom curr ligger på.
    int curr_page = start_page;
    while (curr_page == start_page) {
      if (*curr == '\0') { // Kolla om vi har kommit till \0, om vi har det så sätt found_end till true så vi går ut ur loopen.
        found_end = true;
        break;
      }
      // Sätt curr till nästa adress samt hämta page_number f ör den
      curr = curr + 1;
      curr_page = pg_no(curr);
    }
  }
  return true;
}

bool pointer_validation(void* p, size_t length)
{
  return !(p == NULL || is_kernel_vaddr(p) || !verify_fix_length(p, length));
}

bool variable_validation(char* p)
{
  return !(p == NULL || is_kernel_vaddr(p) || !verify_variable_length(p));
}

// Returns fp or NULL
struct file* get_file(struct thread* t, int fd)
{
  return map_find(&t->open_file_table, fd);
}

// FUNCTIONS FOR SYSCALLS
void halt()
{
  DBG("# Rad %d i filen %s SYS_HALT interupt ", __LINE__, __FILE__);
  power_off();
}

void exit(int exit_code)
{
  struct thread* current = thread_current();
  set_exit_status(exit_code);

  DBG("# SYS_EXIT med koden %d på tråden %s med ID: %d", exit_code, current->name, current->tid);
  thread_exit();
}

int read(int32_t* esp)
{
  int fd          = *(esp+1);
  char *buffer    = (char*) *(esp+2);
  int length      = *(esp+3);
  int count       = 0;

  // Kollar om buffer pekaren är korrekt
  if (!pointer_validation(buffer, length))
    exit(-1);

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

  // Kollar om buffer pekaren är korrekt
  if (!pointer_validation(buffer, length))
    exit(-1);

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
  if (!variable_validation(file_name))
    exit(-1);

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

  if (!variable_validation(file_name))
    exit(-1);
  return filesys_create(file_name, initial_size);
}

void close(int32_t* esp)
{
  struct thread* t = thread_current();
  int fd           = *(esp+1);
  struct file* f   = map_remove(&t->open_file_table, fd);

  filesys_close(f);
}

bool remove(int32_t* esp)
{
  char* file_name = (char*) *(esp+1);
  if (!variable_validation(file_name))
    exit(-1);
  return filesys_remove(file_name);
}

void seek(int32_t* esp)
{
  int fd             = *(esp+1);
  unsigned  position = *(esp+2);
  struct thread* t   = thread_current();
  struct file* fp    = get_file(t, fd);

  if(fp != NULL)
  {
    file_seek(fp, position);
  }
}

unsigned tell(int32_t* esp)
{
  int fd           = *(esp+1);
  struct thread* t = thread_current();
  struct file* fp  = get_file(t, fd);

  if(fp != NULL)
  {
    return file_tell(fp);
  }
  else
  {
    return -1;
  }
}

int file_size(int32_t* esp)
{
  int fd           = *(esp+1);
  struct thread* t = thread_current();
  struct file* fp  = get_file(t, fd);

  if(fp != NULL)
  {
    return file_length(fp);
  }
  else
  {
    return -1;
  }
}

void sleep(int32_t* esp)
{
  int ms = *(esp+1);
  timer_sleep(ms);
}

int exec(int32_t* esp)
{
  char* command_line = (char*) *(esp+1);
  if (!variable_validation(command_line))
    exit(-1);
  return process_execute(command_line);
}

int wait(int32_t* esp)
{
  int id = *(esp+1);
  return process_wait(id);
}

static void
syscall_handler (struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;

  // Validate ESP
   if (!pointer_validation(esp, sizeof(esp)))
    exit(-1);

  // Kolla alla argument
  int sys_read_arg_count = argc[*esp];
  for (int i = 1; i <= sys_read_arg_count; i++) {
    if(!pointer_validation(&esp[i], sizeof(esp[i])))
      exit(-1);
  }

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
      exit(*(esp+1));
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
    case (SYS_SEEK):
    {
      seek(esp);
      break;
    }
    case (SYS_TELL):
    {
      f->eax = tell(esp);
      break;
    }
    case (SYS_FILESIZE):
    {
      f->eax = file_size(esp);
      break;
    }
    case (SYS_PLIST):
    {
      process_print_list();
      break;
    }
    case (SYS_SLEEP):
    {
      sleep(esp);
      break;
    }
    case (SYS_EXEC):
    {
      f->eax = exec(esp);
      break;
    }
    case (SYS_WAIT):
    {
      f->eax = wait(esp);
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
