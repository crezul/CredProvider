//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//


#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
# pragma comment(lib, "wbemuuid.lib")
# pragma comment(lib, "credui.lib")
# pragma comment(lib, "comsuppw.lib")
#include <wincred.h>
#include <strsafe.h>

#include "CommandWindow.h"
#include <strsafe.h>

#include "consts.h"

// Custom messages for managing the behavior of the window thread.
#define WM_EXIT_THREAD              WM_USER + 1
#define WM_TOGGLE_CONNECTED_STATUS  WM_USER + 2
#define WM_DATA_USER_IS_ACCEPT		WM_USER + 8

HWND hWnd_;

const WCHAR c_szClassName[] = L"cheaterWindow";

CCommandWindow::CCommandWindow() : _hWnd(NULL), _hInst(NULL), _fConnected(FALSE), _pProvider(NULL)
{
	
}

CCommandWindow::~CCommandWindow()
{
	addLogMessage("~CCommandWindow()");
    if (_hWnd != NULL)
    {
        PostMessage(_hWnd, WM_EXIT_THREAD, 0, 0);
        _hWnd = NULL;
    }

    if (_pProvider != NULL)
    {
        _pProvider->Release();
        _pProvider = NULL;
    }

   
    CoUninitialize();
}

HRESULT CCommandWindow::Initialize(__in CSampleProvider *pProvider)
{
    HRESULT hr = S_OK;
	addLogMessage("CommandWindow : INITALIZE");
    if (_pProvider != NULL)
    {
        _pProvider->Release();
    }
    _pProvider = pProvider;
    _pProvider->AddRef();
    
    HANDLE hThread = CreateThread(NULL, 0, _ThreadProc, this, 0, NULL);
	

	if (hThread == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

// Wraps our internal connected status so callers can easily access it.
BOOL CCommandWindow::GetConnectedStatus()
{
    return _fConnected;
}

//
//  FUNCTION: _MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
HRESULT CCommandWindow::_MyRegisterClass()
{
	addLogMessage("_MyRegisterClass");
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc      = _WndProc;
    wcex.hInstance        = _hInst;
    wcex.hIcon            = NULL;
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName    = c_szClassName;

    return RegisterClassEx(&wcex) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

//
//   FUNCTION: _InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HRESULT CCommandWindow::_InitInstance()
{
	addLogMessage("InitInstance");
    HRESULT hr = S_OK;

    _hWnd = CreateWindowEx(
        WS_EX_TOPMOST, 
        c_szClassName, 
		L"", 
        WS_DLGFRAME,
        200, 200, 200, 80, 
        NULL,
        NULL, _hInst, NULL);

	
    if (_hWnd == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
	//TCHAR textBuffer[128];
	//wsprintf(textBuffer, L"Descriptiof of Window: 0x%08X", _hWnd);
	//MessageBox(0, textBuffer, L"INFO", MB_OK);
	hWnd_ = _hWnd;
    if (SUCCEEDED(hr))
    {
		if (!ShowWindow(_hWnd, SW_HIDE))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

            if (SUCCEEDED(hr))
            {
                if (!UpdateWindow(_hWnd))
                {
                   hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
    }

    return hr;
}

BOOL CCommandWindow::_ProcessNextMessage()
{
	addLogMessage("_ProccesNextMessage()");
    MSG msg;
    GetMessage(&(msg), _hWnd, 0, 0);
    TranslateMessage(&(msg));
    DispatchMessage(&(msg));

    switch (msg.message)
    {
    case WM_EXIT_THREAD: return FALSE;
    case WM_TOGGLE_CONNECTED_STATUS:
		addLogMessage("WM_TOGGLE_CONNECTED_STATUS");
		_fConnected=true;
		_pProvider->OnConnectStatusChanged();
		break;
	}
    return TRUE;

}

LRESULT CALLBACK CCommandWindow::_WndProc(__in HWND hWnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam)
{
	addLogMessage("_WinProc");
    switch (message)
    {
    case WM_DATA_USER_IS_ACCEPT:
	{
		addLogMessage("WM_DATA_USER_IS_ACCEPT is start!");
		PostMessage(hWnd, WM_TOGGLE_CONNECTED_STATUS, 0, 0);

		break;
	}
    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
		addLogMessage("WM_CLOSE is start!");
        PostMessage(hWnd, WM_EXIT_THREAD, 0, 0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

DWORD WINAPI CCommandWindow::_ThreadProc(__in LPVOID lpParameter)
{
	addLogMessage("_ThreadProc is starting!");

	HRESULT hr = S_OK;
	CCommandWindow *pCommandWindow = static_cast<CCommandWindow *>(lpParameter);
    
	if (pCommandWindow == NULL)
    {
        return 0;
    }

	HANDLE hPipe = CreateThread(NULL, 0, _ThreadPipe, 0, 0, NULL);


	if (hPipe == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
    // Create the window.
    pCommandWindow->_hInst = GetModuleHandle(NULL);

	
    if (pCommandWindow->_hInst != NULL)
    {            
        hr = pCommandWindow->_MyRegisterClass();
        if (SUCCEEDED(hr))
        {
            hr = pCommandWindow->_InitInstance();
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
	}

	ShowWindow(pCommandWindow->_hWnd, SW_HIDE);

    if (SUCCEEDED(hr))
    {        
        while (pCommandWindow->_ProcessNextMessage()) 
        {
        }
    }
    else
    {
        if (pCommandWindow->_hWnd != NULL)
        {
            pCommandWindow->_hWnd = NULL;
        }
    }

    return 0;
}
DWORD WINAPI CCommandWindow::_ThreadPipe(__in LPVOID lpParameter)
{

	addLogMessage("_ThreadPipe is starting!");

	HANDLE hPipe = INVALID_HANDLE_VALUE;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeforcp");

	while (hPipe == INVALID_HANDLE_VALUE)
	{
		hPipe = CreateFile(
			lpszPipename,             // pipe name 
			GENERIC_READ | GENERIC_WRITE,                // blocking mode 
			0,							// max. instances  == listen_socket
			NULL,                  // output buffer size 
			OPEN_EXISTING,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 
		Sleep(1000);
	}
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, L"ERROR OF OPEN NAMEPIPE", L"PIPE_ERROR", MB_OK);
		return 1;
	}

	DWORD pmod = PIPE_READMODE_MESSAGE; //mode of pipe

	BOOL 	bSuccess = 1;

	bSuccess = SetNamedPipeHandleState(hPipe, &pmod, NULL, NULL);
	if (!bSuccess)
	{
		MessageBox(NULL, L"ERROR OF SET MODE READ", L"ERROR", MB_OK);
		return -1;
	}
	//bSuccess = 1;
	char bufferRequest[255] = ""; // буфер для принятия ответа
	DWORD buffsize = 255; // размер максимально принятих данніх
	DWORD buffersizereaden = 0; // принятых
								//login
	bSuccess = ReadFile(
		hPipe,        // handle to pipe 
		bufferRequest,    // buffer to receive data 
		buffsize, // size of buffer 
		&buffersizereaden, // number of bytes read 
		NULL);        // not overlapped I/O 
	if (bSuccess)
		MessageBoxA(NULL, bufferRequest, "DATA_RECIEV", MB_OK);
	else
		MessageBoxA(NULL, "ERROR READ_PIPE", "ERROR", MB_OK);
	addLogMessage("Message of pipe accept:"); addLogMessage(bufferRequest);
	//обработка в юникод
	wchar_t* wbuflogin = new wchar_t[strlen(bufferRequest)];			//!
	MultiByteToWideChar(CP_UTF8, 0, bufferRequest, -1, wbuflogin, 255);
	USERNAME = wbuflogin;
	//MessageBox(NULL, USERNAME, L"USERNAME UNITCODE", MB_OK);
	buffersizereaden = 0;
	//pass
	bSuccess = ReadFile(
		hPipe,        // handle to pipe 
		bufferRequest,    // buffer to receive data 
		buffsize, // size of buffer 
		&buffersizereaden, // number of bytes read 
		NULL);        // not overlapped I/O 
	if (bSuccess)
		MessageBoxA(NULL, bufferRequest, "DATA_RECIEV", MB_OK);
	else
		MessageBoxA(NULL, "ERROR READ_PIPE", "ERROR", MB_OK);
	addLogMessage("Message of pipe accept:"); addLogMessage(bufferRequest);
	wchar_t* wbufpass = new wchar_t[strlen(bufferRequest)];			//!
	MultiByteToWideChar(CP_UTF8, 0, bufferRequest, -1, wbufpass, 255);
	PASSWORD = wbufpass;
	//MessageBox(NULL, PASSWORD, L"PASSWORD UNITCODE", MB_OK);
	DisconnectNamedPipe(hPipe);
	FlushFileBuffers(hPipe);
	CloseHandle(hPipe);
	/*TCHAR textBuffer[128];
	wsprintf(textBuffer, L"Descriptiof of Window2: 0x%08X", hWnd_);
	MessageBox(0, textBuffer, L"INFO", MB_OK);*/
	PostMessage(hWnd_, WM_DATA_USER_IS_ACCEPT, 0, 0); // можно передать локально как в предыдущем потоке обьект комманд
	addLogMessage("WM_DATA_USER_IS_ACCEPT signal");

	addLogMessage("Pipe is close");
	return 0;
}
