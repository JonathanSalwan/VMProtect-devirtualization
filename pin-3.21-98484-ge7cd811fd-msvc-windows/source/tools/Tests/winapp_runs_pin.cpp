/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// winapp_runs_pin.cpp : Defines the entry point for the application.
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "winapp_runs_pin.h"
#include <stdio.h>

FILE* fp;
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                     // current instance
TCHAR szTitle[MAX_LOADSTRING];       // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

bool WaitAndVerify(HANDLE process)
{
    if (WaitForSingleObject(process, INFINITE) == WAIT_FAILED)
    {
        fprintf(fp, "WaitAndVerify WAIT_FAILED\n");
        fflush(fp);
        return FALSE;
    }
    DWORD processExitCode;
    if (GetExitCodeProcess(process, &processExitCode) == FALSE)
    {
        fprintf(fp, "WaitAndVerify GetExitCodeProcess failed\n");
        fflush(fp);
        return FALSE;
    }
    if (processExitCode != 0)
    {
        fprintf(fp, "WaitAndVerify processExitCode!=0\n");
        fflush(fp);
        return FALSE;
    }
    fprintf(fp, "WaitAndVerify OK\n");
    fflush(fp);
    return TRUE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    fp = fopen("winapp_runs_pin.txt", "w");

    char createProcessCmd[1024];
    memset(createProcessCmd, 0, 1024);
    strcpy(createProcessCmd, (const char*)lpCmdLine);
    fprintf(fp, "createProcessCmd: %s\n", createProcessCmd);
    char* ptr = strstr(createProcessCmd, "copy");
    ptr += 4;
    *ptr = 0;

    fprintf(fp, "createProcessCmd: %s\n", createProcessCmd);
    fflush(fp);
    // TODO: Place code here.
    //MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_WINAPP_RUNS_PIN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPP_RUNS_PIN));

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));
    fprintf(fp, "call CreateProcess\n");
    fflush(fp);
    // create the pin.exe process with these flags that may cause an extra conhost process
    DWORD flags = CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS;
    if (!CreateProcess(NULL, (LPSTR)createProcessCmd, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi))
    {
        fprintf(fp, "CreateProcess failed\n");
        fflush(fp);
        return (-1);
    }
    fprintf(fp, "WaitAndVerify\n");
    fflush(fp);
    BOOL success = WaitAndVerify(pi.hProcess);
    fprintf(fp, "WaitAndVerify success %d\n", success);
    fflush(fp);
    if (success)
    {
        return (0);
    }
    else
    {
        return (-1);
    }

    /* Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
    */
}

//
//  FUNCTION: MyRegisterClass()
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
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPP_RUNS_PIN));
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName  = MAKEINTRESOURCE(IDC_WINAPP_RUNS_PIN);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance,
                        NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code here...
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
