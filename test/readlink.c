/***
readlink.c

readlink()会将参数path的符号链接内容存储到参数buf所指的内存空间，返回的内容不是以\000作字符串结尾，但会将字符串的字符数返回，
这使得添加\000变得简单。若参数bufsiz小于符号连接的内容长度，过长的内容会被截断，
如果 readlink 第一个参数指向一个文件而不是符号链接时，readlink 设 置errno 为 EINVAL 并返回 -1。
readlink()函数组合了open()、read()和close()的所有操作。

path是一个存在的软连接。

***/
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
int main()
{
    int i;
    char *p;
    char path[1024];
    /*返回当前程序执行的决定路径*/
    int result = readlink("/proc/self/exe",path,1023);
 
    if (result < 0 || (result >= 1023))
    {
        perror("readlink ");
        return 0;
    }

    path[result] = '\0';
    printf("%s\r\n",path);
    /* strip filename */
    if ((p = strrchr (path, '/')) == 0)
    {
       return 0;
    }
    *p = 0;

    printf("%s\r\n",path);
    /* strip bin/ */
    if ((p = strrchr (path, '/')) == 0)
    {
       return 0;
    }
    *p = 0;
    printf("%s\r\n",path);

    return 0;
}
