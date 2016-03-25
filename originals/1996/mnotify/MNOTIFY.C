//REGEN_FILEHEADING
//           // Mnotify.c - Primitives Benachrichigungssystem auf Dateibasis
// M. Lercher, Maerz 1996
//
#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <string.h>
#include <sys/stat.h>
//REGEN_FILEHEADING


     /********************************************************************
      *                                                                  *
      *   Source File: MNOTIFY.c                                         *
      *   Date:        Sat Mar 02 18:29:47 1996                          *
      *                                                                  *
      ********************************************************************/

#include <windows.h>
#include "MNOTIFY.h"


//REGEN_VARIABLES
HWND	hauptfenster;
HWND	hWndEdit, hWndCombo;
void	Timer(HWND);
char 	szFilename[256], szSeparator[256], szCommonDir[256], szNoti[256];
char	caBuf[256], caBuf1[256];
long	lTimer;
BOOL	fFirst = TRUE;
int		fAppend, fInfo, iMethod;
char    szTitle[] = "Notification Protocol";
char	szTitle1[256] = "(checking ";
void	ParseName(char *, const char *);
//REGEN_VARIABLES

long FAR PASCAL MainWndProc(HWND, unsigned, WORD, LONG);
HANDLE   hInst;

#pragma argsused
int PASCAL WinMain(HANDLE hInstance,       // Module instance handle
                   HANDLE hPrevInstance,   // Handle to previous module instance
                   LPSTR  lpszCmdLine,     // Pointer to command line arguments
                   int    nCmdShow)		   // Show window flag
{
static char szAppName[] = "MNOTIFY";
MSG      msg;
HWND     hWndMain;
WNDCLASS wndclass;

//REGEN_BEGINFUNCTION
int      res;
//REGEN_BEGINFUNCTION


   hInst = hInstance;    // Store the application instance handle 
   if(!hPrevInstance)
   {
      wndclass.style           = CS_HREDRAW | CS_VREDRAW;
	  wndclass.lpfnWndProc     = MainWndProc;
      wndclass.cbClsExtra      = 0;
      wndclass.cbWndExtra      = 0;
      wndclass.hInstance       = hInstance;
      wndclass.hCursor         = LoadCursor(NULL, IDC_ARROW);
      wndclass.hIcon           = LoadIcon(hInstance, "APPL_ICO");
	  wndclass.hbrBackground   = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
      wndclass.lpszMenuName    = "mnotify";
      wndclass.lpszClassName   = szAppName;

      if(!RegisterClass(&wndclass))
         return FALSE;
   }
   //REGEN_INITVIEW
   getcwd(caBuf1, sizeof(caBuf1));
   strcat(caBuf1, "\\");
   strcat(caBuf1, szAppName);
   strcat(caBuf1, ".ini");
   GetPrivateProfileString("main", "filename", "f:\\temp\\mnotify", caBuf, sizeof(caBuf), caBuf1);
   ParseName(szFilename, caBuf);
   strcat(szTitle1, szFilename); strcat(szTitle1, ")");
   GetPrivateProfileString("main", "common", "f:\\temp\\*.*", caBuf, sizeof(caBuf), caBuf1);
   ParseName(szCommonDir, caBuf);
   GetPrivateProfileString("main", "text", "Wichtige Nachricht", caBuf, sizeof(caBuf), caBuf1);
   ParseName(szNoti, caBuf);
   GetPrivateProfileString("main", "time", "60000", caBuf, sizeof(caBuf), caBuf1);
   ParseName(caBuf, caBuf);
   lTimer = atol(caBuf);
   GetPrivateProfileString("main", "append", "1", caBuf, sizeof(caBuf), caBuf1);
   ParseName(caBuf, caBuf);
   fAppend = atoi(caBuf);
   GetPrivateProfileString("main", "withinfo", "0", caBuf, sizeof(caBuf), caBuf1);
   ParseName(caBuf, caBuf);
   fInfo = atoi(caBuf);
   GetPrivateProfileString("main", "method", "3", caBuf, sizeof(caBuf), caBuf1);
   ParseName(caBuf, caBuf);
   iMethod = atoi(caBuf);
   GetPrivateProfileString("main", "separator", "-----", szSeparator, sizeof(szSeparator), caBuf1);
   ParseName(szSeparator, szSeparator);
   if (strlen(szSeparator))
   		strcat(szSeparator, "\r\n");
//   MessageBox(hWndMain, szSeparator, "Sep", MB_OK | MB_ICONHAND);
//   MessageBox(hWndMain, caBuf, "Time", MB_OK | MB_ICONHAND);
   //REGEN_INITVIEW

   if(!(hWndMain = CreateWindow(szAppName,
                       szTitle,
                       WS_OVERLAPPEDWINDOW,
                       CW_USEDEFAULT, 0,
                       CW_USEDEFAULT, 0,
                       NULL, NULL, hInstance, NULL)))
        return FALSE;

   //REGEN_MAINWND
   hauptfenster = hWndMain;
   res = SetTimer(hWndMain, 101, (UINT) lTimer, NULL);
   if (!res)
   {
	MessageBox(hWndMain, "Kein Timer. Ende der Funktion.", "Fehler", MB_OK | MB_ICONHAND);
	return 255;
   }
   //REGEN_MAINWND

   ShowWindow(hWndMain, nCmdShow);
   UpdateWindow(hWndMain);

   //REGEN_MAINEND
   //REGEN_MAINEND

   while(GetMessage(&msg, NULL, 0, 0))
          {
             TranslateMessage(&msg);
             DispatchMessage(&msg);
          }

   //REGEN_APPTERM
   KillTimer(hWndMain, 101);
   //REGEN_APPTERM



   return msg.wParam;
}

