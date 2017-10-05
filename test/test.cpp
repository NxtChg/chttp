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

	if(!stricmp(ext, ".html") == 0 || !stricmp(ext, ".htm" ) == 0) return "text/html";
	if(!stricmp(ext, ".jpg")  == 0 || !stricmp(ext, ".jpeg") == 0) return "image/jpeg";
	if(!stricmp(ext, ".gif")  == 0) return "image/gif";
	if(!stricmp(ext, ".png")  == 0) return "image/png";
	if(!stricmp(ext, ".ico")  == 0) return "image/x-icon";
	if(!stricmp(ext, ".txt")  == 0) return "text/plain";
	if(!stricmp(ext, ".csv")  == 0) return "text/csv";
	if(!stricmp(ext, ".css")  == 0) return "text/css";
	if(!stricmp(ext, ".js")   == 0) return "text/javascript";	// "application/javascript" is the correct type, but 'text' is more compatible...
	if(!stricmp(ext, ".json") == 0) return "application/json";
	if(!stricmp(ext, ".xml")  == 0) return "text/xml";			// again, should be "application/xml"
	if(!stricmp(ext, ".au")   == 0) return "audio/basic";
	if(!stricmp(ext, ".wav")  == 0) return "audio/wav";
	if(!stricmp(ext, ".mp3")  == 0) return "audio/mpeg";
	if(!stricmp(ext, ".ogg")  == 0) return "audio/ogg";
	if(!stricmp(ext, ".otf")  == 0) return "font/otf";
	if(!stricmp(ext, ".ttf")  == 0) return "font/ttf";
	if(!stricmp(ext, ".woff") == 0) return "font/woff";
	if(!stricmp(ext, ".woff2")== 0) return "font/woff2";
	if(!stricmp(ext, ".avi")  == 0) return "video/x-msvideo";
	if(!stricmp(ext, ".mpeg") == 0 || !stricmp(ext, ".mpg") == 0) return "video/mpeg";

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
