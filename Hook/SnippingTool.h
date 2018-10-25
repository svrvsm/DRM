#pragma once
#include <Windows.h>

bool ResetSnipAlertIfCaptureStopped(WPARAM, MSLLHOOKSTRUCT &);
bool SetSnipAlertIfCaptureStarted(MSG &);
DWORD GetThreadIdForSnipEditorWnd();
