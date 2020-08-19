#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUF 1048576

char buf[BUF + 100];

void swap(char s[],int a, int b){
    char t = s[a];
    s[a] = s[b];
    s[b] = t;
}

void reverse(char *s){
    int n = strlen(s);
    for (int i = 0; i < n / 2; i++)
        swap(s,i,n-i-1);
}

int main(int a, char** argv){

    int fd1, fd2;
    char dest_path[1000], tmp[8];
    long long filesize, count=0, offset;
    
    struct stat stats;
    

    mkdir("Assignment", 0700);
    fd1 = open(argv[1], O_RDONLY);
    strcpy(dest_path, "./Assignment/");
    strcat(dest_path, argv[1]);
    fd2 = open(dest_path, O_WRONLY | O_CREAT, 0600);
    
    


    if (fd1 < 0 || fd2 < 0){
        perror("r1");
        exit(1);
    }

    if (!stat(argv[1], &stats))
        filesize = (long long)stats.st_size;
    
    offset = lseek(fd1, -BUF, SEEK_END);

    while (offset >= 0){
        count = count + read(fd1, buf, BUF);
        buf[BUF] = '\0';
        reverse(buf);
        write(fd2, buf, BUF);
        
        offset = lseek(fd1, -BUF * 2, SEEK_CUR);
        
        write(1, "Progress: ", sizeof("Progress: "));
        float p = (float)100 * (float)count / (float)(filesize - 1);
        snprintf(tmp, sizeof(tmp), "%.2f%%\r", p);
        write(1, tmp, sizeof(tmp));

    }
    
    if (0 > offset){
        lseek(fd1, 0, SEEK_SET);
        read(fd1, buf, filesize - count);
        buf[filesize - count] = '\0';
        reverse(buf);
        write(fd2, buf, filesize - count);
    }

    close(fd2);
    close(fd1);
    return 0;
}
