#include "nash.h"

void torelative(char *path){
    if(strlen(path) >= strlen(home) && strncmp(path, home, strlen(home)) == 0)
        sprintf(path, "~%s", path + strlen(home));
}

int hash(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while (c = *str++){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        hash = hash % HASH_MAX;
    }
    return hash;
}


void calculate_hash(){

    cmd_functions[hash("pwd")] = &pwd_nash;
    cmd_functions[hash("echo")] = &echo_nash;
    cmd_functions[hash("cd")] = &cd_nash;
    cmd_functions[hash("clear")] = &clear_nash;
    cmd_functions[hash("c")] = &clear_nash;
    cmd_functions[hash("quit")] = &exit_nash;
    cmd_functions[hash("exit")] = &exit_nash;
    cmd_functions[hash("ls")] = &ls_nash;
    cmd_functions[hash("pinfo")] = &pinfo_nash;
    cmd_functions[hash("nightswatch")] = &nightswatch_nash;
    cmd_functions[hash("history")] = &history_nash;
    cmd_functions[hash("setenv")] = &setenv_nash;
    cmd_functions[hash("unsetenv")] = &unsetenv_nash;
    cmd_functions[hash("jobs")] = &jobs_nash;
    cmd_functions[hash("kjob")] = &kjob_nash;
    cmd_functions[hash("fg")] = &fg_nash;
    cmd_functions[hash("bg")] = &bg_nash;
    cmd_functions[hash("overkill")] = &overkill_nash;
    cmd_functions[hash("cronjob")] = &cronjob_nash;
}


void update(){

    //Update username (u_name)
    struct passwd *p = getpwuid(getuid());
    char *name = p -> pw_name;
    strcpy(u_name, name);

    //Update hostname (host)
    gethostname(host, sizeof(host));

    //Update present working directory (pwd)
    getcwd(pwd, sizeof(pwd));

}

char* get_prompt(){

    char p[BUF_PMT];
    char path[BUF_PWD];

    update();

    //Check if ~ should be used
    strcpy(path, pwd);
    torelative(path);

    sprintf(p, "<\x1b[34m%s\x1b[0m@\x1b[34m%s\x1b[32m[%s]\x1b[0m> ", u_name, host, path);

    strcpy(prompt, p);
    return prompt;
}


void local_history(char *cmd){

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
    FILE *fd = fopen(hist_path, "w");

    int counter = 0;

    if(n>=20)
        counter++;

    char tp[BUF_COM];
    tp[0] ='\0';

    for(int i=counter;i<n;i++){
        strcat(tp, l[i]);    
        strcat(tp, ",");    
    }
    strcat(tp,cmd);
    fprintf(fd,"%s",tp);

    fclose(fd);
}

int get_commands(){

    char *pr = get_prompt();
    char *cmds = readline(pr);

    int n = 0;

    if(cmds == NULL){
        printf("\n");
        return 0;
    }
    add_history(cmds);
    //local_history(cmds);

    commands[0] = strtok(cmds,";");

    while(commands[n] != NULL)
        commands[++n] = strtok(NULL, ";");

    return n;
}

int tokenize(char *command){

    int n = 0;

    char com[BUF_COM];
    strcpy(com,command);

    tokens[0] = strtok(com, " \t\n\r\a");

    while(tokens[n] != NULL)
        tokens[++n] = strtok(NULL, " \t\n\r\a");

    return n;
}

int extract_flags(int n, char** args){

    for(int i=0;i<256;i++)
        flag_hash[i] = 0;

    int new_n = 0;

    for(int i=0;i<n;i++){
        if(args[i][0] == '-'){
            for(int j=1;j<strlen(args[i]);j++)
                flag_hash[args[i][j]] = 1;
        }
        else{
            strcpy(args[new_n], args[i]);
            new_n++;
        }
    }
    return new_n;
}

