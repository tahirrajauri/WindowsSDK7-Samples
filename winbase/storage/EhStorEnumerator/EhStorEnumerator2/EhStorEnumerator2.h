// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "resource.h"

#include "PortableDeviceManagerImp.h"

extern CPortableDeviceManagerImp g_DevManager;

int GetSelectedDevice(HWND hwndDlg, LPTSTR szPNPID, LONG nBufferSize);
void DisplayError(HWND hwndDlg, LPCTSTR szMessage, HRESULT hrError);
