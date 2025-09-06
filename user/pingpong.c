#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//Write a user-level program that uses xv6 system calls to ''ping-pong'' a byte between two processes
//over a pair of pipes, one for each direction. The parent should send a byte to the child;
//the child should print "<pid>: received ping", where <pid> is its process ID,
//write the byte on the pipe to the parent, and exit; the parent should read the byte from the child,
//print "<pid>: received pong", and exit

int main(int argc, char *argv[]) {
    // p[0] 是读端，p[1] 是写端
    int p[2];
    int n;
    char buf[5];
    pipe(p);
    if (fork() != 0) {  // parent
        write(p[1], "ping", 4);
        close(p[1]);
        wait(0);
        n = read(p[0], buf, sizeof(buf) - 1);
        if (n >= 0) {
            buf[n] = '\0'; // 在实际读取到的数据末尾，手动添加结束符
        }
        printf("%d: received %s\n",getpid(), buf);
        close(p[0]);
        exit(0);
    }else{
        n = read(p[0], buf, sizeof(buf) - 1);
        if (n >= 0) {
            buf[n] = '\0';
        }
        close(p[0]);
        printf("%d: received %s\n",getpid(), buf);
        write(p[1], "pong", 4);
        close(p[1]);
        exit(0);
    }
}