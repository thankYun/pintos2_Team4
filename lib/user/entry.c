#include <syscall.h>

int main (int, char *[]);
void _start (int argc, char *argv[]);

// user program의 진입점이다.
void
_start (int argc, char *argv[]) {
	exit (main (argc, argv));
}
