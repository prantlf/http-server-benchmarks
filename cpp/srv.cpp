#include <iostream>
#include <sstream>
#include <string_view>
#include <thread>
#include <vector>
#include <initializer_list> 
#include <netinet/tcp.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip>
#include <atomic>
#include <csignal>

namespace {

using TextPair = std::pair<std::string_view, std::string_view>;

class ErrNoException: public std::runtime_error {
public:
	ErrNoException(const char *desc):std::runtime_error(buildMsg(errno,desc).c_str()) {}
protected:
	static std::string buildMsg(int err, const char *desc) {
		std::ostringstream bld;
		bld << desc << " - " << strerror(err) << "(" << err << ")";
		return bld.str();
	}
};

template<typename T, typename CloseFn, CloseFn closeFn, const T *invval, class Tag = void>
class RAII {
public:
	RAII():h(*invval) {}
	RAII(T &&h):h(std::move(h)) {}
	RAII(const T &h):h(h) {}
	RAII(RAII &&other):h(other.h) {other.h = *invval;}
	RAII &operator=(RAII &&other) {
		if (this != &other) {
			close();
			h = other.h;
			other.h = *invval;
		}
		return *this;
	}
	operator T() const {return h;}
	T get() const {return h;}
	T operator->() const {return h;}
	void close() {
		if (!is_invalid())  closeFn(h);
		h = *invval;
	}
	~RAII() {close();}
	T detach() {T res = h; h = *invval; return res;}
	bool is_invalid() const {return h == *invval;}
	bool operator !() const {return is_invalid();}
	T *ptr() {return &h;}
	const T *ptr() const {return &h;}
protected:
	T h;
};

template<typename T> class pointer_raii_traits_t {
public:
	static T *null;
	static void free(T *x) {operator delete(static_cast<void *>(x));}
	using FreeFn = decltype(&free);
	using RAII = ::RAII<T *, FreeFn, &pointer_raii_traits_t<T>::free, &null>;
};

template<typename T> T *pointer_raii_traits_t<T>::null = nullptr;

static const int invalid_descriptor = -1;
class SocketDescTag;
using Socket = RAII<int, decltype(&close), &close, &invalid_descriptor, SocketDescTag>;
class FileDescTag;
using FileDesc = RAII<int, decltype(&close), &close, &invalid_descriptor, FileDescTag>;
using AddrInfo = RAII<addrinfo *, decltype(&freeaddrinfo), &freeaddrinfo, &pointer_raii_traits_t<addrinfo>::null>;

static Socket open_port(const std::string_view &portdef) {
	auto splt = portdef.rfind(':');
	std::string addr (portdef.substr(0,splt));
	std::string port (portdef.substr(splt+1));

	AddrInfo resolved;
	struct addrinfo hint{};

	hint.ai_flags = AI_PASSIVE;
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(addr.empty()?nullptr:addr.c_str(), port.c_str(),&hint, resolved.ptr())) {
		throw ErrNoException("getaddrinfo failed");
	}
	Socket sock = socket(resolved->ai_family, SOCK_STREAM, resolved->ai_protocol);
	fcntl(sock, O_CLOEXEC);
	if (!sock) throw ErrNoException("socket failed");

	int flag = 1;
	(void)setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

	if (bind(sock,resolved->ai_addr, resolved->ai_addrlen)) {
		throw ErrNoException("tcp bind failed");
	}
	if (listen(sock,SOMAXCONN)) {
		throw ErrNoException("tcp listen failed");
	}

	(void)setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(char *) &flag,sizeof(int));
	return sock;
}

static int stream_read(const Socket &sock, char *buffer, std::size_t size) {
	return recv(sock,buffer,size,0);
}
static int stream_write(const Socket &sock, const std::string_view &buffer) {
	return send(sock,buffer.data(),buffer.length(),0);
}
static int stream_read(const FileDesc &sock, char *buffer, std::size_t size) {
	return read(sock,reinterpret_cast<unsigned char *>(buffer),size);
}
static int stream_write(const FileDesc &sock, const std::string_view &buffer) {
	return write(sock, reinterpret_cast<const unsigned char *>(buffer.data()),buffer.length());
}

