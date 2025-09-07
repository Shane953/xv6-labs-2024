#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"


// Write a simple version of the UNIX find program for xv6: find all the files in a directory tree with a specific name.
// Look at user/ls.c to see how to read directories.
// Use recursion to allow find to descend into sub-directories.
// Don't recurse into "." and "..".
// Changes to the file system persist across runs of qemu; to get a clean file system run make clean and then make qemu.
// You'll need to use C strings. Have a look at K&R (the C book), for example Section 5.5.
// Note that == does not compare strings like in Python. Use strcmp() instead.
// Add the program to UPROGS in Makefile.
// init: starting sh
// $ echo > b
// $ mkdir a
// $ echo > a/b
// $ mkdir a/aa
// $ echo > a/aa/b
// $ find . b
// ./b
// ./a/b
// ./a/aa/b

void
find(char const*path,  char const*filename) {
    int fd;
    struct dirent de;
    struct stat st;
    // p 一个临时目录指针，用来在 path 的基础上，构建出下一层所有文件/目录的完整路径，以便传给 stat 或进行递归调用
    char buf[512], *p;

    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    // 通过文件描述符获取该路径的元数据（类型、大小等），存入 st 结构体
    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    if(st.type != T_DIR){
        // 修改：初始路径如果不是目录，直接报错。
        fprintf(2, "Usage: find <directory> <filename>\n");
        close(fd);
        return;
    }

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("find: path too long\n");
        return;
    }
    // 将原始路径 path 拷贝到 buf 中
    strcpy(buf, path);
    // 从 buf 的起始地址开始，向后移动 strlen(buf) 个格子的距离，然后把这个新位置的地址存到指针 p 里面
    //  buf 中路径的末尾加上一个 /，并将指针 p 移动到 / 之后
    p = buf+strlen(buf);
    *p++ = '/';
    //遍历目录fd, 读出一个个 struct dirent（目录项）结构体
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        // inum（inode 编号）为 0 的目录项是无效或已删除的
        if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;
        //从de.name地址 移动或拷贝 DIRSIZ 个字节到目标 p 内存地址，buf 已经变成目前的路径
        memmove(p, de.name, DIRSIZ);
        // DIRSIZ固定大小，\0字符串的结束标记
        p[DIRSIZ] = 0;
        // 获取任何类型的文件系统条目（普通文件、目录、设备文件等）的元数据
        if(stat(buf, &st) < 0){
            printf("find: cannot stat %s\n", buf);
            continue;
        }
        switch(st.type){
            case T_FILE:
                // 如果是文件，比较名字
                if(strcmp(de.name, filename) == 0){
                    printf("%s\n", buf);
                }
                break;

            case T_DIR:
                // 如果是目录，递归进去
                find(buf, filename);
                break;
        }
    }
    close(fd);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(2, "Usage: find files...\n");
        exit(1);
    }
    const char* path = argv[1];
    const char* filename = argv[2];
    find(path, filename);
    exit(0);
}

