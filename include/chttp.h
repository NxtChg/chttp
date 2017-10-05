/*=============================================================================
  Created by NxtChg (admin@nxtchg.com), 2017. License: Public Domain.
=============================================================================*/

enum CHTTP_ERRORS
{
	CHTTP_OK,
	CHTTP_ERR_BAD_PARAMS,
	CHTTP_ERR_NET_FAILED,
};
//_____________________________________________________________________________

enum CHTTP_TYPES { CHTTP_BAD, CHTTP_LOG, CHTTP_GET, CHTTP_POST };

enum CHTTP_PARAMS{ CHTTP_MAX_RESPONSE = 256*1024, CHTTP_MAX_CONNECTIONS = 100 }; // 256 Kb x 2 x 100 = 51 Mb max memory footprint

#pragma pack(push,4)

struct chttp
{
	int  type, size; unsigned int ip;

	int  status; char *status_txt; // set these to (200, "OK") to send a response or use any other error status, and don't forget 'size'!

	char path   [8*1024];
	char params [8*1024];
	char headers[8*1024];

	char data[CHTTP_MAX_RESPONSE + 16*1024];
};

#pragma pack(pop)
//_____________________________________________________________________________

#define CHTTP_API(ret) extern "C" ret __stdcall

typedef int (*chttp_cb)(chttp *event);

CHTTP_API(int)  chttp_start(char *address, chttp_cb cb);
CHTTP_API(void) chttp_tick();
CHTTP_API(void) chttp_stop();
//_____________________________________________________________________________
