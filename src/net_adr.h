/*=============================================================================
  Created by NxtChg (admin@nxtchg.com), 2017. License: Public Domain.
=============================================================================*/
/*
	We rely here on INADDR_ANY == 0 and INADDR_NONE == 0xFFFFFFFF.
*/

class net_adr
{
 private:
		class  IP_Range{ public: uint start, end; bool owns(int ip){ return ((uint)ip >= start && (uint)ip <= end); } };

		static IP_Range local_ips[];

 public:
		int ip; ushort port;

	net_adr(int port = 0){ any(); this->port = (ushort)port; }

	net_adr(char *ip_str, int port = 0){ set(ip_str, port); }

	void any(){ ip = 0; }

	bool ok(){ return (ip != -1); } // and (p != 0)?

	bool valid(){ return (ok() && ip != 0 && port > 0 && !is_local()); }

	bool is_local();

	bool set(char *ip_str, int def_port = 0);

	char* str(bool with_port = false, char dst[32] = null);

	bool operator == (net_adr &a){ return (ip == a.ip && port == a.port); }
	bool operator != (net_adr &a){ return (ip != a.ip || port != a.port); }
};//___________________________________________________________________________

net_adr::IP_Range net_adr::local_ips[] =
{
	{ 0x00000000, 0x00FFFFFF }, //     0.0.0.0 - 0.255.255.255
	{ 0x0A000000, 0x0AFFFFFF }, //    10.0.0.0 - 10.255.255.255
	{ 0x64400000, 0x647FFFFF }, //  100.64.0.0 - 100.127.255.255
	{ 0x7F000000, 0x7FFFFFFF }, //   127.0.0.0 - 127.255.255.255
	{ 0xA9FE0000, 0xA9FEFFFF }, // 169.254.0.0 - 169.254.255.255
	{ 0xAC100000, 0xAC1FFFFF }, //  172.16.0.0 - 172.31.255.255
	{ 0xC0000000, 0xC0000007 }, //   192.0.0.0 - 192.0.0.7
	{ 0xC0000200, 0xC00002FF }, //   192.0.2.0 - 192.0.2.255
	{ 0xC0586300, 0xC05863FF }, // 192.88.99.0 - 192.88.99.255
	{ 0xC0A80000, 0xC0A8FFFF }, // 192.168.0.0 - 192.168.255.255
	{ 0xC6120000, 0xC613FFFF }, //  198.18.0.0 - 198.19.255.255
	{ 0xC6336400, 0xC63364FF }, //198.51.100.0 - 198.51.100.255
	{ 0xCB007100, 0xCB0071FF }, // 203.0.113.0 - 203.0.113.255
	{ 0xE0000000, 0xEFFFFFFF }, //   224.0.0.0 - 239.255.255.255
	{ 0xF0000000, 0xFFFFFFFE }, //   240.0.0.0 - 255.255.255.254
	{ 0xFFFFFFFF, 0xFFFFFFFF }, //      255.255.255.255
};//___________________________________________________________________________

bool net_adr::is_local()
{
	till(dimof(local_ips)){ if(local_ips[i].owns(ip)) return true; }

	return false;
}//____________________________________________________________________________

bool net_adr::set(char *ip_str, int def_port)
{
	char *v = ip_str, tmp[8];
	
	int t = 0; ip = -1; port = def_port;

	till(4)
	{
		v = str_read(v, tmp, 8, (i < 3 ? "." : ":")); if(!v) return false;

		int b = atoi(tmp); if((uint)b < 256){ t |= b << (24 - i * 8); } else return false;
	}

	ip = t; if(v){ v = str_read(v, tmp, 8); if(v){ t = atoi(tmp); if(t >= 0 && t < 0x10000) port = t; } }

	return ok();
}//____________________________________________________________________________

char* net_adr::str(bool with_port, char dst[32])
{
	static char out[32];
	
	byte *i = (byte*)&ip; if(!dst) dst = out;

	sprintf(dst, (with_port ? "%u.%u.%u.%u:%u" : "%u.%u.%u.%u"), i[3], i[2], i[1], i[0], port);

	return dst;
}
