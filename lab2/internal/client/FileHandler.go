package FileHandler

import (
	"fmt"
	"io/ioutil"
	"os"
)

func PrepareFileForSending(filename string) ([]byte, error) {
	file, err = os.Open(filename)
	if err != nil {
		return nil, err
	}	
	
	defer file.Close()

	data, err = io.ReadAll(file)
	if err != nil {
		return nil, err
	}

	return data, err
}