long FAR PASCAL MainWndProc(HWND hWnd, unsigned wMessage, WORD wParam, LONG lParam)
{
//REGEN_WINDOWPROCVARIABLES
#define TOP (24)
#define WIDTH (250)
//REGEN_WINDOWPROCVARIABLES

   switch(wMessage)
   {
       //REGEN_WINDOWPROC
	   case WM_CREATE :
		hWndEdit = CreateWindow("EDIT", NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, 0, 0, hWnd, 1, hInst, NULL);
		if (!hWndEdit)
			MessageBox(hWnd, "Protokollfenster nicht erzeugbar.", NULL, MB_OK);
		if (fInfo)
		{
			hWndCombo = CreateWindow("COMBOBOX", NULL,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL |
			CBS_SORT | CBS_AUTOHSCROLL | CBS_DROPDOWN | CBS_HASSTRINGS,
			0, 0, WIDTH, 150, hWnd, 1, hInst, NULL);
			if (!hWndCombo)
				MessageBox(hWnd, "Combobox nicht erzeugbar.", NULL, MB_OK);
			SendMessage(hWndCombo, WM_SETTEXT, 0, (LPARAM) "Dropdown for unread notifications");
        }
		Timer(hWnd); // erstmaliger Aufruf
		break;
	   case WM_SETFOCUS: SetFocus(hWndEdit); break;
	   case WM_SIZE:
			MoveWindow(hWndEdit,  0, fInfo ? TOP : 0, LOWORD(lParam), HIWORD(lParam)-(fInfo ? TOP : 0), TRUE);
			break;
       case WM_TIMER : Timer(hWnd); break;
       //REGEN_WINDOWPROC

	   case WM_COMMAND :
		  if(!LOWORD(lParam))
		  {
			 switch(wParam)             // Determine which Menu ID
			 {
			 // Process Menu Commands
			 case IDM_ABOUT :
                {
                //PROCESSMENU_ABOUT_BEGIN
				MessageBox(hWnd, "V1.3 vom "__DATE__"\r\nM. Lercher", "Martin's Notify", MB_OK | MB_ICONINFORMATION);
                //PROCESSMENU_ABOUT_END

                }
                break;
             case IDM_CLEAR :
                {
                //PROCESSMENU_CLEAR_BEGIN
				SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM) "");
				fFirst = TRUE;
                //PROCESSMENU_CLEAR_END

                }
                break;
             case IDM_EXIT :
                {
                //PROCESSMENU_EXIT_BEGIN
				PostQuitMessage(0);
                //PROCESSMENU_EXIT_END

                }
                break;
             case IDM_HELP :
                {
                //PROCESSMENU_EXIT_BEGIN
				WinHelp(hWnd, "mnotify.hlp", HELP_INDEX, 0l);
                //PROCESSMENU_EXIT_END

                }
                break;

             }
          }
          else
          {
             //REGEN_CUSTOMCOMMAND
			 switch(HIWORD(lParam))
			 {
			 case CBN_DROPDOWN:
				SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);
				SendMessage(hWndCombo, CB_DIR, 0, (LPARAM) ((LPCSTR) szCommonDir));
				break;
             case CBN_KILLFOCUS:
				SendMessage(hWndCombo, WM_SETTEXT, 0, (LPARAM) "Dropdown for unread notifications");
				break;
             }
			 //REGEN_CUSTOMCOMMAND

          }
          break;

       case WM_DESTROY :
          //REGEN_WM_DESTROY
          WinHelp(hWnd, "mnotify.hlp", HELP_QUIT, 0l);
          //REGEN_WM_DESTROY
          PostQuitMessage(0);
          break;

       default :
            return DefWindowProc(hWnd, wMessage, wParam, lParam);
   }
   return 0L;
}

