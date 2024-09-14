package inreader

import (
	"fmt"
	"bufio"
	"os"
	"strings"
)

func ReadInput() (string, error) {
	var str string
	r := bufio.NewReader(os.Stdin)
	str, err := r.ReadString('\n')
	if err != nil {
		//TODO Handle this case
		fmt.Printf("%s", err)
		return "error", err
	}
	return str, nil
}

func ParseInput(str string) (string, string, error) {
	fmt.Println(str)
	tokens := strings.Split(str, " ")
	fmt.Println(tokens)
	if len(tokens) != 2 {
		return "", "", nil
	}
	return tokens[0], tokens[1], nil  
}