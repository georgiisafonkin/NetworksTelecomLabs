package main

import (
	"webserver/internal/server"
	"net/http"
)

func main() {
	http.HandleFunc("/", server.HandlerFunc)
	http.ListenAndServe(":8080", nil)
	// var frh server.FileReaderHandler;
	// frh.ReadFileData("test.txt")
}