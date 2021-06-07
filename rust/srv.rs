use async_std::net::TcpListener;
use async_std::net::TcpStream;
use async_std::prelude::*;
use async_std::task;

#[async_std::main]
async fn main() {
  let listener = TcpListener::bind("127.0.0.1:7012").await.unwrap();

  loop {
    let (stream, _) = listener.accept().await.unwrap();
    stream.set_nodelay(true).expect("set_nodelay call failed");
    task::spawn(handler(stream));
  }
}

async fn handler(mut stream: TcpStream) {
  let mut buffer = [0; 1024];
  stream.read(&mut buffer).await.unwrap();

  let response = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/plain\r\nContent-length: 5\r\n\r\nHello";
  stream.write(response.as_bytes()).await.unwrap();
  stream.flush().await.unwrap();
}
