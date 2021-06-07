require "http/server"

server = HTTP::Server.new do |context|
  context.response.content_type = "text/plain"
  context.response.content_length = 5
  context.response.print "Hello"
end

server.listen(7012)
