#include <arpa/inet.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LISTENQ 1024
#define MAXLINE 1024
#define RIO_BUFSIZE 1024

typedef struct {
  int rio_fd;
  int rio_cnt;
  char *rio_bufptr;
  char rio_buf[RIO_BUFSIZE];
} rio_t;

typedef struct sockaddr SA;

typedef struct {
  char filename[512];
  off_t offset;
  size_t end;
} http_request;

void rio_readinitb(rio_t *rp, int fd) {
  rp->rio_fd = fd;
  rp->rio_cnt = 0;
  rp->rio_bufptr = rp->rio_buf;
}

ssize_t writen(int fd, void *usrbuf, size_t n) {
  size_t nleft = n;
  ssize_t nwritten;
  char *bufp = usrbuf;

  while (nleft > 0) {
    if ((nwritten = write(fd, bufp, nleft)) <= 0) {
      if (errno == EINTR)
        nwritten = 0;
      else
        return -1;
    }
    nleft -= nwritten;
    bufp += nwritten;
  }
  return n;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
  int cnt;
  while (rp->rio_cnt <= 0) {

    rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
               sizeof(rp->rio_buf));
    if (rp->rio_cnt < 0) {
      if (errno != EINTR)
        return -1;
    }
    else if (rp->rio_cnt == 0)
      return 0;
    else
      rp->rio_bufptr = rp->rio_buf;
  }

  cnt = n;
  if (rp->rio_cnt < n)
    cnt = rp->rio_cnt;
  memcpy(usrbuf, rp->rio_bufptr, cnt);
  rp->rio_bufptr += cnt;
  rp->rio_cnt -= cnt;
  return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
  int n, rc;
  char c, *bufp = usrbuf;

  for (n = 1; n < maxlen; n++){
    if ((rc = rio_read(rp, &c, 1)) == 1){
      *bufp++ = c;
      if (c == '\n')
        break;
    } else if (rc == 0){
      if (n == 1)
        return 0;
      else
        break;
    } else
      return -1;
  }
  *bufp = 0;
  return n;
}

int open_listenfd(int port) {
  int listenfd, optval=1;
  struct sockaddr_in serveraddr;

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;
  fcntl(listenfd, O_CLOEXEC);

  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
           (const void *)&optval , sizeof(int)) < 0)
    return -1;

  if (setsockopt(listenfd, 6, TCP_NOPUSH,
           (const void *)&optval , sizeof(int)) < 0)
    return -1;

  if (setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY,
           (const void *)&optval , sizeof(int)) < 0)
    return -1;

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)port);
  if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
    return -1;

  if (listen(listenfd, LISTENQ) < 0)
    return -1;
  return listenfd;
}

void url_decode(char* src, char* dest, int max) {
  char *p = src;
  char code[3] = { 0 };
  while(*p && --max) {
    if(*p == '%') {
      memcpy(code, ++p, 2);
      *dest++ = (char)strtoul(code, NULL, 16);
      p += 2;
    } else {
      *dest++ = *p++;
    }
  }
  *dest = '\0';
}

void parse_request(int fd, http_request *req) {
  rio_t rio;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE];
  req->offset = 0;
  req->end = 0;

  rio_readinitb(&rio, fd);
  rio_readlineb(&rio, buf, MAXLINE);
  sscanf(buf, "%s %s", method, uri);
  /* read all */
  while(buf[0] != '\n' && buf[1] != '\n') {
    rio_readlineb(&rio, buf, MAXLINE);
    if(buf[0] == 'R' && buf[1] == 'a' && buf[2] == 'n') {
      sscanf(buf, "Range: bytes=%llu-%lu", &req->offset, &req->end);
      if( req->end != 0) req->end ++;
    }
  }
  char* filename = uri;
  if(uri[0] == '/') {
    filename = uri + 1;
    int length = strlen(filename);
    if (length == 0){
      filename = ".";
    } else {
      for (int i = 0; i < length; ++ i) {
        if (filename[i] == '?') {
          filename[i] = '\0';
          break;
        }
      }
    }
  }
  url_decode(filename, req->filename, MAXLINE);
}

void handler(int fd) {
  char* const buf = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/plain\r\nContent-length: 5\r\n\r\nHello";
  writen(fd, buf, strlen(buf));
}

void process(int fd, struct sockaddr_in *clientaddr) {
  http_request req;
  parse_request(fd, &req);
  handler(fd);
}

int main(int argc, char** argv) {
  struct sockaddr_in clientaddr;
  int listenfd, connfd;
  char buf[256];
  char *path = getcwd(buf, 256);
  socklen_t clientlen = sizeof clientaddr;
  listenfd = open_listenfd(7012);
  if (listenfd > 0) {
    printf("listen on port %d\n", 7012);
  } else {
    perror("ERROR");
    exit(listenfd);
  }

  signal(SIGPIPE, SIG_IGN);

  for(int i = 0; i < 10; i++) {
    int pid = fork();
    if (pid == 0) {
      while(1){
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        process(connfd, &clientaddr);
        close(connfd);
      }
    } else if (pid > 0) {
      printf("child pid is %d\n", pid);
    } else {
      perror("fork");
    }
  }

  for(;;) {
    connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
    process(connfd, &clientaddr);
    close(connfd);
  }

  return 0;
}
