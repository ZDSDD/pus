#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static int sigintCounter = 0;
static int sigusr1Counter = 0;

void handler_sigint(int sig){
    printf("\nNaciśnięto Ctrl+c\n");
    sigintCounter += 1;
    sleep(5);
    printf("\nZakończono obsługiwać siginta. Stan licznika: %d", sigintCounter);
}
void handler_sigint_new(int sig){
    printf("\nNaciśnięto Ctrl+c. Jest to drugi handler!\n");
    sleep(1);
    printf("Wracam do obsługi siginita domyślnym handlerem....");
    signal(SIGINT, handler_sigint);
}
void handler_sigusr1(int sig){
    printf("\nObługuję sygnał SIGUSR1\n");
    sigusr1Counter += 1;
    sleep(5);
    printf("\nZakończono obsługiwać sigusr1. Stan licznika: %d", sigusr1Counter);
}

int main(){
    signal(SIGINT, handler_sigint);
    signal(SIGINT, handler_sigint_new);
    signal(SIGUSR1, handler_sigusr1);
    for(;;){
        printf(".");
        fflush(stdout);
        sleep(1);
    }
}