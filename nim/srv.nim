import asynchttpserver, asyncdispatch

#proc main {.async.} =
var server = newAsyncHttpServer()
proc cb(req: Request) {.async.} =
  let headers = {
    "Connection": "keep-alive", "Content-type": "text/plain", "Content-Length": "5"
  }
  await req.respond(Http200, "Hello", headers.newHttpHeaders())
 
waitFor server.serve(Port(7012), cb)
#  server.listen Port(7012)
#  while true:
#    if server.shouldAcceptRequest():
#      await server.acceptRequest(cb)
#    else:
#      poll()

#asyncCheck main()
#runForever()
