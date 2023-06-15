#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "threads/synch.h"

void syscall_init (void);
void validate_address(void *addr);

struct lock filesys_lock;
#endif /* userprog/syscall.h */
