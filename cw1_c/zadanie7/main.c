#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>

#define LOGIN "user"
#define PASSWORD "pass"

volatile sig_atomic_t timeout_flag = 0;
jmp_buf env;

void handle_alarm(int sig) {
    timeout_flag = 1;
    longjmp(env, 1);
}

int main() {
    char input_login[50];
    char input_password[50];
    int sum = 0;
    int number;

    signal(SIGALRM, handle_alarm);

    while (1) {
        printf("Podaj login: ");
        scanf("%s", input_login);
        printf("Podaj hasło: ");
        scanf("%s", input_password);

        if (strcmp(input_login, LOGIN) == 0 && strcmp(input_password, PASSWORD) == 0) {
            printf("Zalogowano pomyślnie.\n");

            sum = 0;
            while (1) {
                printf("Podaj liczbę (masz 2 sekundy): ");
                fflush(stdout);

                timeout_flag = 0;
                alarm(2);

                if (setjmp(env) == 0) {
                    if (scanf("%d", &number) == 1) {
                        alarm(0);
                        sum += number;
                        printf("Aktualna suma: %d\n", sum);
                    } else {
                        alarm(0);
                        printf("Nieprawidłowe dane. Spróbuj ponownie.\n");
                        while (getchar() != '\n');  // Czyszczenie bufora
                    }
                } else {
                    printf("\nPrzekroczono czas oczekiwania. Wylogowywanie...\n");
                    break;
                }
            }
        } else {
            printf("Nieprawidłowy login lub hasło. Spróbuj ponownie.\n");
        }
    }

    return 0;
}