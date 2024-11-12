#include <signal.h>
#include <stddef.h>
void handle_signal(int sig, siginfo_t *siginfo, void *context);
int main_signal_communication_test();