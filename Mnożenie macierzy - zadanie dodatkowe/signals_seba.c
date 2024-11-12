#include "signals_seba.h"

void handle_signal(int sig, siginfo_t *siginfo, void *context)
{
    printf("Received signal %d with value %d from process %ld\n", sig, siginfo->si_value.sival_int, (long)siginfo->si_pid);
}

int main_signal_communication_test()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &handle_signal;
    sigaction(SIGUSR1, &sa, NULL);

    if (fork() == 0)
    {
        // Child process
        sleep(1); // Give parent time to set up handler
        union sigval value;
        value.sival_int = __INT_MAX__;       // Arbitrary data to send
        sigqueue(getppid(), SIGUSR1, value); // Send signal with data to parent
        exit(0);
    }
    else
    {
        // Parent process waits for signal
        pause(); // Wait for signal to be delivered
    }
    return 0;
}