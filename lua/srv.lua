local socket = require 'socket'

local server = assert(socket.bind('127.0.0.1', 7012))
local ip, port = server:getsockname()

while 1 do
  local client, errmsg = server:accept()
  if client then
    client:setoption("tcp-nodelay", true)
    client:send('HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/plain\r\nContent-length: 5\r\n\r\nHello')
    client:close()
  else
    io.stderr:write('Failed to accept connection:' .. errmsg .. '\n')
  end
end
