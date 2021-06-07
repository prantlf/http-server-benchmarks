local server = require 'http.server'
local headers = require 'http.headers'

local s = server.listen {
  host = 'localhost',
  port = 7012,
  onstream = function (sv, st)
    local rsh = headers.new()
    rsh:append(':status','200')
    rsh:append('connection','keep-alive')
    rsh:append('content-type','text/plain')
    rsh:append('content-length','5')
    st:write_headers(rsh, false)
    st:write_chunk("Hello", true)
  end
}

s:listen()
s:loop()
