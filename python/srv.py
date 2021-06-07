from http.server import BaseHTTPRequestHandler, HTTPServer

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Connection", "keep-alive")
        self.send_header("Content-type", "text/plain")
        self.send_header("Content-length", "5")
        self.end_headers()
        self.wfile.write(bytes("Hello", "utf-8"))
    def log_message(self, format, *args):
        return

server = HTTPServer(("", 7012), Handler)

try:
    server.serve_forever()
except KeyboardInterrupt:
    pass

server.server_close()
