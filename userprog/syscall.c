#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
<<<<<<< HEAD
#include "include/lib/user/syscall.h"
#include "filesys/filesys.h"

#define READ_FILE_NUMBER 0
#define WRITE_FILE_NUMBER 1
=======
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/process.h"
#include "threads/synch.h"
// 오류 를 일으켜서 추가한 라이브러리.
#include "devices/input.h"
#include "lib/kernel/stdio.h"
#include "threads/palloc.h"
>>>>>>> 8eab5c344a23949b238ea79d2fbe7c0f136fb6a5

void syscall_entry (void);
void syscall_handler (struct intr_frame *);
void check_address(void *addr);
void halt(void);
void exit(int status);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int exec(const char *cmd_line);
int wait(int pid);
int open(const char *file_name);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
tid_t fork(const char *thread_name, struct intr_frame *f);

struct file *get_file_struct(int);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
<<<<<<< HEAD

=======
>>>>>>> 8eab5c344a23949b238ea79d2fbe7c0f136fb6a5
	lock_init(&filesys_lock);
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
<<<<<<< HEAD
	int sys_number = f->R.rax;
	int address = f->rsp;

	validate_address(address);

	// rdi -> rsi -> rdx -> r10 -> r8 -> r9
	/**
	 * 레지스터는 들어온 순서만을 의미한다. 
	*/
	switch (sys_number)	{
		case SYS_HALT:
			halt();
			break;
		case SYS_EXIT:
			exit(f->R.rdi);
			break;
		case SYS_CREATE:
			f->R.rax = create(f->R.rdi, f->R.rsi);
			break;
		case SYS_REMOVE:
			f->R.rax = remove(f->R.rdi);
			break;
		case SYS_OPEN:
			f->R.rax = open(f->R.rdi);
			break;
		case SYS_FILESIZE:
			f->R.rax = filesize(f->R.rdi);
			break;
		case SYS_READ:
			f->R.rax = read(f->R.rdi, f->R.rsi, f->R.rdx);
			break;
		case SYS_WRITE:
			f->R.rax = write(f->R.rdi, f->R.rsi, f->R.rdx);
			break;
		case SYS_SEEK:
			seek(f->R.rdi, f->R.rsi);
			break;
		case SYS_TELL:
			f->R.rax = tell(f->R.rdi);
			break;
		case SYS_CLOSE:
			close(f->R.rdi);
			break;
		default:
			break;
	}
	// printf("syscall Number: %d\n", sys_number);
	// printf ("system call!\n");
	// thread_exit ();
}

void
halt (void) {
	power_off();
}

void
exit (int status) {
	struct thread *current_thread = thread_current();	
	printf("%s: exit(%d)\n", current_thread->name, status);

	thread_exit();
}

bool
create(const char *file, unsigned initial_size) {
	validate_address(file);

	return filesys_create(file, initial_size);
}

bool 
remove(const char *file) {
	validate_address(file);

	return filesys_remove(file);
}

int
open(const char *file) {
	validate_address(file);

	struct file *opened_file = filesys_open(file);
	 
	// file이 생성되었는지 확인한다.
	if (opened_file == NULL){
		// printf("확인 필요\n");
		return -1;
	}

	// file을 fdt에 추가하고 index를 반환한다.
	int fd = add_file_to_fdt(opened_file);

	return fd;
}

int
add_file_to_fdt(struct file *file) {
	struct thread *c_thread = thread_current();	// 현재 current_thread를 가져온다.
	struct file **fdt = c_thread->fd_table;		// fdt를 가져온다.

	// 현재 thread의 f_index를 확인한다.
	int fd = c_thread->f_index;
	
	while (fdt[fd] != NULL && fd < MAX_FDT_SIZE)	{
		fd += 1;
	}
	
	// 범위 밖이면 -1을 반환한다. -> 다시 닫는다.
	if (fd == MAX_FDT_SIZE)	{
		return -1;
	}

	c_thread->f_index = fd;
	fdt[fd] = file;
	
	return fd;
}

int
filesize(int fd) {
	validate_fd(fd);

	struct thread *c_thread = thread_current();
	struct file **fdt = c_thread->fd_table;

	return file_length(fdt[fd]);
}

void
validate_fd(int fd){
	if (fd < 0 || fd >= MAX_FDT_SIZE) {
		exit(-1);
	}
}

