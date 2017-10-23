/*=============================================================================
  Created by NxtChg (admin@nxtchg.com), 2017. License: Public Domain.
=============================================================================*/

class Connection
{
 public:
 		int state; enum{ FREE, CONNECTED, CLOSING };

		SOCKET s; int id; uint ip, last_req;

	Connection(){ state = FREE; }
	
	void setup(int id, SOCKET s, uint ip);

//	int  age();

	bool is_ok  (){ return (state == CONNECTED); }
	bool is_free(){ return (state == FREE); }
//	bool timeout(){ return (state == CONNECTED && age() > 4); }

	bool send(char *buf, int size = 0);
	int  read(char *buf, int limit, bool extract = false);

	void disconnect(){ if( is_ok  ()){ shutdown(s, SD_SEND);                 state = CLOSING; } }
	void      close(){ if(!is_free()){ shutdown(s, SD_BOTH); closesocket(s); state = FREE;    } }
};//___________________________________________________________________________

void Connection::setup(int id, SOCKET s, uint ip)
{
	state = CONNECTED; this->id = id; this->s = s; this->ip = ip;

	this->last_req = time(null);
}//____________________________________________________________________________

bool Connection::send(char *buf, int size)
{
	if(state != CONNECTED) return false;

	if(size < 1) size = strlen(buf);

	int sz = ::send(s, buf, size, 0); if(sz == SOCKET_ERROR){ disconnect(); return false; }

	return true;
}//____________________________________________________________________________

int Connection::read(char *buf, int limit, bool extract)
{
	int sz = ::recv(s, buf, limit, (extract ? 0 : MSG_PEEK));
	
	if(sz == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSAEWOULDBLOCK) disconnect(); // error
		
		return 0; // nothing received
	}

	if(sz == 0){ close(); return 0; } // connection closed

	buf[sz] = 0; last_req = time(null);

	return sz;
}
