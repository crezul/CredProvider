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
wchar_t* wbuflogin;
wchar_t* wbufpass;

const WCHAR c_szClassName[] = L"Anroid Credential Provider";

CCommandWindow::CCommandWindow() : _hWnd(NULL), _hInst(NULL), _fConnected(FALSE), _pProvider(NULL) , _pCredential(NULL)
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
	if (_pCredential != NULL)
	{
		_pCredential->Release();
		_pCredential = NULL;
	}
	if (wbuflogin != NULL && wbufpass != NULL)
	{
		delete wbuflogin;
		delete wbufpass; // clear reference
	}
    CoUninitialize();
}

HRESULT CCommandWindow::Initialize(__in CSampleProvider *pProvider, __in CSampleCredential *pCredential)
{
    HRESULT hr = S_OK;
	addLogMessage("CommandWindow : INITALIZE");
    if (_pProvider != NULL)
    {
        _pProvider->Release();
    }
	_pProvider = pProvider;
	_pProvider->AddRef();
	

	if (_pCredential != NULL)
	{
		_pCredential->Release();
	}
	_pCredential = pCredential;
	_pCredential->AddRef();

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
		addLogMessage("ERROR:_hWnd = NULL");
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
	
	hWnd_ = _hWnd;


            if (SUCCEEDED(hr))
            {
                if (!UpdateWindow(_hWnd))
                {
					addLogMessage("ERROR:Update wondow");
                   hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }

    return hr;
}

BOOL CCommandWindow::_ProcessNextMessage()
{
	addLogMessage("\nStart _ProccesNextMessage() - it's cycle in:_ThreadProc method. \n");
    MSG msg;
    GetMessage(&(msg), _hWnd, 0, 0);
    TranslateMessage(&(msg));
    DispatchMessage(&(msg));

    switch (msg.message)
    {
    case WM_EXIT_THREAD: return FALSE;
    case WM_TOGGLE_CONNECTED_STATUS:
		addLogMessage("WM_TOGGLE_CONNECTED_STATUS signal is accept");
		_pCredential->InitializeToSignal(wbuflogin, wbufpass); // delete in this method
		_fConnected=true;
		_pProvider->OnConnectStatusChanged();
		break;
	}
    return TRUE;

}

LRESULT CALLBACK CCommandWindow::_WndProc(__in HWND hWnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam)
{
	addLogMessage("_WinProc - The method sends a signal to the window that there is data-credentials are changed ");
    switch (message)
    {
    case WM_DATA_USER_IS_ACCEPT:
	{
		addLogMessage("WM_DATA_USER_IS_ACCEPT in _WinPeoc method call WM_TOGGLE_CONNECTED_STATUS");
		PostMessage(hWnd, WM_TOGGLE_CONNECTED_STATUS, 0, 0);

		break;
	}
    case WM_CLOSE:
		addLogMessage("WM_CLOSE is start!");
        PostMessage(hWnd, WM_EXIT_THREAD, 0, 0);
        break;
	case WM_DESTROY:
		addLogMessage("WM_DESTROY is start!");
		break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

DWORD WINAPI CCommandWindow::_ThreadProc(__in LPVOID lpParameter)
{
	addLogMessage("\n_ThreadProc method is starting (it's main method-theard in CommandWindow class)\n");

	HRESULT hr = S_OK;
	CCommandWindow *pCommandWindow = static_cast<CCommandWindow *>(lpParameter);
    
	if (pCommandWindow == NULL)
    {
		addLogMessage("ERROR:pCommandWindow == NULL");
        return -1;
    }

    // Create the window.
    pCommandWindow->_hInst = GetModuleHandle(NULL);

	
    if (pCommandWindow->_hInst != NULL)
    {            
        hr = pCommandWindow->_MyRegisterClass();
        if (SUCCEEDED(hr))
        {
            hr = pCommandWindow->_InitInstance();
			if (!SUCCEEDED(hr))
				addLogMessage("ERROR of INITINSTANCE");
        }
		else
		{
			addLogMessage("ERROR of  MYREGISTRCLASS() in THREADPROC");
		}
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		addLogMessage("ERROR _hInst = NULL");
		return -1;
	}

	ShowWindow(pCommandWindow->_hWnd, SW_HIDE);


    if (SUCCEEDED(hr))
    {
		HANDLE hPipe = CreateThread(NULL, 0,  _ThreadPipe, 0, 0, NULL);

		if (hPipe == NULL)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			addLogMessage("ERROR: hPipe return error");
			return -1;
		}

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
	addLogMessage("\nEnd _ThreadProc method.\n");
    return 0;
}
DWORD WINAPI CCommandWindow::_ThreadPipe(__in LPVOID lpParameter)
{

	addLogMessage("\n_ThreadPipe() theard is starting!");

	HANDLE hPipe = INVALID_HANDLE_VALUE;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeforcp");

	while (hPipe == INVALID_HANDLE_VALUE) // wait to connect
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
	else
	{
		addLogMessage("Pipe is connect");
	}

	DWORD pmod = PIPE_READMODE_MESSAGE; //mode of pipe

	BOOL 	bSuccess = 1;

	bSuccess = SetNamedPipeHandleState(hPipe, &pmod, NULL, NULL);
	if (!bSuccess)
	{
		MessageBox(NULL, L"ERROR OF SET MODE READ", L"ERROR", MB_OK);
		return -1;
	}
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
	// 
	addLogMessage("Message of pipe accept:"); addLogMessage(bufferRequest);
	
	 wbuflogin = new wchar_t[strlen(bufferRequest)];		

	MultiByteToWideChar(CP_UTF8, 0, bufferRequest, -1, wbuflogin, buffsize);
	
	buffersizereaden = 0;
	//pass
	bSuccess = ReadFile(
		hPipe,        // handle to pipe 
		bufferRequest,    // buffer to receive data 
		buffsize, // size of buffer 
		&buffersizereaden, // number of bytes read 
		NULL);        // not overlapped I/O 
	
	addLogMessage("Message of pipe accept:");
	addLogMessage(bufferRequest);

	wbufpass = new wchar_t[strlen(bufferRequest)];			

	 MultiByteToWideChar(CP_UTF8, 0, bufferRequest, -1, wbufpass, buffsize);

	
//	 MessageBox(NULL, wbufpass, L"wbufpass UNITCODE in therd pipe", MB_OK);

	DisconnectNamedPipe(hPipe);
	FlushFileBuffers(hPipe);
	CloseHandle(hPipe);

	addLogMessage("Pipe is close");

	PostMessage(hWnd_, WM_DATA_USER_IS_ACCEPT, 0, 0); 
	
	addLogMessage("_ThreadPipe::WM_DATA_USER_IS_ACCEPT signal send to _WinProc method.");
	addLogMessage("Theard _ThreadPipe is stop working\n");
	return 0;
}

