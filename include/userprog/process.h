#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_create_initd (const char *file_name);
tid_t process_fork (const char *name, struct intr_frame *if_);
int process_exec (void *f_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (struct thread *next);
/* project 2 */
void argument_stack(char **argv, int argc, void **rsp);
struct thread *get_child_process(int pid);
int process_add_file(struct file *f);
struct file *process_get_file(int fd);
void process_close_file(int fd);
struct thread *get_child_process(int pid);
//! project 3
static bool load_segment(struct file *file, off_t ofs, uint8_t *uapge, uint32_t read_bytes, uint32_t zero_bytes, bool writable);
bool lazy_load_segment(struct page *page, void *aux);
struct for_lazy{
	struct file *file;
	off_t ofs;
	uint32_t read_bytes;
	uint32_t zero_bytes;
};

#endif /* userprog/process.h */
