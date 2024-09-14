package server

import (
	"fmt"
	"os"
	"io"
	"net/http"
)

func HandlerFunc(w http.ResponseWriter, r *http.Request) {
	switch r.Method {
	case http.MethodGet:
		//Handle file download
		fmt.Println("I am in DOWNLOAD case")
		fileDownload(w, r)
	case http.MethodPost:
		//Handle file upload
		fmt.Println("I am in UPLOAD case")
		fileUpload(w, r)
	default:
		http.Error(w, "Invalid request method", http.StatusBadRequest)
	}
}

func fileUpload(w http.ResponseWriter, r *http.Request) {
	err := r.ParseMultipartForm(128 << 20) //32MB max file size
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return	
	}

	file, handler, err := r.FormFile("file") //TODO is it okay???
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	defer file.Close()

	// Read the entire file into a byte slice
	fileBytes, err := io.ReadAll(file)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	//Save the file to a directory
	err = os.WriteFile("uploads/"+handler.Filename, fileBytes, 0666) //TODO Should change directory
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	fmt.Fprint(w, "File uploaded successfully!")	
}

func fileDownload(w http.ResponseWriter, r *http.Request) {
	filePath := "uploads/" + r.URL.Query().Get("file")

	file, err := os.Open(filePath)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	defer file.Close()


	w.Header().Set("Content-Disposition", "attachment; filename="+filePath)

	http.ServeFile(w, r, filePath)
}