int 
read (int fd, void *buffer, unsigned size) {
	validate_address(buffer);				// 버퍼 시작 주소 확인
	validate_address(buffer + size - 1);	// 버퍼 끝 주소 확인
	validate_fd(fd);

	unsigned char *buf = buffer;
	int read_byte = 0;

	// 파일을 읽는다.
	struct file *read_file = get_file_struct(fd);

	if (read_file == NULL) 
		return -1;
	
	// STDIN_FILENO일 때, read한다.
	if (fd == STDIN_FILENO) {
		char key;
		for (read_byte = 0; read_byte < size; read_byte++) {
			key = input_getc();
			*buf++ = key;
			if (key == '\0') {
				break;
			}
		}	
	}

	// STDOUT_FILENO일 때, -1을 반환한다.
	else if (fd == STDOUT_FILENO) {
		return -1;
	}

	// 그 외의 경우에는 file_read() 함수를 이용해서 읽어온다.
	else {
		lock_acquire(&filesys_lock);
		read_byte = file_read(read_file, buffer, size);
		lock_release(&filesys_lock);
	}
	return read_byte;
}

struct file
*get_file_struct(int fd) {
	struct thread *c_thread = thread_current();
	struct file **fdt = c_thread->fd_table;
	struct file *file = fdt[fd];

	return file;
}

/**
fd에 크기 바이트를 작성하고
실제로 작성된 byte를 반환한다. 
 * fd가 1이면 putbuf() 함수를 사용한다.
 * file_write 함수 사용
*/

int
write (int fd, const void *buffer, unsigned size) {
	validate_address(buffer);
	validate_address(buffer + size - 1);
	validate_fd(fd);

	int written_byte = 0;

	struct file *write_file = get_file_struct(fd);

	if (write_file == NULL)
		return -1;

	// STDOUT_FILENO일 때 -> putbuff 사용
	if (fd == STDOUT_FILENO) {
		putbuf(buffer, size);
	}
	else if (fd == STDIN_FILENO) {
		return -1;
	}
	else {
		lock_acquire(&filesys_lock);
		written_byte = file_write(write_file, buffer, size);
		lock_release(&filesys_lock);		
	}
	
	return written_byte;
}

void
seek (int fd, unsigned position) {
	validate_fd(fd);
	struct file *seek_file = get_file_struct(fd);
	if (seek_file == NULL) {
		return;
	}
	
	file_seek(seek_file, position);
}

unsigned 
tell (int fd) {
	validate_fd(fd);
	struct file *tell_file = get_file_struct(fd);
	if (tell_file == NULL) {
		return;
	}

	return file_tell(tell_file);
}

void
close (int fd) {
	validate_fd(fd);
	struct file *close_file = get_file_struct(fd);
	if (close_file == NULL) {
		return;
	}

	file_close(close_file);
	remove_fd(fd);
}

void 
remove_fd (int fd) {
	struct thread *current_thread = thread_current();
	current_thread->fd_table[fd] = NULL;
}

void
validate_address(void *addr){
	// VADDR이 user virtual address인가
	if (!is_user_vaddr(addr)) {
		exit(-1);
	}

	// 할당된 공간에 접근하였는가
	if (pml4_get_page(thread_current()->pml4, addr) == NULL) {
		exit(-1);
	}
=======
	int systemcall_num = f->R.rax;
	printf("%d\n", systemcall_num);

	switch (systemcall_num)
	{
	case SYS_HALT:
		halt();
		break;
	case SYS_EXIT:
		exit(f->R.rdi);
		break;
	case SYS_FORK:
		f->R.rax = fork(f->R.rdi, f);
		break;
	case SYS_EXEC:
		f->R.rax = exec(f->R.rdi);
		break;
	case SYS_WAIT:
		f->R.rax = wait(f->R.rdi);
		break;
	case SYS_CREATE:
		f->R.rax = create(f->R.rdi, f->R.rsi);
		break;
	case SYS_REMOVE:
		f->R.rax = remove(f->R.rdi);
		break;
	case SYS_OPEN:
		f->R.rax = open(f->R.rdi);
		break;
	case SYS_FILESIZE:
		f->R.rax = filesize(f->R.rdi);
		break;
	case SYS_READ:
		f->R.rax = read(f->R.rdi, f->R.rsi, f->R.rdx);
		break;
	case SYS_WRITE:
		f->R.rax = write(f->R.rdi, f->R.rsi, f->R.rdx);
		break;
	case SYS_SEEK:
		seek(f->R.rdi, f->R.rsi);
		break;
	case SYS_TELL:
		f->R.rax = tell(f->R.rdi);
		break;
	case SYS_CLOSE:
		close(f->R.rdi);
	}
}

