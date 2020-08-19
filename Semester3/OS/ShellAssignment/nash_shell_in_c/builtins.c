#include "nash.h"

int exit_nash(int n, char **args){
    printf("Exiting Nash, Goodbye\n");
    exit(1);
    return 1;
}

int pwd_nash(int n, char **args){
    printf("%s\n", pwd);
    return 1;
}

int echo_nash(int n, char **args){
    for(int i=1; i<n; i++)
        printf("%s", tokens[i]);
    printf("\n");
    return 1;
}

int cd_nash(int n, char **args){

    char cdpath[BUF_PWD];

    for(int i=0;i<n;i++){
        //   printf("%s\n", args[i]);
    } 
    if(args[1][0] == '~'){
        strcpy(cdpath, home);
        strcat(cdpath, args[1]+1);
    }
    else{
        strcpy(cdpath, args[1]);
    }
    if(chdir(cdpath) < 0)
        perror("cd Error");

    return 1;
}

int clear_nash(int n, char **args){
    printf("\033[H\033[J");
    return 1;
}


int ls_nash(int n, char **args){

    struct dirent *dp;
    DIR *dir;
    char items[BUF_PWD][BUF_PWD];
    int no_items = 0;

    if(n > 1)
        cd_nash(n, args);

    dir = opendir(".");

    while((dp = readdir(dir)) != NULL){
        if((dp -> d_name[0] == '.' && flag_hash['a']) || dp -> d_name[0] != '.'){
            strcpy(items[no_items], dp -> d_name);
            no_items++;
        }
    }

    for(int i=0;i<no_items;i++){
        if(flag_hash['l']){

            struct stat fileStat;
            if(stat(items[i], &fileStat) < 0)
                continue;
            struct passwd *pws;
            pws = getpwuid(fileStat.st_uid);
            struct group *grp;
            grp = getgrgid(fileStat.st_gid);

            printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf("%2ld", fileStat.st_nlink);
            printf("%9s", pws->pw_name);
            printf("%9s", grp->gr_name);
            printf("%6ld ", fileStat.st_size);
            printf("%.12s", ctime(&fileStat.st_mtime) + 4);
            printf(" %s", items[i]);
            printf("\n");

        }
        else
            printf("%s\n", items[i]);
    }
    chdir(pwd);
    return 1;
}

int pinfo_nash(int n, char **args){


    char pinfo_path[BUF_PWD];
    char p_path[BUF_PWD];

    if(n > 1)
        sprintf(p_path, "/proc/%s/", args[1]);  
    else
        strcpy(p_path, "/proc/self/");

    strcpy(pinfo_path, p_path);
    strcat(pinfo_path, "stat");

    FILE *stat = fopen(pinfo_path, "r");
    if(stat == NULL){
        perror("statfile Error:");
        return 0;
    }

    int pid, mem = 0;
    char status, expath[BUF_PWD], pname[BUF_PWD];

    fscanf(stat, "%d %s %c", &pid, pname, &status);
    fclose(stat);

    strcat(pinfo_path, "m");

    FILE *statm = fopen(pinfo_path, "r");
    if(statm == NULL){
        perror("statfile Error:");
        return 0;
    }
    fscanf(statm, "%d", &mem);
    fclose(statm);

    strcpy(pinfo_path, p_path);
    strcat(pinfo_path, "exe"); 

    readlink(pinfo_path, expath, sizeof(expath));
    torelative(expath);

    printf("pid -> %d\n", pid);
    printf("Status -> %c\n", status);
    printf("Memory -> %d\n", mem);
    printf("Executable Path -> %s\n", expath);
	//random comment to make hacktoberfest pull request
    return 1;
}

int nightswatch_nash(int n, char **args){

    int time = atoi(args[1]);
    fd_set input_set;
    struct timeval timeout;

    /* Empty the FD Set */
    FD_ZERO(&input_set);
    /* Listen to the input descriptor */
    FD_SET(STDIN_FILENO, &input_set);

    if (!strcmp(args[2],"interrupt")){
        // Interupt
        printf("0\tCPU0\tCPU1\tCPU2\tCPU3\n");
        int k = 1;

        do {
            FILE *interrupt = fopen("/proc/interrupts", "r");       
            ssize_t reads;
            size_t len = 0;
            char * line = NULL;

            if (interrupt == NULL){
                perror("Error opening interrupt file: ");
                return 0;
            }

            int i = 0;

            while(i < 3 && (reads = getline(&line, &len, interrupt)) != -1) {
                i++;
            }
            long long int cpu0, cpu1, cpu2, cpu3;
            // printf("%s\n", line);

            sscanf(line, "%*lld: %lld %lld %lld %lld", &cpu0, &cpu1, &cpu2, &cpu3); 

            printf("%d\t%lld\t%lld\t%lld\t%lld\n", k, cpu0, cpu1, cpu2, cpu3);
            k++;

            fclose(interrupt);

            timeout.tv_sec = time;    // time seconds
            timeout.tv_usec = 0;    // 0 milliseconds
            select(1, &input_set, NULL, NULL, &timeout);
        }
        while(1);

        return 0;
    }
    if (!strcmp(args[2],"dirty")){
        // dirty

        do {
            FILE *meminfo = fopen("/proc/meminfo", "r");       
            ssize_t reads;
            size_t len = 0;
            char * line = NULL;

            if (meminfo == NULL){
                perror("Error opening meminfo file: ");
                return 0;
            }

            int i = 0;

            while(i < 17 && (reads = getline(&line, &len, meminfo)) != -1) {
                i++;
            }
            printf("%s", line);

            fclose(meminfo);

            timeout.tv_sec = time;    // time seconds
            timeout.tv_usec = 0;    // 0 milliseconds
            select(1, &input_set, NULL, NULL, &timeout);
        }
        while(1);
        return 0;
    }

    return 1;
}


