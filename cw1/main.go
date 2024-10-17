package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"
)

func main() {
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs)

	done := make(chan bool, 1)

	go func() {
		for {
			sig := <-sigs
			fmt.Printf("Received signal: %v\n", sig)
			if sig == syscall.SIGPWR {
				done <- true
				return
			}
			if sig == syscall.SIGKILL {
				fmt.Println("hahahha i am not afraid of you sigkill!")
			}
		}
	}()

	fmt.Println("PID:", os.Getpid())
	fmt.Println("Awaiting signals. Use 'kill -SIGNAL PID' to send signals.")
	fmt.Println("SIGINT or SIGTERM will cause the program to exit.")

	// Keep the program running
	for {
		select {
		case <-done:
			fmt.Println("Exiting...")
			return
		default:
			fmt.Print(".")
			time.Sleep(1 * time.Second)
		}
	}
}
