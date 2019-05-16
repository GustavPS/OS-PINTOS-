#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

struct thread;

void syscall_init (void);

bool verify_fix_length(void*, int);
bool verify_variable_length(char*);
bool pointer_validation(void*, size_t);
bool variable_validation(char*);
void halt(void);
void exit(int);
int read(int32_t*);
int write(int32_t*);
int open(int32_t*);
bool create(int32_t*);
void close(int32_t*);
bool remove(int32_t*);
void seek(int32_t*);
unsigned tell(int32_t*);
int file_size(int32_t*);
int exec(int32_t*);
void sleep(int32_t*);
int wait(int32_t*);

struct file* get_file(struct thread*, int);
void close_all_files(struct thread*);

#endif /* userprog/syscall.h */
