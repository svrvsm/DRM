// Hook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Hook.h"
#include <windows.h>
#include "SnippingTool.h"
#include <atlstr.h>

#pragma comment(linker, "/section:.shared,RWS")
#pragma data_seg(".shared")
HWND  gMonitorWnd = NULL;
HHOOK gKeyboardHook = NULL;
HHOOK gMouseHook = NULL;
HHOOK gGetMsgHook = NULL;
int gSnipAlert = 0;
#pragma data_seg()

#define DLLEXPORT				__declspec(dllexport) 

extern HMODULE gInstance;
//====================== Helpers ===============================
bool
IsMonitorWindowVisible()
{
	HDC wndDC = GetDC(gMonitorWnd);
	if (!wndDC)
		return false;
	
	RECT rect = {0, 0, 0, 0} ;

	// Check if the window obscured by other window.
	// Handle cases of DWM disabled. With DWM enabled
	// we use SetWindowAffinity
	int clipResult = GetClipBox(wndDC, &rect);
	if ((clipResult == NULL) || 
		(clipResult == ERROR) ||
		((rect.bottom == 0) && 
		(rect.right = 0)))
	{
		ReleaseDC(gMonitorWnd, wndDC);
		return false;
	}

	ReleaseDC(gMonitorWnd, wndDC);

	return true;
}

bool 
InstallHookHelper(HHOOK & hook, int idHook, HOOKPROC hookProc, DWORD threadId = 0)
{
	hook = SetWindowsHookEx(idHook, hookProc, gInstance, threadId);

	CString message;
	if (!hook)
	{
		message.Format(L"Hook failed. Id %d Last error %d", idHook, GetLastError());
		OutputDebugString(message);
		return false;
	}

	return true;
}

bool 
UnInstallHookHelper(HHOOK & hook, int idHook)
{
	if ((hook == NULL) || (UnhookWindowsHookEx(hook) == FALSE))
	{
		CString message;
		message.Format(L"Hook uninstall failed. Id %d last error %d", idHook, GetLastError());
		return false;
	}

	hook = NULL;

	return true;
}
//===============================================================================
LRESULT DLLEXPORT CALLBACK 
KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{

	if (code < 0)
		return CallNextHookEx(gKeyboardHook, code, wParam, lParam);	

	KBDLLHOOKSTRUCT * pkb = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
 	if (!pkb)
	{
		OutputDebugString(L"Null lParam");
		return CallNextHookEx( gKeyboardHook, code, wParam, lParam );	
	}

	if (pkb->vkCode == VK_SNAPSHOT)
	{
		if (IsMonitorWindowVisible())
		{
			OutputDebugString(L"Print screen disabled");
			// Disable print screen 
			return 1;
		}

		OutputDebugString(L"Not in the visible region. Allowing print screen."); 
	}

	LRESULT RetVal = CallNextHookEx(gKeyboardHook, code, wParam, lParam);	

	return  RetVal;
}

LRESULT DLLEXPORT CALLBACK 
MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0)
		return CallNextHookEx(gMouseHook, code, wParam, lParam);

	MSLLHOOKSTRUCT * pMsll = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
 	if (!pMsll)
	{
		OutputDebugString(L"Null lParam");
		return CallNextHookEx( gMouseHook, code, wParam, lParam );	
	}

	ResetSnipAlertIfCaptureStopped(wParam, *pMsll);

	return CallNextHookEx(gMouseHook, code, wParam, lParam);
}

LRESULT DLLEXPORT CALLBACK 
GetMessageProc(int code, WPARAM wParam, LPARAM lParam)
{
	LPMSG pMsg = reinterpret_cast<LPMSG>(lParam);

	if ((code < 0) || (!pMsg))
		return CallNextHookEx(gGetMsgHook, code, wParam, lParam);

	SetSnipAlertIfCaptureStarted(*pMsg);

	return CallNextHookEx(gMouseHook, code, wParam, lParam);
}

void DLLEXPORT CALLBACK 
SetWindowHandle(HWND wnd)
{
	gMonitorWnd = wnd;
}


DWORD DLLEXPORT CALLBACK 
InstallHook()
{
	if (!InstallHookHelper(gKeyboardHook, WH_KEYBOARD_LL, (HOOKPROC)KeyboardProc ))
	{
		return false;
	}

	if (!InstallHookHelper(gMouseHook, WH_MOUSE_LL, (HOOKPROC)MouseProc))
	{
		return false;
	}

	DWORD snipEditorThreadID = GetThreadIdForSnipEditorWnd();
	
	CString message;
	message.Format(L"Editor thread id %d", snipEditorThreadID);  
	OutputDebugString(message);

	// TODO: Watch out for shell hook window creation and install appopriately.
	// Currently the window need to be present prior to starting.
	// Window not found. Skipping install of message hook for now. 
	if (!snipEditorThreadID)
		return true; 

	if (!InstallHookHelper(gGetMsgHook, WH_GETMESSAGE, (HOOKPROC)GetMessageProc, snipEditorThreadID))
	{
		return false;
	}

	return true;
}

bool DLLEXPORT CALLBACK 
UninstallHook()
{
	bool keyboardResult = UnInstallHookHelper(gKeyboardHook, WH_KEYBOARD_LL);
	bool mouseResult  = UnInstallHookHelper(gMouseHook, WH_MOUSE_LL);
	bool getMsgResult = UnInstallHookHelper(gGetMsgHook, WH_GETMESSAGE);

	if (!keyboardResult || !mouseResult || !getMsgResult)
	{
		return false;
	}

	return true;
}
