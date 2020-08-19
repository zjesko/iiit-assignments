#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUF 1048576

char *yes = "Yes\n\0";
char *no = "No\n\0";

void printp(char path[], char f[]){
    struct stat st;
    char line[1000];

    if(stat(path, &st) == 0){
        strcpy(line, "User has read permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IRUSR) ? yes : no);
        write(1, line, strlen(line));

        strcpy(line, "User has write permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IWUSR) ? yes : no);
        write(1, line, strlen(line));

        strcpy(line, "User has execute permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IXUSR) ? yes : no);
        write(1, line, strlen(line));

        strcpy(line, "Group has read permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IRGRP) ? yes : no);
        write(1, line, strlen(line));

        strcpy(line, "Group has write permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IWGRP) ? yes : no);
        write(1, line, strlen(line));

        strcpy(line, "Group has execute permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IXGRP) ? yes : no);
        write(1, line, strlen(line));

        strcpy(line, "Others has read permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IROTH) ? yes : no);
        write(1, line, strlen(line));

        strcpy(line, "Others has write permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IWOTH) ? yes : no);
        write(1, line, strlen(line));

        strcpy(line, "Others has execute permissions on ");
        strcat(line, f);
        strcat(line, (st.st_mode & S_IXOTH) ? yes : no);
        write(1, line, strlen(line));

    }
    else{
        char er[1000];
        strcpy(er, "Please check the path for ");
        strcat(er, f);
        er[strlen(er)-2] = '\n';
        write(1, er, strlen(er)-1);
        perror("r1");
    }
    write(1,"\n",1);
}

char buf[BUF + 100];
char buf2[BUF + 100];

void reverse(char *str){
    int n = strlen(str);
    for (int i = 0; i < n / 2; i++)
    {
        char t = str[i];
        str[i] = str[n - i - 1];
        str[n - i - 1] = t;
    }
}

char tmp[8];

int check(char* newfile, char* oldfile){
    int fd2 = open(newfile, O_RDONLY);
    int fd1 = open(oldfile, O_RDONLY);

    if(fd1 < 0 || fd2 < 0)
        return 0;

    int flag=1;

    long long filesize, count=0, offset;
    struct stat stats;
    if (!stat(oldfile, &stats))
        filesize = (long long)stats.st_size;

    offset = lseek(fd1, -BUF, SEEK_END);

    while (offset >= 0){
        count += read(fd1, buf, BUF);
        read(fd2, buf2, BUF);
        buf[BUF] = '\0';
        buf2[BUF] = '\0';

        reverse(buf);
        if(strcmp(buf,buf2)!=0)
            flag = 0;
        
        offset = lseek(fd1, -BUF * 2, SEEK_CUR);
    }

    if (offset < 0){
        lseek(fd1, 0, SEEK_SET);
        read(fd1, buf, filesize - count);
        read(fd2, buf2, filesize - count);
        buf[filesize - count] = '\0';
        buf2[filesize - count] = '\0';

        reverse(buf);
         if(strcmp(buf,buf2)!=0)
            flag = 0;
    }
    close(fd1);
    close(fd2);
    return flag;
}

int main(int a, char** argv){

    struct stat s;
    char l[100];
    strcpy(l, "Directory is created: ");
    strcat(l, (stat(argv[3], &s) == 0) ? yes : no);
    write(1, l, strlen(l));

    strcpy(l, "Whether file contents are reversed in newfile: ");
    strcat(l, (check(argv[1],argv[2])) ? yes : no);
    write(1, l, strlen(l));
    write(1,"\n",1);

    printp(argv[1], "newfile: ");
    printp(argv[2], "oldfile: ");
    printp(argv[3], "directory: ");
    return 0;
}