int redirect(int n, char** args){

    int out = -1, in = -1, append = -1;
    for(int i=0;i<n;i++){
        if(!strcmp(args[i], ">"))
            out = i;
        if(!strcmp(args[i], ">>"))
            append = i;
        if(!strcmp(args[i], "<"))
            in = i;
    }
    if(out >= 0){
        args[out] = NULL;
        int fd = open(args[out+1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if(fd < 0){
            perror("Unable to open the output file.");
            return n-2;
        }
        args[out+1] = NULL;
        dup2(fd, 1);
        close(fd);
    }

    if(append >= 0){
        args[append] = NULL;

        int fd = open(args[append+1], O_RDWR | O_APPEND | O_CREAT, 0644);
        if(fd < 0){
            perror("Unable to open the output file.");
            return n-2;
        }
        args[append+1] = NULL;
        dup2(fd, 1);
        close(fd);
    }

    if(in >= 0){
        strcpy(args[in], args[in+1]);
        args[in+1] = NULL;
        /*
           args[in] = NULL;
           int fd = open(args[in+1], O_RDONLY, 0);
           if(fd < 0){
           perror("Unable to open the input file.");
           return n-2;
           }
           args[in+1] = NULL;
           dup2(fd, 0);
           close(fd);
           */
        n--;

    }
    //if(out >= 0 || append >= 0 || in >= 0)
    if(out >= 0 || append >= 0)
        n-=2;

    return n;
}

void child_exited(int n){

    int status;
    pid_t wpid = waitpid(-1, &status, WUNTRACED);

    if(wpid > 0 && WIFEXITED(status)==0){
        delJob(wpid);
        printf("\nProcess with pid %d exited normally\n", wpid);
    }
    if(wpid > 0 && WIFSIGNALED(status)==0){
        delJob(wpid);
        printf("\nProcess with pid %d exited due to a user-defined signal\n", wpid);
    }
}

int execute_program(char* command){

    int no_tokens = tokenize(command);
    int savestdout = dup(1);
    no_tokens = redirect(no_tokens, tokens);
    if(no_tokens == -1)
        return -1;     

    if((*cmd_functions[hash(tokens[0])]) != NULL){
        if(strcmp(tokens[0], "cronjob")!=0)
            no_tokens = extract_flags(no_tokens, tokens);
        int exitcode = (*cmd_functions[hash(tokens[0])])(no_tokens, tokens);
        dup2(savestdout,1);
        return exitcode;
    }

    int bg = 0;

    pid_t pid = fork();

    if(strcmp(tokens[no_tokens-1],"&")==0){
        no_tokens--;
        tokens[no_tokens] = NULL;
        bg = 1;

    } 

    if(pid < 0){
        perror("Fork failed:");
        return 0;
    }
    else if(pid == 0){
        if(bg)
            setpgid(0, 0);

        int proc = execvp(tokens[0], tokens);
        if(proc == -1)
            perror("Error executing:");

        exit(EXIT_FAILURE);
    }
    else {

        int status;
        if(!bg){
            fpid = pid;
            waitpid(pid, &status, WUNTRACED);
            if(WIFSTOPPED(status)){
                appendJob(pid, command);
            }
            //do{
            //    waitpid(pid, &status, WUNTRACED);
            //} while (!WIFEXITED(status) && !WIFSIGNALED(status));
            fpid = 0;
        }
        else{
            char com[BUF_COM];
            strncpy(com, command, strlen(command) - 2);
            com[strlen(command) - 2] = '\0';
            appendJob(pid,com);
            signal(SIGCHLD, child_exited);
        }
    }
    dup2(savestdout,1);
    return 1; 
}

int exec_pipe(char *command){

    int fd[2];
    pipe(fd);
    pid_t pid = fork();

    if (pid == 0){
        dup2(fd[1], 1);
        execute_program(command);
        abort();
    }

    else if (pid > 0){
        dup2(fd[0], 0);
        close(fd[1]);
    }

    close(fd[0]);
    close(fd[1]);
}


int exec_com(char *command){

    int no_pipes = 0;
    char* pipes[BUF_COM]; 

    int savestdout = dup(1);
    int savestdin = dup(0);
    pipes[0] = strtok(command, "|");

    while(pipes[no_pipes] != NULL)
        pipes[++no_pipes] = strtok(NULL, "|");
    if(no_pipes == 1)
        return execute_program(pipes[0]);

    for(int i=0;i<no_pipes;i++){
        if(i == no_pipes - 1){
            dup2(savestdout,1);
            execute_program(pipes[i]);
        }
        else
            exec_pipe(pipes[i]);
            
    }

    dup2(savestdin,0);
    dup2(savestdout,1);
    return 0;
}

struct Job* nth_node(int n){
    struct Job* temp = head;
    for(int i=1;i<n;i++){
        if(temp!=NULL)
            temp = temp->next;
        else
            return NULL;
    }
    return temp;
}


struct Job* newJob(int pid, char* cmd){
    struct Job* temp = (struct Job*)malloc(sizeof(struct Job));
    temp->pid = pid;
    strcpy(temp->command, cmd);
    temp->next = NULL;
    return temp;
}

void appendJob(int pid, char* cmd){
    struct Job* new = newJob(pid, cmd);
    
    if(head == NULL)
        head = new;
    else{
        struct Job* i = head;
        while(i->next)
            i = i->next;
        i->next = new;
    }
    no_jobs++;
}

int delJob(int pid){
    no_jobs--;
    struct Job* temp = head;
    
    if(temp->pid == pid){
        head = temp->next;
        free(temp);
        return 1;
    }
    while(temp->next){
        if(temp->next->pid == pid){
            struct Job* n = temp->next->next;
            free(temp->next);
            temp->next = n;
            return 1;
        }
        temp = temp->next;
    }
    return 1;
}


int cronjob_nash(int n, char** args){
    
    char com[BUF_COM], tot, freq;
    strcpy(com, "");
    int i = 0;
    while(i < n){
        if(strcmp(args[i], "-c") == 0){
            i++;
            while(i < n && strcmp(args[i%n], "-t") && strcmp(args[i%n], "-p")){
                strcat(com, args[i]);
                strcat(com, " ");
                i++;
            }
        }
        if(strcmp(args[i%n], "-t") == 0)
            tot = atoi(args[i+1]);
        if(strcmp(args[i%n], "-p") == 0)
            freq = atoi(args[i+1]);
        i++;
    }

    int pid = fork();
    if(pid == 0){
        setpgid(0,0);
        signal(SIGINT, SIG_DFL);
        int k = tot / freq;
        while(k--){
            sleep(freq);
            exec_com(com);
        }
        exit(0);
    }
    else{
        char croncom[BUF_COM];
        strcpy(croncom, "");
        for(int i=0;i<n;i++)
           strcat(croncom, args[i]); 
        appendJob(pid,croncom);
        signal(SIGCHLD, child_exited);
    }
    return 0;
}           

void handler(int sig){
    printf("PD: %d\n",fpid);
    if(fpid)
        kill(fpid,sig);
}

void zhandler(int sig){
    return;
}
