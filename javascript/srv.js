const http = require('http')

function handler(req, res) {
  res.writeHead(200, {
    'Connection': 'keep-alive', 'Content-Type': 'text/plain', 'Content-Length': 5
  })
  res.end('Hello')
}

http.createServer(handler).listen(7012)