template<typename Socket = Socket>
class Conn {
public:
	explicit Conn(Socket &&sock);
	std::string_view read();
	void put_back(const std::string_view &buff);
	bool read_line(std::string &ln, const std::string_view &sep = "\r\n");

	const Socket &get_socket() const {return sock;}
	void write(const std::string_view &data);

protected:
	Socket sock;
	char buffer[4096];
	std::string_view put_back_buff;
};

template<typename Socket>
Conn<Socket>::Conn(Socket &&sock):sock(std::move(sock)) {}

template<typename Socket>
std::string_view Conn<Socket>::read() {
	if (!put_back_buff.empty()) {
		auto tmp = put_back_buff;
		put_back_buff = std::string_view();
		return tmp;
	} else {
		int r = stream_read(sock,buffer,sizeof(buffer));
		if (r < 0)
			throw ErrNoException("read failed");
		std::string_view out(buffer, r);
		return out;
	}
}
template<typename Socket>
void Conn<Socket>::put_back(const std::string_view &buff) {
	put_back_buff = buff;
}

template<typename Socket>
bool Conn<Socket>::read_line(std::string &ln, const std::string_view &sep) {
	ln.clear();
	std::size_t nwln;
	std::string_view data;
	do {
		data = read();
		if (data.empty()) return false;
		auto startpos = ln.length();
		if (startpos>=sep.length()-1) startpos-=sep.length()-1;
		ln.append(data);
		nwln = ln.find(sep,startpos);
	} while (nwln == ln.npos);
	auto remain = data.substr(data.length()-(ln.length() - nwln - sep.length()));
	put_back(remain);
	ln.resize(nwln);
	return true;
}

template<typename Socket>
void Conn<Socket>::write(const std::string_view &data) {
	int r = stream_write(sock, data);
	if (r < 1) return;
	auto more_data = data.substr(r);
	if (!more_data.empty()) write(more_data);
}

class SplitString {
public:
	SplitString(const std::string_view &text, const std::string_view &sep, std::size_t limit):text(text),sep(sep),limit(limit) {}
	bool operator!() const {return reached_end;}
	std::string_view operator()() {
		auto pos = limit < 2?text.npos:text.find(sep);
		if (pos == text.npos) {
			auto res = text;
			text = std::string_view();
			reached_end = true;
			return res;
		} else {
			--limit;
			auto res = text.substr(0,pos);
			text = text.substr(pos+sep.length());
			return res;
		}
	}
	SplitString &operator()(std::string_view &res) {res = operator()();return *this;}
protected:
	std::string_view text;
	std::string_view sep;
	std::size_t limit;
	bool reached_end = false;
};

class Server {
public:
	Server(Socket &&s);
	void run() noexcept;

protected:
	Conn<Socket> conn;
	std::string hdrln;
	std::string ln;
	std::string tmpln;
	std::string tmpln2;
	std::string outbuff;

	bool run_1cycle();

	template<typename Iterable>
	void send_response(int code, const std::string_view &message, const std::string_view &httpver, Iterable &&beg, Iterable &&end, bool closeconn=false);

	using InlineHeaders = std::initializer_list<TextPair>;
	void send_response(int code, const std::string_view &message, const std::string_view &httpver, InlineHeaders hdrs, bool closeconn=false) {
		send_response(code,message, httpver, hdrs.begin(), hdrs.end(), closeconn);
	}
	void send_error(int code, const std::string_view &message, const std::string_view &httpver);

	template<typename T> static void number_to_string(T number, std::string &out, bool level2 = false);
};

Server::Server(Socket &&s)
	:conn(std::move(s)) {}