//REGEN_CUSTOMCODE
void conv(unsigned long u, char *d)
{	int i=0;
	int dig[35];

	if (u == 0)
	{
		*d++ = '0';
		*d = 0;
	}
	while(u)
	{
		dig[i++] = (int) (u % 10);
		u /= 10;
	}
	while(i)
	{
		*d++ = '0'+dig[--i];
	}
	*d=0;
}

void ReadFile(void)
{	int hFile;
	OFSTRUCT OfStruct;
	struct stat FileStatus;
	HANDLE hBuffer, hBuffer1;
	LPSTR lpBuffer, lpBuffer1;
	DWORD siz;
	char *p;
	LPSTR q;

	hFile = OpenFile(szFilename, &OfStruct, OF_READ);
	if (hFile == HFILE_ERROR)
	{
		MessageBox(hauptfenster, "Datei öffnen", NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	fstat(hFile, &FileStatus);
	if (FileStatus.st_size > 0xfffeul)
	{
		MessageBox(hauptfenster, "Datei zu groß", NULL, MB_OK | MB_ICONEXCLAMATION);
		_lclose(hFile);
		return;
	}
	hBuffer = LocalAlloc(LMEM_MOVEABLE|LMEM_ZEROINIT, (WORD) FileStatus.st_size+1);
	if (!hBuffer)
	{
		MessageBox(hauptfenster, "Kein Speicher für Datei verfügbar.", NULL, MB_OK | MB_ICONEXCLAMATION);
		_lclose(hFile);
		return;
	}
	lpBuffer = LocalLock(hBuffer);
	_lread(hFile, lpBuffer, 0xfffeu);
	_lclose(hFile);
	for(p=caBuf, q=lpBuffer; *q && *q != '\n'; q++, p++)
		*p = *q;
	*p = 0;
	if (fAppend)
	{
		siz = SendMessage(hWndEdit, WM_GETTEXTLENGTH, 0, 0);
		siz += FileStatus.st_size + 1 + strlen(szSeparator);
		if (siz > 0xfffeul)
		{
			MessageBox(hauptfenster, "Überlauf.", NULL, MB_OK | MB_ICONEXCLAMATION);
			return;
		}
		hBuffer1 = LocalAlloc(LMEM_MOVEABLE|LMEM_ZEROINIT, (WORD) siz);
		if (!hBuffer1)
		{
			MessageBox(hauptfenster, "Kein Speicher verfügbar.", NULL, MB_OK | MB_ICONEXCLAMATION);
			return;
		}
		lpBuffer1 = LocalLock(hBuffer1);
		SendMessage(hWndEdit, WM_GETTEXT, 0xfffeu, (LPARAM) lpBuffer1);
		if (fFirst == FALSE)
			lstrcat(lpBuffer1, szSeparator);
		fFirst = FALSE;
		lstrcat(lpBuffer1, lpBuffer);
		SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM) lpBuffer1);
		LocalUnlock(hBuffer1);
		LocalFree(hBuffer1);
	}
	else
		SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM) lpBuffer);
	LocalUnlock(hBuffer);
	LocalFree(hBuffer);
}

void Timer(HWND hWnd)
{
	OFSTRUCT OfStruct;
	int hFile;
	static int iSema = 0;

	if (iSema)
		return;
	iSema = 1;
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM) szTitle1);
	hFile = OpenFile(szFilename, &OfStruct, OF_EXIST);
	if (hFile != HFILE_ERROR)
	{
		ReadFile();
		SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM) szTitle);
		switch(iMethod)
		{
			case 0:
				break;
			case 1:
				MessageBeep(0);
				break;
			case 2:
				MessageBox(hWnd, caBuf, szNoti, MB_OK | MB_ICONINFORMATION);
				break;
			case 3:
				MessageBox(hWnd, caBuf, szNoti, MB_OK | MB_SYSTEMMODAL | MB_ICONINFORMATION);
				break;
			default:
				break;
		}
		hFile = OpenFile(szFilename, &OfStruct, OF_DELETE);
	}
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM) szTitle);
	iSema = 0;
}

void ParseName(char *d, const char *s)
{
	char buf[256], *p, *q, *e;

	q = strcpy(buf, s);
	    
	while(*q)
    {
		if (*q == '%')
		{
			q++;
			if (! *q)
            	break;
			if (*q != '%')
			{
				p = strchr(q, '%');
				if (p)
				{
					*p = 0;
					e = getenv(strupr(q));
					if (e)
                    {
						strcat(d, e);
						d += strlen(e);
					}
					q = p + 1;
                    continue;
                }
            }
		}
		*d++ = *q++;
	}
	*d = 0;
}
//REGEN_CUSTOMCODE

