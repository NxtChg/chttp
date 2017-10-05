/*=============================================================================
  Created by NxtChg (admin@nxtchg.com), 2017. License: Public Domain.
=============================================================================*/

#include "net_adr.h"
#include "connection.h"

class Server
{
 private:
		SOCKET s; bool wsa_inited;

		Connection con[CHTTP_MAX_CONNECTIONS]; int cur_con, Connections;

	void send_status(int status, char *txt);
	void send_response();

	int  parse_headers(chttp &in);
	bool parse_request(chttp &in);

	bool wsa_init(){ WSADATA wd; return (wsa_inited || WSAStartup(0x202, &wd) == NO_ERROR ? (wsa_inited = true) : false); }

	int  sa_sz(){ return sizeof(sockaddr_in); }

	sockaddr* sa(net_adr &adr);

	bool get_option(int opt, int &val, int level = SOL_SOCKET){ int sz = 4; return (getsockopt(s, level, opt, (      char*)&val, &sz) != SOCKET_ERROR); }
 	bool set_option(int opt, int  val, int level = SOL_SOCKET){             return (setsockopt(s, level, opt, (const char*)&val,   4) != SOCKET_ERROR); }

 	bool do_accept();
	void do_read();

 public:
		chttp_cb cb;

		static chttp ctx, tmp;

	Server(){ s = INVALID_SOCKET; wsa_inited = false; cur_con = -1; Connections = 0; }

	int  start(char *address, chttp_cb user_cb);

	void tick();
	void stop();

	~Server(){ stop(); }

} server;
//_____________________________________________________________________________

chttp Server::ctx, Server::tmp;
//_____________________________________________________________________________

void __cdecl dbg(char *msg, ...)
{
	static chttp ctx; // we better have our own context to be safe...

	if(server.cb)
	{
		va_list v; va_start(v, msg); _vsnprintf(ctx.data, CHTTP_MAX_RESPONSE-1, msg, v); va_end(v);

		ctx.type = CHTTP_LOG; ctx.size = strlen(ctx.data); *ctx.headers = 0;

		server.cb(&ctx);
	}
}//____________________________________________________________________________

void Server::send_status(int status, char *txt)
{
	if(cur_con < 0) return;

	static char headers[16*1024];

	sprintf(headers, "HTTP/1.1 %d %s\r\n\r\n", status, txt);

	con[cur_con].send(headers);
}//____________________________________________________________________________

void Server::send_response()
{
	if(cur_con < 0) return;

	char *p = tmp.data;

	p += sprintf(p, "HTTP/1.1 %d %s\r\n", ctx.status, ctx.status_txt);

	if(ctx.size >= 0){ p += sprintf(p, "Content-length: %d\r\n", ctx.size); }

	char *end = strstr(ctx.headers, "\r\n\r\n");
	
	if(!end && *ctx.headers) // user added some headers, copy
	{
		p += sprintf(p, "%s", ctx.headers); // can be extra paranoid here and check for headers length...
	}

	p += sprintf(p, "\r\n");

	int size = (p - tmp.data);

	if(ctx.size > 0 && ctx.size < CHTTP_MAX_RESPONSE)
	{
		memcpy(p, ctx.data, ctx.size); size += ctx.size;
	}

	// tmp.data[size] = 0; printf("\nSend: %d bytes, [%s]", size, tmp.data);

	con[cur_con].send(tmp.data, size);
}//____________________________________________________________________________

int Server::parse_headers(chttp &in) // < 0 = error, 0 = incomplete, > 0 = success
{
	char *src = skip_spaces(in.data, true); if(!*src) return 0;

	char *end = strstr(src, "\r\n\r\n"); if(!end) return 0;

	int size = (end - src) + 4;

	// process request line

	if(str_equal(src, "GET ",  4)){ ctx.type = CHTTP_GET;  src += 4; } else
	if(str_equal(src, "POST ", 5)){ ctx.type = CHTTP_POST; src += 5; } else{ send_status(501, "Not Implemented"); return -1; } // only GET and POST for now

	// should also check for 'HTTP/1.1' here...

	char *e = str_char(src, " ?"); if(!*e) return -1;

	int path_sz = (e - src); if(path_sz > dimof(ctx.path)-1){ send_status(414, "URI Too Long"); return -1; }

	str_copy(ctx.path, src, path_sz+1);

	// copy url params if any

	if(*e == '?')
	{
		src = e + 1; e = str_char(src, " "); if(!*e) return -1;

		int par_sz = (e - src); if(par_sz > dimof(ctx.params)-1){ send_status(414, "URI Too Long"); return -1; }

		str_copy(ctx.params, src, par_sz+1);
	}
	else *ctx.params = 0;

	src = str_char(src, "\n"); if(!*src) return -1; else src++;

	// copy headers

	int hdr_sz = (end - src) + 4; // take CRLF's with it

	if(hdr_sz > dimof(ctx.headers)-1){ send_status(431, "Request Header Fields Too Large"); return -1; }

	str_copy(ctx.headers, src, hdr_sz+1);

	// parse headers

	src = ctx.headers;

	static char line[8192];

	while(*src)
	{
		src = str_read(src, line, dimof(line), "\n"); if(!src) return -1;

		// make sure Host: is present

		if(str_equal_nc(line, "Transfer-Encoding:", strlen("Transfer-Encoding:")) ||
		   str_equal_nc(line, "Content-Length:",    strlen("Content-Length:"))      )
		{
			send_status(501, "Not Implemented"); return -1; // later will proobably need to process the body correctly for POST!
		}
	}

	return size;
}//____________________________________________________________________________

