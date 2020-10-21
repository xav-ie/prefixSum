#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/errno.h>

int isMapped(const void *ptr, int bytes) {
        if(ptr == NULL) return 0;
        // create a pipe that goes nowhere
        int fd[2];
        int valid = 1;
        // creates a new pipe
        pipe(fd);
        if(write(fd[1], ptr, bytes) < 0) {
                if(errno == EFAULT) valid = 0;
        }
        close(fd[0]);
        close(fd[1]);
        return valid;
}

void testptr(void *ptr, int bytes, char *name) {
        printf("%s:\t%d\t%p\n", name,isMapped(ptr, bytes),ptr);
}

int main() {
        int *junk = NULL;
        int *junk2 = (int*)((uintptr_t)0xfeedbeef);
        int *p = malloc(50);
        int x = 5;
        int *px = &x;
        
        testptr(junk, 1, "junk");
        testptr(junk2, 1, "junk2");
        testptr(p, 50, "p");
        testptr(p, 51, "notp");
        testptr(&x, 1, "&x");
        testptr(px, 1, "px");

}
