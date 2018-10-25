#include "stdafx.h"
#include "SnippingTool.h"
#include "Hook.h"
#include <atlstr.h>

#define SNIP_TOOLBAR_PARENT_WND_CLASS_NAME		L"Microsoft-Windows-Tablet-SnipperToolbar"
#define SNIP_EDITOR_WND_CLASS_NAME				L"Microsoft-Windows-Tablet-SnipperEditor"
#define SNIP_TOOLBAR_CLASS_NAME					L"ToolbarWindow32"

#define NEW_BUTTON_WIDTH						66
#define NEW_BUTTON_HEIGHT						36

#define TOOLBAR_BUTTON_NEW_MSG_ID				WM_USER + 1280
#define FILE_NEWSNIP_COMMAND_ID					40011
#define SNIP_TOOLBAR_NEW_BUTTON_ID				219				

extern int gSnipAlert;
extern HWND gMonitorWnd;

//==========================Helper functions===========================

DWORD
GetThreadIdForWindow(LPCWSTR className)
{
	HWND wnd = FindWindow(className, NULL);

	DWORD mainThread = 0;

	if (wnd)
		mainThread = GetWindowThreadProcessId(wnd, NULL);

	return mainThread;

}

DWORD GetThreadIdForSnipEditorWnd()
{
	return GetThreadIdForWindow(SNIP_EDITOR_WND_CLASS_NAME);
}

bool
IsSnipToolNewButtonClicked(HWND snipWnd, POINT pt)
{
	RECT rect;
	if (!GetWindowRect(snipWnd, &rect))
	{
		OutputDebugString(L"GetWindowRect false");
		return false;
	}
	
	// check whether the point lies in the rect.
	RECT newButtonRect;

	newButtonRect.left = rect.left;
	newButtonRect.top = rect.top;
	newButtonRect.right = rect.left + NEW_BUTTON_WIDTH;
	newButtonRect.bottom = rect.top + NEW_BUTTON_HEIGHT;

	CString message;
	message.Format(L"Pt.x %d Pt.y %d TB.left %d TB.top %d TB.right %d TB.bottom %d \
					 NewBtn.left %d NewBtn.top %d NewBtn.right %d NewBtn.bottom %d",  
					 pt.x, pt.y, rect.left, rect.top, rect.right, rect.bottom,
					 newButtonRect.left, newButtonRect.top, newButtonRect.right, newButtonRect.bottom);
	OutputDebugString(message);

	// check if the point is within the new boundary
	if ((pt.x >= newButtonRect.left) && 
		(pt.x <= newButtonRect.right) &&
		(pt.y >= newButtonRect.top) &&
		(pt.y <= newButtonRect.bottom))
	{
		return true;
	}

	return false;
}

bool
IsPointInSnipWindow(POINT & pt)
{
	HWND ptWindow = WindowFromPoint(pt);
	if (!ptWindow)
	{
		OutputDebugString(L"Point window null");
		return false;
	}

	// Checks for point in
	// 1. toolbar window
	// 2. if not present check toolbar window in editor window
	// 3. If any of them true, check new button got clicked.

	CString message;
	

	HWND snipToolbarParentWnd = FindWindow(SNIP_TOOLBAR_PARENT_WND_CLASS_NAME, NULL);
	HWND snipActualToolbarWnd = NULL;
	if (snipToolbarParentWnd)
	{
		snipActualToolbarWnd = FindWindowEx(snipToolbarParentWnd, NULL, SNIP_TOOLBAR_CLASS_NAME, NULL);
	
		if (ptWindow == snipActualToolbarWnd)
		{
			bool result = IsSnipToolNewButtonClicked(snipActualToolbarWnd, pt);
			message.Format(L"Snip Alert %d %s", gSnipAlert, result ? L"Yay!!! Toolbar new button hit in toolbar UI" : 
							L"Point outside the new button area");
			OutputDebugString(message);
			return result;
		}
		snipActualToolbarWnd = NULL;
	}

	HWND snipToolbarEditorWnd = FindWindow(SNIP_EDITOR_WND_CLASS_NAME, NULL);

	snipActualToolbarWnd = FindWindowEx(snipToolbarEditorWnd, NULL, SNIP_TOOLBAR_CLASS_NAME, NULL);
	if (ptWindow == snipActualToolbarWnd)
	{
		bool result = IsSnipToolNewButtonClicked(snipActualToolbarWnd, pt);
		message.Format(L"Snip Alert %d %s", gSnipAlert, result ? L"Yay!!! Toolbar new button hit in editor" : 
						L"Point outside the new button area");
		OutputDebugString(message);
		return true;
	}

	return false;
}