bool Server::parse_request(chttp &in)
{
	ctx.type = CHTTP_BAD;
	ctx.size = 0;
	ctx.status = 0;
	ctx.status_txt = null;

	int sz = parse_headers(in);

	if(sz <  0) return false;
	if(sz == 0) return true;

	// parse_body();

	in.size = sz;

	return true;
}//____________________________________________________________________________

sockaddr* Server::sa(net_adr &adr)
{
	static sockaddr_in sadr;

	sadr.sin_family = AF_INET;
	sadr.sin_port   = htons(adr.port);
	sadr.sin_addr.S_un.S_addr = htonl(adr.ip);
	
	return (sockaddr*)&sadr;
}//____________________________________________________________________________

int Server::start(char *address, chttp_cb user_cb)
{
	if(!address || !*address || !user_cb) return CHTTP_ERR_BAD_PARAMS;

	cb = user_cb; dbg("starting on %s...", address);

	if(s != INVALID_SOCKET) closesocket(s);

	if(!wsa_init()){ dbg("failed to init WinSock v2.2!"); return CHTTP_ERR_NET_FAILED; }

	net_adr adr; if(!adr.set(address)) adr.any(); dbg("address parsed: %s", adr.str(true));

	// create and setup main socket

	s = socket(AF_INET, SOCK_STREAM, 0); if(s == INVALID_SOCKET){ dbg("failed to create main socket!"); return CHTTP_ERR_NET_FAILED; }

	unsigned long one = 1; if(ioctlsocket(s, FIONBIO, &one) == SOCKET_ERROR){ dbg("ioctlsocket() failed!"); return CHTTP_ERR_NET_FAILED; }

	set_option(SO_REUSEADDR, 1);
	set_option(TCP_NODELAY,  1, IPPROTO_TCP); // this should be passed to connections

	// increase buffers

	if(!set_option(SO_RCVBUF, CHTTP_MAX_RESPONSE)){ dbg("failed to increase receive buffer!"); return CHTTP_ERR_NET_FAILED; }
	if(!set_option(SO_SNDBUF, CHTTP_MAX_RESPONSE)){ dbg("failed to increase send buffer!");    return CHTTP_ERR_NET_FAILED; }

	/*int sz = 0;
	
	if(get_option(SO_RCVBUF, sz)) dbg("recv buffer size = %d", sz);
	if(get_option(SO_SNDBUF, sz)) dbg("send buffer size = %d", sz);
    */

	// bind and listen

	if(bind(s, sa(adr), sa_sz()) == SOCKET_ERROR){ dbg("failed to bind main socket!"); return CHTTP_ERR_NET_FAILED; }

	if(listen(s, 5) == SOCKET_ERROR){ dbg("listen() failed with error %d!", WSAGetLastError()); return CHTTP_ERR_NET_FAILED; } // 5 backlog connections

	dbg("listening...");

	return CHTTP_OK;
}//____________________________________________________________________________

bool Server::do_accept()
{
	sockaddr_in sadr; int sz = sa_sz();

	SOCKET sok = accept(s, (sockaddr*)&sadr, &sz); // adr can be obtained via getpeername(), so no need to store it

	if(sok == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSAEWOULDBLOCK) // ERROR_IO_PENDING
		{
			dbg("accept() failed with error %d!", WSAGetLastError()); return false;
		}
	}
	else
	{
		dbg("new connection from %s:%u", inet_ntoa(sadr.sin_addr), ntohs(sadr.sin_port));

		till(Connections) // find a free connection
		{
			if(con[i].is_free()) break;
		}

		if(i == Connections && Connections < CHTTP_MAX_CONNECTIONS)
		{
			Connections++;
		}

		if(i < Connections) // found a free connection
		{
			con[i].setup(i, sok, ntohl(sadr.sin_addr.s_addr)); dbg("connection ID = %d, ip = %X", i, con[i].ip);
		}
		else // no more connections available - close it
		{
			shutdown(sok, SD_BOTH); closesocket(sok); dbg("too many connections!");
		}
	}

	return true;
}//____________________________________________________________________________

void Server::do_read()
{
	till(Connections)
	{
		if(con[i].is_free()) continue;

		tmp.size = con[i].read(tmp.data, CHTTP_MAX_RESPONSE-1); if(tmp.size < 1) continue;

		cur_con = i; // a bit of a hack...

		if(!parse_request(tmp)) con[i].disconnect(); // we don't like such requests...

		if(ctx.type != CHTTP_BAD) // parsed successfully
		{
			con[i].read(tmp.data, tmp.size, true); // remove request from buffer

			if(cb)
			{
            	ctx.ip = con[i].ip; cb(&ctx);

            	if(ctx.status > 0 && ctx.status_txt) // need to send a reponse
            	{
            		send_response();
            	}
            	else con[i].disconnect(); // Let's assume all requests must be responded to...
			}
		}

		cur_con = -1;
	}
}//____________________________________________________________________________

void Server::tick()
{
	if(s ==	INVALID_SOCKET) return;

	//do_timeout();

	if(!do_accept()){ } // need to handle errors here probably and notify the host somehow...

	do_read();
}//____________________________________________________________________________

void Server::stop()
{
	dbg("stopping..."); cb = null;

	till(Connections){ con[i].close(); } Connections = 0;

    if(s != INVALID_SOCKET){ closesocket(s); s = INVALID_SOCKET; }

	if(wsa_inited){ WSACleanup(); wsa_inited = false; }
}
