#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void halt(void);
void exit(int32_t*);
int read(int32_t*);
int write(int32_t*);

#endif /* userprog/syscall.h */