//=====================================================================================
// For detecting the "screen capture stop" the least action that user need to do
// is LBUTTONUP outside the area of toolbar ui NEW button.
// So we need a mouse hook to trap anywhere in the screen.
bool
ResetSnipAlertIfCaptureStopped(WPARAM wParam, MSLLHOOKSTRUCT & msll)
{
	if ((wParam == WM_MOUSEMOVE) || (wParam == WM_LBUTTONDOWN))
	{
		return false;
	}

	CString message;

	if ((wParam == WM_LBUTTONUP) && gSnipAlert)
	{
		CString message;

		// Add additional check to see if the point is on the 
		// snip tool new command again or elsewhere
		// If its on the new command don't reset.
		if (IsPointInSnipWindow(msll.pt))
		{
			OutputDebugString(L"Skipped reset. Toolbar new button hit");
			return false;
		}

		gSnipAlert = 0;
		if (!PostMessage(gMonitorWnd, WM_SNIP_ALERT, gSnipAlert, 0))
		{
			message.Format(L"PostMessage failed Last Error %d", GetLastError());
			OutputDebugString(message);
		} 

		return true;
	}

	return false;
}

bool
SetSnipAlertIfCaptureStarted(MSG & msg)
{
	if (gSnipAlert == 1)
	{
		OutputDebugString(L"SnipAlert already set. Skipping further check");
		return true;
	}

	// Toolbar button new in toolbar ui.
	if (msg.message == TOOLBAR_BUTTON_NEW_MSG_ID)
	{
		OutputDebugString(L"Toolbar New button clicked");
		gSnipAlert = 1;
	}

	if (msg.message == WM_COMMAND)
	{
		// File -> New skip workflow or Ctrl + N
		// toolbar new or toolbar arrow + any type of capture	
		CString message;
		message.Format(L"GetMessage WM_COMMAND wParam %d lParam %d", msg.wParam, msg.lParam);
		OutputDebugString(message);

		if (msg.wParam == FILE_NEWSNIP_COMMAND_ID)
		{
			// Editor UI
			OutputDebugString(L"File new menu item clicked");
			gSnipAlert = 1;
		} else if (msg.wParam  == SNIP_TOOLBAR_NEW_BUTTON_ID && !msg.lParam)
		{
			// Toolbar UI - new button. Sometimes we get this.
			OutputDebugString(L"Toolbar new button clicked");
			gSnipAlert = 1;
		} else {
			int wmId, wmEvent;
			wmId    = LOWORD(msg.wParam);
			wmEvent = HIWORD(msg.wParam);
			CString message;
			message.Format(L"WParam hiword %d loword %d", wmEvent, wmId);
			OutputDebugString(message);
			// toolbar UI
			if (wmEvent == 0) // menu item clicked
			{
				gSnipAlert = 1;
				OutputDebugString(L"One of the snip new menu item clicked");
			} 

			// accelerator new
			if ((wmEvent == 1) && (wmId == FILE_NEWSNIP_COMMAND_ID))
			{
				gSnipAlert = 1;
				OutputDebugString(L"Accelerator new menu item ");
			}
		}
	}

	if (gSnipAlert)
	{
		if (!PostMessage(gMonitorWnd, WM_SNIP_ALERT, gSnipAlert, 0))
		{
			CString message;
			message.Format(L"PostMessage failed Last Error %d", GetLastError());
			OutputDebugString(message);
		} 

		return true;
	}

	return false;
}