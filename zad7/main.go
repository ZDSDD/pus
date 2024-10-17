package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"time"
)

const (
	username = "admin"
	password = "pass123"
	timeout  = 2 * time.Second // limit czasu na wprowadzenie liczby
)

var sum int

// Funkcja obsługująca logowanie
func login() bool {
	reader := bufio.NewReader(os.Stdin)

	fmt.Print("Podaj login: ")
	inputUser, _ := reader.ReadString('\n')
	fmt.Print("Podaj hasło: ")
	inputPass, _ := reader.ReadString('\n')

	// Usuwanie nowej linii
	inputUser = inputUser[:len(inputUser)-1]
	inputPass = inputPass[:len(inputPass)-1]

	// Sprawdzanie poprawności
	if inputUser == username && inputPass == password {
		fmt.Println("Poprawnie zalogowano!")
		return true
	}

	fmt.Println("Błędny login lub hasło!")
	return false
}

// Funkcja obsługująca wczytywanie liczby z limitem czasu
func inputNumberWithTimeout() (int, error) {
	reader := bufio.NewReader(os.Stdin)
	inputChan := make(chan string, 1)

	// Gorutyna do wczytywania liczby
	go func() {
		input, _ := reader.ReadString('\n')
		inputChan <- input
	}()

	// Timer na 2 sekundy
	timer := time.NewTimer(timeout)

	// Oczekiwanie na dane lub timeout
	select {
	case input := <-inputChan:
		timer.Stop() // Zatrzymanie timera, gdy użytkownik wprowadzi liczbę
		num, err := strconv.Atoi(input[:len(input)-1])
		if err != nil {
			fmt.Println("To nie jest poprawna liczba!")
			return 0, err
		}
		return num, nil
	case <-timer.C:
		return 0, fmt.Errorf("przekroczono limit czasu")
	}
}

func main() {
	for {
		// Logowanie
		for !login() {
			fmt.Println("Spróbuj ponownie.")
		}

		sum = 0
		for {
			fmt.Print("Podaj liczbę: ")

			num, err := inputNumberWithTimeout()
			if err != nil {
				fmt.Println("Wylogowano po 2 sekundach bezczynności.")
				break
			}

			sum += num
			fmt.Println("Aktualna suma:", sum)
		}
	}
}