void check_address(void *addr){
	if (addr == NULL)
		exit(-1);
	if (!is_user_vaddr(addr))
		exit(-1);
	if (pml4_get_page(thread_current()->pml4, addr) == NULL)
		exit(-1);
	// if (addr == NULL || !(is_user_vaddr(addr))||pml4_get_page(cur->pml4, addr) == NULL){
	// 	exit(-1);
	// }
}

// pintos를 종료시키는 시스템 콜
void halt(void){
	power_off(); // 핀토스를 종료시키는 함수
}

// 현재 프로세스를 종료시키는 시스템 콜
void exit(int status){
	struct thread *cur = thread_current();
	cur->exit_status = status;
	printf("%s: exit(%d)\n",thread_name(), status);
	thread_exit();
}

//파일을 생성하는 시스템 콜
bool create(const char *file, unsigned initial_size){
	check_address(file);
	return filesys_create(file,initial_size); // 파일 이름과 파일 사이즈를 인자 값으로 받아 파일을 생성하는 함수
}

//파일을 삭제하는 시스템 콜
bool remove(const char *file){
	check_address(file);
	return filesys_remove(file); // 파일 이름에 해당하는 파일을 제거하는 함수
}

int exec(const char *cmd_line)
{
	check_address(cmd_line);	

	// 새 스레드를 생성하지 않고 process_exec을 호출한다.

	char *cmd_line_copy;	// filename으로 const char* 복사본을 만든다. 
	cmd_line_copy = palloc_get_page(0);
	if (cmd_line_copy == NULL) // 메모리 할당 실패시 exit(-1)
		exit(-1);
	strlcpy(cmd_line_copy, cmd_line, PGSIZE); // cmd_line을 복사한다.

	if (process_exec(cmd_line_copy) == -1)
		exit(-1); // 실패 시 status -1로 종료한다.
}

int wait(int pid)
{
	return process_wait(pid);
}

// 파일을 열 때 사용하는 시스템 콜
int open(const char *file_name)
{
	check_address(file_name);
	struct file *file = filesys_open(file_name);
	if (file == NULL)
		return -1;
	int fd = process_add_file(file);
	if (fd == -1) // 여기에 땀 있었음.(닦아줌)
		file_close(file);
	return fd;
}

// 파일의 크기를 알려주는 시스템 콜
int filesize(int fd)
{
	struct file *file = process_get_file(fd);
	if (file == NULL)
		return -1;
	return file_length(file);
}

// 열린 파일의 데이터를 읽는 시스템 콜
int read(int fd, void *buffer, unsigned size)
{
	check_address(buffer);

	char *ptr = (char *)buffer;
	int bytes_read = 0;

	lock_acquire(&filesys_lock); // 락을 요구합니다.
	if(fd == STDIN_FILENO)
	{
		for (int i = 0; i < size; i++)
		{
			*ptr++ = input_getc();
			bytes_read++;
		}
		lock_release(&filesys_lock);
	}
	else
	{
		if (fd < 2)
		{
			lock_release(&filesys_lock);
			return -1;
		}
		struct file *file = process_get_file(fd);
		if (file == NULL)
		{
			lock_release(&filesys_lock);
			return -1;
		}
		bytes_read = file_read(file, buffer, size);
		lock_release(&filesys_lock);
	}
	return bytes_read;
}

// 열린 파일의 데이터를 기록 시스템 콜
int write(int fd, const void *buffer, unsigned size)
{
	check_address(buffer);
	int bytes_write = 0;
	if (fd == STDOUT_FILENO)
	{
		putbuf(buffer, size);
		bytes_write = size;
	}
	else
	{
		if (fd < 2)
			return -1;
		struct file *file = process_get_file(fd);
		if (file == NULL)
			return -1;
		lock_acquire(&filesys_lock);
		bytes_write = file_write(file, buffer, size);
		lock_release(&filesys_lock);
	}
	return bytes_write;
}


// 열린 파일의 위치를 이동하는 시스템 콜
void seek(int fd, unsigned position)
{
	struct file *file = process_get_file(fd);
	if (file == NULL)
		return;
	file_seek(file, position);
}

// 열린 파일의 위치를 알려주는 시스템 콜
unsigned tell(int fd)
{
	struct file *file = process_get_file(fd);
	if (file == NULL)
		return;
	return file_tell(file);
}

// 열린 파일을 닫는 시스템 콜
void close(int fd)
{
	struct file *file = process_get_file(fd);
	if (file == NULL)
		return;
	file_close(file);
	process_close_file(fd);
}


tid_t fork(const char *thread_name, struct intr_frame *f)
{
	return process_fork(thread_name, f);
>>>>>>> 8eab5c344a23949b238ea79d2fbe7c0f136fb6a5
}