#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/param.h"



// 定义一个静态缓冲区和相关指针
// static 变量只会被初始化一次，在多次调用 readline 之间保持其值
static char read_buf[512];
static int buf_pos = 0; // 当前在缓冲区read_buf中处理到的位置
static int buf_len = 0; // 缓冲区中有效数据的长度

// 高性能的 readline 函数
int readline(char* line_buf, int max_len)
{
    int i = 0;
    // 在用户态缓冲区 (read_buf) 中逐个字符地查找换行符
    for (;; i++) {
        // 如果缓冲区处理完了，就从内核读取一大块新的数据，
        if (buf_pos >= buf_len) {
            buf_pos = 0;
            // buf_len 本次 read 系统调用实际读取了多少字节
            buf_len = read(0, read_buf, sizeof(read_buf));
            // 如果读不到任何数据, EOF (返回 0) 和错误 (返回 -1)
            if (buf_len <= 0) {
                line_buf[i] = 0;
                return 0;
            }
        }

        // 从 read_buf 中逐个字符地取出数据 ，并复制到 line_buf 输出缓冲区
        char c = read_buf[buf_pos++];

        if (i + 1 >= max_len) {
            fprintf(2, "xargs: argument line too long\n");
            exit(1);
        }

        // 对于 xargs 来说，它期望的标准输入就是以换行符 \n 作为行分隔符的文本数据。
        // 单行输入：echo hello too | xargs ...，xargs 收到的就是 hello too\n。
        // 多行输入：(echo 1 ; echo 2) | xargs ...，xargs 收到的就是 1\n2\n
        if (c == '\n') {
            break;
        }
        // 复制read_buf的内容到line_buf
        line_buf[i] = c;
    }
    line_buf[i] = 0; // 添加字符串结束符
    return 1; // 返回 1 表示成功读取一行
}

int main(int argc, char** argv) {
    // printf("xargs started with %d argument(s):\n", argc);
    // for (int i = 0; i < argc; i++) {
    //     printf("argv[%d] = %s\n", i, argv[i]);
    // }

    if (argc < 2) {
        fprintf(2, "Usage: xargs command (arg ...)\n");
        exit(1);
    }

    int base_argc = argc - 1; // command + args
    char *new_argv[MAXARG];
    // 直接让 new_argv 的指针指向 argv 里的字符串地址
    for (int i = 1; i < argc; i++) {
        new_argv[i - 1] = argv[i];
    }
    char* command = argv[1];
    char line_buffer[1024];
    while (readline(line_buffer, sizeof(line_buffer))){
        new_argv[base_argc] = line_buffer;
        // 用 NULL 结束 new_argv 数组
        new_argv[argc] = 0;
        // for (int i = 0; i < argc; i++) {
        //     printf("new_argv[%d] = %s\n", i, new_argv[i]);
        // }
        if(fork() == 0) {
            exec(command, new_argv);
            fprintf(2, "exec %s failed\n", command);
            exit(1);
        }else{
            wait(0);
        }
    }
    exit(0);
}

