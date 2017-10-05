/*=============================================================================
  Created by NxtChg (admin@nxtchg.com), 2017. License: Public Domain.
=============================================================================*/

#include "include/chttp.h"

#include "src/common.h"
#include "src/server.h"

#pragma comment(linker, "/comment:\" [built on " __DATE__ " at " __TIME__ " by NxtChg] \"")
//_____________________________________________________________________________

CHTTP_API(int)  chttp_start(char *address, chttp_cb cb){ return server.start(address, cb); }
CHTTP_API(void) chttp_tick(){ server.tick(); }
CHTTP_API(void) chttp_stop(){ server.stop(); }
//_____________________________________________________________________________

BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID reserved)
{
	reserved = 0; // make compiler happy

	switch(reason)
	{
		case DLL_PROCESS_ATTACH: DisableThreadLibraryCalls(h); break;
		case DLL_PROCESS_DETACH:                               break;
	}
	return TRUE;
}
