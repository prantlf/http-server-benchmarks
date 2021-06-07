package main

import (
	"io"
	"log"
	"net/http"
)

func handler(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Connection", "keep-alive")
	w.Header().Set("Content-Type", "text/plain")
	w.Header().Set("Content-Length", "5")
	w.WriteHeader(http.StatusOK)
	io.WriteString(w, "Hello")
}

func main() {
	http.HandleFunc("/", handler)
	log.Fatal(http.ListenAndServe(":7012", nil))
}
