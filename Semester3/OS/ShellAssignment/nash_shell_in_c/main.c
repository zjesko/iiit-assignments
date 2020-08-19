/*
 * Author: Akshat Chhajer
 * Date: 19 August 2019
 * Purpose: A unix style shell written in c supporting basix commands
 */
    
#include "nash.h"

int main(){
    
    fpid = 0; 
    signal(SIGINT, handler); 
    signal(SIGQUIT, handler); 
    signal(SIGTSTP, zhandler); 
    signal(SIGCHLD, child_exited); 
    no_jobs = 0;
    head = NULL;
    getcwd(home, sizeof(home));
    calculate_hash();

    while(1){
        int no_commands = get_commands();
        for(int i=0 ;i<no_commands; i++){
            int exitcode = exec_com(commands[i]);
        } 
    }
    return 0;
}