void Server::run() noexcept {
	try {
		while (run_1cycle());
	} catch (std::exception &e) {
		std::cerr << "ERROR:" << e.what() << std::endl;
	}
}

bool Server::run_1cycle(){
	if (!conn.read_line(hdrln)) return false;

  std::string_view cmd, uri, proto;
	SplitString(hdrln," ",3)(cmd)(uri)(proto);

	bool headers_sent = false;

	try {
		do {
			if (!conn.read_line(ln)) return false;
		} while (!ln.empty());

		send_response(200,"OK",proto,{
			{"Connection", "keep-alive"},
			{"Content-Type", "text/plain"},
			{"Content-Length", "5"}
		});

		headers_sent = true;

  	conn.write("Hello");

		return true;
	} catch (std::exception &e) {
		std::cerr << "ERROR:" << e.what() << std::endl;
		if (!headers_sent) send_error(404,"Not found",proto);
		return false;
	}
}

template<typename Iterable>
inline void Server::send_response(int code, const std::string_view& message,
		const std::string_view& httpver, Iterable&& beg, Iterable&& end,
		bool closeconn) {

  outbuff.clear();
	outbuff.append(httpver).append(" ");
	number_to_string(code, outbuff);
	outbuff.append(" ").append(message).append("\r\n");
	Iterable x = beg;
	while (x != end) {
		outbuff.append(x->first).append(": ").append(x->second).append("\r\n");
		++x;
	}
	if (closeconn) {
		outbuff.append("Connection: close\r\n");
	}
	outbuff.append("\r\n");
	conn.write(outbuff);
}

inline void Server::send_error(int code, const std::string_view& message, const std::string_view& httpver) {
	send_response(code, message, httpver, {
			{ "Content-Type","text/html;charset=utf-8" },
			{"Allow","GET"}},true);

  std::ostringstream output;
	output << code << " " << message;
	conn.write(output.str());
}

template<typename T>
inline void Server::number_to_string(T number, std::string& out, bool level2) {
	if (number < static_cast<T>(1)) {
		if (!level2) out.push_back('0');
	} else {
		number_to_string(number/10, out, true);
		out.push_back('0' + static_cast<char>((number % 10)));
	}
}

class SignalHandler {
public:
	SignalHandler(const Socket &sock):sock(sock) {}
	~SignalHandler() {
		if (this == cur_handler) cur_handler = nullptr;
	}

	bool operator !() const {return !signaled;}
	void signal() {
		signaled=true;
		shutdown(sock, SHUT_RD);
	}

	static void handler_proc(int) {
		if (cur_handler) cur_handler->signal();
	}

	void make_active() {cur_handler = this;}

protected:
	static SignalHandler *cur_handler;
	const Socket &sock;
	bool signaled = false;
};

SignalHandler *SignalHandler::cur_handler = nullptr;

}

int main(int argc, char **argv) {
	try {
		Socket sock;
		std::string_view port("127.0.0.1:7012");
    sock = open_port(port);

		SignalHandler sig_hndl(sock);
		sig_hndl.make_active();

		std::signal(SIGPIPE,SIG_IGN);
		std::signal(SIGCHLD,SIG_IGN);
		std::signal(SIGQUIT,&SignalHandler::handler_proc);
		std::signal(SIGTERM,&SignalHandler::handler_proc);
		std::signal(SIGINT,&SignalHandler::handler_proc);
		std::signal(SIGHUP,&SignalHandler::handler_proc);

		for(;;) {
			Socket s (accept(sock, nullptr, 0));
			if (!s) {
				if (!sig_hndl) {
					throw ErrNoException("accept failed");
				} else {
					break;
				}
			}

			std::thread thr([s = Socket(std::move(s))] ()mutable {
				Server srv(std::move(s));
				srv.run();
			});
			thr.detach();
		}
		return 0;
	} catch (std::exception &e) {
		std::cerr << "ERROR:" << e.what() << std::endl;
		return 120;
	}
}
