package main

import(
	"fmt"
	"webserver/internal/client/inreader"
)

func main() {
	//TODO Client App Initialization here
	fmt.Println("Welcome to my Web server!\nYou can download/upload data with this server.\n")
	for {
		str, err := inreader.ReadInput()
		if err != nil {
			//TODO Handle this case
		}
		command, arg, err := inreader.ParseInput(str)
		if err != nil {
			//TODO handle this case
			fmt.Printf("%s\nThe Work continues")
		}
		fmt.Printf("Command: %s\nArg: %s\n", command, arg)

		switch command {
		case "upload":
			fmt.Println("upload case")
		case "download":
			fmt.Println("download case")
		default:
			fmt.Println("unknown command")
		}


	}
}