int history_nash(int k, char** args){

    char hist_path[BUF_PWD];
    strcpy(hist_path, home);
    strcat(hist_path, "/history.txt");

    char* l[BUF_COM];
    char c[BUF_COM];
    int n=0;
    FILE *f = fopen(hist_path, "r");
    fgets(c,BUF_COM, f);
    l[0] = strtok(c, ",");

    while(l[n] != NULL)
        l[++n] = strtok(NULL, ",");
    fclose(f);

    int t;
    if(k==1)
        t=10;
    else
        t=atoi(args[1]);

    for(int i=n-t;i<n; i++){
        printf("%s\n", l[i]);
    }

    return 1;
}

int setenv_nash(int n, char **args){

    if(n!=2 && n!=3){
        printf("setenv: Incorrect number of arguments\n");
        return -1;
    }

    char var[BUF_ENV], val[BUF_ENV];
    strcpy(var, args[1]);
    strcpy(val, "");

    if(n == 3)
        strcpy(val, args[2]);

    setenv(var, val, 1);
    return 1;
}

int unsetenv_nash(int n, char **args){

    if(n!=2){
        printf("unsetenv: Incorrect number of arguments\n");
        return -1;
    }

    char var[BUF_ENV];
    strcpy(var, args[1]);

    unsetenv(var);
    return 1;
}

int jobs_nash(int n, char **args){
    
    struct Job* i = head;
    int count = 0;
    while(i){
         
        count++;
        char procpath[BUF_PWD];
        sprintf(procpath, "/proc/%d/stat", i->pid);
    
        FILE *stat = fopen(procpath, "r");
        if(stat == NULL){
            printf("Infomation not available for PID: %d\n", i->pid);
            i = i->next;
            continue;
        }
        
        int pid;
        char s, pname[BUF_PWD];

        fscanf(stat, "%d %s %c", &pid, pname, &s);
        fclose(stat);
        
        char status[8]; 
        if(s == 'S' || s == 'R')
            strcpy(status, "Running");
        else if(s == 'T')
            strcpy(status, "Stopped");
        else
            strcpy(status, "UNKNOWN");
        
        printf("[%d] %s %s [%d]\n", count, status, i->command, i->pid);
        i = i->next;
    }
    return 1;
}

int kjob_nash(int n, char **args){
    
    if(n != 3){
        printf("kjob: Incorrect number of arguments\n");
        return 0;
    }
    int jobno = atoi(args[1]);
    int sig = atoi(args[2]);
    struct Job* temp = nth_node(jobno);
    if(temp == NULL){
        printf("kjob: No such job exists\n");
        return 1;
    }
    if(kill(temp->pid, sig) == -1){
        perror("");
        return 0;
    }
    return 1;
}
    
int fg_nash(int n,char **args){
    
    if(n != 2){
        printf("fg: Incorrect number of arguments\n");
        return 0;
    }
    int jobno = atoi(args[1]);
    struct Job* temp = nth_node(jobno);
    if(jobno > no_jobs){
        printf("fg: No such job exists\n");
        return 1;
    }
    signal(SIGCHLD, SIG_IGN);
    kill(temp->pid, SIGCONT);
    fpid = temp->pid; 
    int status;
    struct Job j = *temp;
    delJob(fpid);
    waitpid(j.pid, &status, WUNTRACED);
    fpid = 0;
    if(WIFSTOPPED(status))
        appendJob(j.pid,j.command);
    //signal(SIGCHLD, handler);
    return 1;
}

int bg_nash(int n, char** args){

    if(n != 2){
        printf("bg: Incorrect number of arguments\n");
        return 0;
    }
    int jobno = atoi(args[1]);
    if(jobno > no_jobs){
        printf("bg: No such job exists\n");
        return 0;
    }
    struct Job* temp = nth_node(jobno);
    kill(temp->pid, SIGCONT);
    return 1;
}

int overkill_nash(int n, char**args){
    struct Job* i = head;
    struct Job* temp;
    while(i){
        temp = i;
        i = i->next;
        if(kill(temp->pid,9) == -1){
            perror("");
            return 0;
        }
        delJob(temp->pid);
    }
    return 1;
}


