/*
 * Author: Akshat Chhajer
 * Date: 19 August 2019
 * Purpose: A unix style shell written in c supporting basix commands
 */

#ifndef HEADER
#define HEADER

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <pwd.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <time.h>
#include <grp.h>
#include <sys/select.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>

#define BUF_PWD 1024
#define BUF_USR 1024
#define BUF_HST 1024
#define BUF_HOM 1024
#define BUF_PMT 1024
#define BUF_NCM 128
#define BUF_COM 1024
#define BUF_TOK 1024
#define HASH_MAX 8192
#define BUF_ENV 1024

char pwd[BUF_PWD];
char u_name[BUF_USR];
char host[BUF_PWD];
char home[BUF_HOM];
char prompt[BUF_PMT];
char *commands[BUF_COM];
char *tokens[BUF_TOK];
int flag_hash[256];
int (*cmd_functions[HASH_MAX]) (int, char **);

void handler(int sig);
void zhandler(int sig);
void torelative(char *path);
int hash(unsigned char *str);
int exit_nash(int n, char** args);
int pwd_nash(int n, char** args);
int echo_nash(int n, char** args);
int cd_nash(int n, char** args);
int clear_nash(int n, char** args);
int ls_nash(int n, char** args);
int pinfo_nash(int n, char** args);
int nightswatch_nash(int n, char** args);
void child_exited(int n);
int execute_program(char* command);
int history_nash(int k, char** args);
int setenv_nash(int k, char** args);
int unsetenv_nash(int k, char** args);
int jobs_nash(int n, char** args);
int kjob_nash(int n, char** args);
int bg_nash(int n, char** args);
int fg_nash(int n, char** args);
int overkill_nash(int n, char** args);
int cronjob_nash(int n, char** args);

void calculate_hash();
void update();
char* get_prompt();
void local_history(char *cmd);
int get_commands();
int tokenize(char *command);
int extract_flags(int n, char** args);
int redirect(int n, char** args);
int exec_com(char *command);
int no_jobs;
struct Job* nth_node(int n);

struct Job{
    int pid;
    char command[BUF_COM];
    struct Job* next;
};

struct Job* head;

struct Job* newJob(int pid, char* cmd);
void appendJob(int pid, char* cmd);
int delJob(int pid);

int fpid;
#endif
