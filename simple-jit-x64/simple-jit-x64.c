#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

const size_t SIZE = 1024;
typedef long (*JittedFunc)(long);

void* alloc_writable_memory(size_t size);
void emit_code_into_memory(unsigned char* m);
int make_memory_executable(void* m, size_t size);
void emit_to_rw_run_from_rx();

int main()
{
        emit_to_rw_run_from_rx();
        return EXIT_SUCCESS;
}

// Allocate Read | Writable (RW) memory of given size and returns a pointer to it. 
// On failure, print out the error and returns NULL. mmap is used to allocate, so
// deallocation has to be done with munmap, and the memory is allocated
// on a page boundary so it's suitable for calling mprotect.
void* alloc_writable_memory(size_t size) 
{
        void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
                perror("mmap");
                return NULL;
        }
        
        return ptr;
}

// Emit code into the given memory, assuming enough was allocated. 
void emit_code_into_memory(unsigned char* m) 
{
        // assembly code of function f(x): return x + 4
        unsigned char code[] = {
                0x48, 0x89, 0xf8,                   // mov %rdi, %rax
                0x48, 0x83, 0xc0, 0x04,             // add $4, %rax
                0xc3                                // ret
        };
        
        memcpy(m, code, sizeof(code));
}

// Set a Read | Exec (RX) permission on the given memory, which must be page-aligned. 
// Return 0 on success. On failure, print out the error and returns -1.
int make_memory_executable(void* m, size_t size) 
{
        if (mprotect(m, size, PROT_READ | PROT_EXEC) == -1) {
                perror("mprotect");
                return -1;
        }
        
        return 0;
}

// Allocate Read | Writable (RW) memory
// Emit the code into the memory
// Set the memory to Read | Exec (RX)
// Excute the code
void emit_to_rw_run_from_rx() 
{       
        // Allocate Read | Writable memory
        void* m = alloc_writable_memory(SIZE);
        if (!m) {
                return;
        }

        // Emit the code into the memory
        emit_code_into_memory((unsigned char*)m);
        
        // Set the memory to Read | Exec
        if (make_memory_executable(m, SIZE) == -1) {
                goto out; 
        }
        
        // Excute the code
        JittedFunc func = m;
        int result = func(2);
        printf("result = %d\n", result);

out:
        // Free the memory
        munmap(m, SIZE);
}