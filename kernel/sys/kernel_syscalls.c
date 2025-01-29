#include <stdlib.h>
#include <sys/errno.h>

// Terminate process (not properly implemented in kernel space)
void _exit(int status) {
    // If you have task management:
    // task_exit(status);
    
    // If no multitasking yet, just halt
    for (;;) {
        __asm__ volatile ("hlt");
    }
    __builtin_unreachable();
}

// Close file descriptor (stub)
int close(int fd) {
    // Basic error checking - no real file system yet
    if (fd < 0 || fd >= MAX_FDS) {  // Define MAX_FDS if needed
        errno = EBADF;
        return -1;
    }
    
    // Add actual device/file closing logic here when implemented
    return 0;  // Pretend success for now
}

// Delete file (stub)
int unlink(const char *pathname) {
    // No filesystem implementation yet
    errno = ENOENT;  // No such file or directory
    return -1;
}
