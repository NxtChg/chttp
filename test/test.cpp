/*=============================================================================
  Created by NxtChg (admin@nxtchg.com), 2017. License: Public Domain.
=============================================================================*/

#include <stdio.h>
#include <windows.h>

#include "..\include\chttp.h"

//#pragma comment(lib, "chttp.lib")

volatile bool quit = false;

BOOL __stdcall on_exit(DWORD type)
{
	switch(type)
	{
		case CTRL_CLOSE_EVENT:    printf("\nExiting...");               quit = true; break;
		case CTRL_C_EVENT:        printf("\nExiting by Ctrl-C...");     quit = true; break;
		case CTRL_BREAK_EVENT:    printf("\nExiting by Ctrl-Break..."); quit = true; break;
		case CTRL_LOGOFF_EVENT:   printf("\nLogoff event detected." );               break;
		case CTRL_SHUTDOWN_EVENT: printf("\nExiting by shutdown...");   quit = true; break;
	}

	return TRUE;
}//____________________________________________________________________________

char* get_mime(char *name)
{
	char *ext = strrchr(name, '.'); if(!ext) return NULL;

	if(!stricmp(ext, ".html"  ) || !stricmp(ext, ".htm" )) return "text/html";
	if(!stricmp(ext, ".jpg"   ) || !stricmp(ext, ".jpeg")) return "image/jpeg";
	if(!stricmp(ext, ".gif"  )) return "image/gif";
	if(!stricmp(ext, ".png"  )) return "image/png";
	if(!stricmp(ext, ".ico"  )) return "image/x-icon";
	if(!stricmp(ext, ".txt"  )) return "text/plain";
	if(!stricmp(ext, ".csv"  )) return "text/csv";
	if(!stricmp(ext, ".css"  )) return "text/css";
	if(!stricmp(ext, ".js"   )) return "text/javascript";	// "application/javascript" is the correct type, but 'text' is more compatible...
	if(!stricmp(ext, ".json" )) return "application/json";
	if(!stricmp(ext, ".xml"  )) return "text/xml";			// again, should be "application/xml"
	if(!stricmp(ext, ".au"   )) return "audio/basic";
	if(!stricmp(ext, ".wav"  )) return "audio/wav";
	if(!stricmp(ext, ".mp3"  )) return "audio/mpeg";
	if(!stricmp(ext, ".ogg"  )) return "audio/ogg";
	if(!stricmp(ext, ".otf"  )) return "font/otf";
	if(!stricmp(ext, ".ttf"  )) return "font/ttf";
	if(!stricmp(ext, ".woff" )) return "font/woff";
	if(!stricmp(ext, ".woff2")) return "font/woff2";
	if(!stricmp(ext, ".avi"  )) return "video/x-msvideo";
	if(!stricmp(ext, ".mpeg" ) || !stricmp(ext, ".mpg")) return "video/mpeg";

	return NULL;
}//____________________________________________________________________________

void send_file(chttp &ctx, char *filename)
{
	static char fname[MAX_PATH];

	sprintf(fname, "www\\%s", filename);

	FILE *f = fopen(fname, "rb");
	
	if(f)
	{
		int sz = fread(ctx.data, 1, CHTTP_MAX_RESPONSE, f);

		if(sz > 0)
		{
			ctx.status = 200; ctx.status_txt = "OK"; ctx.size = sz;

			char *mime = get_mime(filename);

			if(mime){ sprintf(ctx.headers, "Content-Type: %s\r\n", mime); }

			return;
		}
	}

	ctx.status = 404; ctx.status_txt = "Not Found";

	sprintf(ctx.headers, "Content-Type: text/html\r\n");
	sprintf(ctx.data, "<html><body><h2>404: Not Found</h2></body></html>");
	
	ctx.size = strlen(ctx.data);
}//____________________________________________________________________________

int chttp_event(chttp *ctx)
{
	chttp &c = *ctx;

	switch(c.type)
	{
		case CHTTP_LOG: printf("\nchttp: %s", c.data); break;

		case CHTTP_GET:
		{
			printf("\nchttp: GET \"%s\", params: [%s], headers: [%s]", c.path, c.params, c.headers);
		  
			char *p = c.path; if(*p == '/') p++; if(!*p) p = "index.html";

			send_file(c, p);
		}
		break;

		case CHTTP_POST:
		{
			printf("\nchttp: POST \"%s\", params: [%s], headers: [%s]", c.path, c.params, c.headers);
		}
		break;
	}

	return 0;
}//____________________________________________________________________________

void __cdecl main(int argc, char **argv)
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleCtrlHandler(on_exit, TRUE);

	if(!h || h == INVALID_HANDLE_VALUE){ printf("\nFailed to set Ctrl-C handler!"); return; }

	//----

	char *address = "127.0.0.1:80";

	if(argc > 1) address = argv[1];

	int err = chttp_start(address, chttp_event);

	if(err){ printf("\nFailed to start the server! Error code: %d", err); return; }

	//----

	while(!quit)
	{
		chttp_tick(); // would be nice to pass time here so the code could sleep the rest of it...

		Sleep(5);
	}

	chttp_stop();

	printf("\n--\nBye!\n");
}
