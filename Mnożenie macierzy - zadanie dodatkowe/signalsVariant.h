void foo(){
    // Set up the signal handler for the real-time signal in the parent process
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_sig_result;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG_RESULT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    multiplyMatricesSignalVariant(first, second, result, row1, col1, col2);
    // Wait until all elements are computed
    while (completed_elements < row1 * col2)
    {
        pause(); // Wait for signals from children
    }
}