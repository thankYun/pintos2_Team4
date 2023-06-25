#include "filesys/file.h"
#include <debug.h>
#include "filesys/inode.h"
#include "threads/malloc.h"

/* An open file. */
struct file {
	struct inode *inode;        /* File's inode.파일의 inode (인덱스 노드) */
	off_t pos;                  /* Current position.현재 위치 */
	bool deny_write;            /* Has file_deny_write() been called?file_deny_write()가 호출되었는지 여부  */
};

/* Opens a file for the given INODE, of which it takes ownership,
and returns the new file.  Returns a null pointer if an
allocation fails or if INODE is null. 

주어진 INODE에 대한 파일을 열고 새로운 파일을 반환합니다.
할당에 실패하거나 INODE가 null인 경우 null 포인터를 반환합니다. */
struct file *
file_open (struct inode *inode) {
	struct file *file = calloc (1, sizeof *file);
	if (inode != NULL && file != NULL) {
		file->inode = inode;
		file->pos = 0;
		file->deny_write = false;
		return file;
	} else {
		inode_close (inode);
		free (file);
		return NULL;
	}
}

/* Opens and returns a new file for the same inode as FILE.
 Returns a null pointer if unsuccessful. 
 FILE과 동일한 inode에 대한 새로운 파일을 열고 반환합니다.
실패한 경우 null 포인터를 반환합니다. */
struct file *
file_reopen (struct file *file) {
	return file_open (inode_reopen (file->inode));
}

/* Duplicate the file object including attributes and returns a new file for the
same inode as FILE. Returns a null pointer if unsuccessful. 
파일 객체를 복제하고 속성을 포함한 새로운 파일을
동일한 inode에 대해 반환합니다. 실패한 경우 null 포인터를 반환합니다.*/
struct file *
file_duplicate (struct file *file) {
	struct file *nfile = file_open (inode_reopen (file->inode));
	if (nfile) {
		nfile->pos = file->pos;
		if (file->deny_write)
			file_deny_write (nfile);
	}
	return nfile;
}

/* Closes FILE.  FILE을 닫습니다.*/
void
file_close (struct file *file) {
	if (file != NULL) {
		file_allow_write (file);
		inode_close (file->inode);
		free (file);
	}
}

/* Returns the inode encapsulated by FILE. FILE에 의해 캡슐화된 inode를 반환합니다. */
struct inode *
file_get_inode (struct file *file) {
	return file->inode;
}

/* Reads SIZE bytes from FILE into BUFFER,
starting at the file's current position.
Returns the number of bytes actually read,
which may be less than SIZE if end of file is reached.
Advances FILE's position by the number of bytes read.
SIZE 바이트를 FILE에서 현재 위치부터 BUFFER로 읽어옵니다.
실제로 읽은 바이트 수를 반환하며, 파일의 끝에 도달하면 SIZE보다 작을 수 있습니다.
파일의 위치는 읽은 바이트 수만큼 진행됩니다 */
off_t
file_read (struct file *file, void *buffer, off_t size) {
	off_t bytes_read = inode_read_at (file->inode, buffer, size, file->pos);
	file->pos += bytes_read;
	return bytes_read;
}

/* Reads SIZE bytes from FILE into BUFFER,
 starting at offset FILE_OFS in the file.
 Returns the number of bytes actually read,
 which may be less than SIZE if end of file is reached.
 The file's current position is unaffected. 
SIZE 바이트를 FILE에서 파일의 오프셋 FILE_OFS부터 BUFFER로 읽어옵니다.
실제로 읽은 바이트 수를 반환하며, 파일의 끝에 도달하면 SIZE보다 작을 수 있습니다.
파일의 현재 위치는 변경되지 않습니다. 
 */
off_t
file_read_at (struct file *file, void *buffer, off_t size, off_t file_ofs) {
	return inode_read_at (file->inode, buffer, size, file_ofs);
}

/* Writes SIZE bytes from BUFFER into FILE,
starting at the file's current position.
Returns the number of bytes actually written,
which may be less than SIZE if end of file is reached.
(Normally we'd grow the file in that case, but file growth is
not yet implemented.)
Advances FILE's position by the number of bytes read.
BUFFER의 SIZE 바이트를 FILE에 파일의 현재 위치부터 씁니다.
실제로 쓴 바이트 수를 반환하며, 파일의 끝에 도달하면 SIZE보다 작을 수 있습니다.
(일반적으로 파일을 확장하지만 파일 확장은 아직 구현되지 않음)
파일의 위치는 쓴 바이트 수만큼 진행됩니다 */
off_t
file_write (struct file *file, const void *buffer, off_t size) {
	off_t bytes_written = inode_write_at (file->inode, buffer, size, file->pos);
	file->pos += bytes_written;
	return bytes_written;
}

/* Writes SIZE bytes from BUFFER into FILE,
starting at offset FILE_OFS in the file.
Returns the number of bytes actually written,
which may be less than SIZE if end of file is reached.
(Normally we'd grow the file in that case, but file growth is
not yet implemented.)
The file's current position is unaffected.
BUFFER의 SIZE 바이트를 FILE에 파일의 오프셋 FILE_OFS부터 씁니다.
실제로 쓴 바이트 수를 반환하며, 파일의 끝에 도달하면 SIZE보다 작을 수 있습니다.
(일반적으로 파일을 확장하지만 파일 확장은 아직 구현되지 않음)
파일의 현재 위치는 변경되지 않습니다. */
off_t
file_write_at (struct file *file, const void *buffer, off_t size,
		off_t file_ofs) {
	return inode_write_at (file->inode, buffer, size, file_ofs);
}

/* Prevents write operations on FILE's underlying inode
  until file_allow_write() is called or FILE is closed.
  파일의 기반이 되는 inode에 대해 파일에 대한 쓰기 작업을
file_allow_write()가 호출되거나 파일이 닫힐 때까지 방지합니다. */
void
file_deny_write (struct file *file) {
	ASSERT (file != NULL);
	if (!file->deny_write) {
		file->deny_write = true;
		inode_deny_write (file->inode);
	}
}

/* Re-enables write operations on FILE's underlying inode.
(Writes might still be denied by some other file that has the
same inode open.) 
FILE의 기반이 되는 inode에 대한 쓰기 작업을 다시 허용합니다.
(다른 파일에서 동일한 inode을 열어서 여전히 쓰기가 거부될 수 있음)*/
void
file_allow_write (struct file *file) {
	ASSERT (file != NULL);
	if (file->deny_write) {
		file->deny_write = false;
		inode_allow_write (file->inode);
	}
}

/* Returns the size of FILE in bytes.FILE의 크기를 바이트 단위로 반환합니다. */
off_t
file_length (struct file *file) {
	ASSERT (file != NULL);
	return inode_length (file->inode);
}

/* Sets the current position in FILE to NEW_POS bytes from the
 start of the file. 파일의 현재 위치를 파일의 시작으로부터 NEW_POS 바이트로 설정합니다.*/
void
file_seek (struct file *file, off_t new_pos) {
	ASSERT (file != NULL);
	ASSERT (new_pos >= 0);
	file->pos = new_pos;
}

/* Returns the current position in FILE as a byte offset from thestart of the file. 
 파일의 현재 위치를 파일의 시작으로부터의 바이트 오프셋으로 반환합니다.*/
off_t
file_tell (struct file *file) {
	ASSERT (file != NULL);
	return file->pos;
}
