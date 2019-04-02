#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

struct thread;

void syscall_init (void);

void halt(void);
void exit(int32_t*);
int read(int32_t*);
int write(int32_t*);
int open(int32_t*);
bool create(int32_t*);
void close(int32_t*);
bool remove(int32_t*);

struct file* get_file(struct thread*, int);

#endif /* userprog/syscall.h */
