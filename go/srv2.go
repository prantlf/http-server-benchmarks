package main

import (
	"log"
	"net"
)

func main() {
	l, err := net.Listen("tcp", "127.0.0.1:7012")
	if err != nil {
		log.Fatal("Error listening: ", err)
	}
	defer l.Close()

	for {
		if conn, err := l.Accept(); err != nil {
			log.Fatal("Error accepting: ", err)
		} else {
			go handler(conn)
		}
	}
}

func handler(conn net.Conn) {
	defer conn.Close()
	buf := make([]byte, 1024)
	if _, err := conn.Read(buf); err != nil {
		// log.Fatal("Error reading:", err)
	}

	res := `HTTP/1.0 200 OK
Connection: keep-alive
Content-Length: 5
Content-Type: text/plain

Hello`
	conn.Write([]byte(res))
}
