// dllmain.cpp : Defines the entry point for the DLL application.


#pragma once

//Because the tests are simple, the declarations and definitions are in the header file.
#include "pch.h"
#include "MinHook.h"
#include <iostream>

#pragma comment(lib, "libMinHook.x64.lib")

using namespace std;


FILE* log_file;




typedef int (WINAPI* MESSAGEBOXW)(HWND, LPCWSTR, LPCWSTR, UINT);
/* 01 */
typedef BOOL(WINAPI* EXTTEXTOUTW)(HDC, int, int, UINT, const RECT*, LPCWSTR, UINT, const INT*);

// Pointer for calling original MessageBoxW.
MESSAGEBOXW fpMessageBoxW = NULL;
/* 02 */
EXTTEXTOUTW fpExtTextOutW = NULL;

// Detour function which overrides MessageBoxW.
int WINAPI DetourMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	return fpMessageBoxW(hWnd, L"Hooked!", lpCaption, uType);
	return 1;
}

/* 03 */
BOOL WINAPI DetourExtTextOutW(HDC hdc, int x, int y, UINT options, const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx)
{

	
	//FILE* log2 = fopen("d:\\projects\\thefile3.txt", "a+");
	
	fputs("\n", log_file);
	fputs("DetourExtTextOutW\n", log_file);
	fprintf(log_file, "hdc: %p\n", hdc);
	fwprintf(log_file, L"lpString: %s\n", lpString);
	fprintf(log_file, "x: %d\n", x);
	fprintf(log_file, "y: %d\n", y);
	fprintf(log_file, "lprect address: %p\n", lprect);
    if (lprect > (RECT*)0x0) {
		fprintf(log_file, "lprect->left: %d\n", lprect->left);
		fprintf(log_file, "lprect->right: %d\n", lprect->right);
	    fprintf(log_file, "lprect->top: %d\n", lprect->top);
	    fprintf(log_file, "lprect->bottom: %d\n", lprect->bottom);
	}

	
	fputs("\n", log_file);
	
	//fclose(log2);
	





	BOOL result = 0;
	result = fpExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	return result;
}


int test_hook_win_api()
{
	// Initialize MinHook.
	if (MH_Initialize() != MH_OK)
	{
		return 1;
	}

	/*
	// Create a hook for MessageBoxW, in disabled state.
	if (MH_CreateHook(&MessageBoxW, &DetourMessageBoxW,
		reinterpret_cast<LPVOID*>(&fpMessageBoxW)) != MH_OK)
	{
		return 1;
	}
	*/

	/* 04 */

	if (MH_CreateHook(&ExtTextOutW, &DetourExtTextOutW,
		reinterpret_cast<LPVOID*>(&fpExtTextOutW)) != MH_OK)
	{
		return 1;
	}



	// or you can use the new helper funtion like this.
	//if (MH_CreateHookApiEx(
	//    L"user32", "MessageBoxW", &DetourMessageBoxW, &fpMessageBoxW) != MH_OK)
	//{
	//    return 1;
	//}

	/*
	// Enable the hook for MessageBoxW.
	if (MH_EnableHook(&MessageBoxW) != MH_OK)
	{
		return 1;
	}
	*/

	/* 05 */
	if (MH_EnableHook(&ExtTextOutW) != MH_OK)
	{
		return 1;
	}


	/*
	// Expected to tell "Hooked!".
	MessageBoxW(NULL, L"Not hooked...", L"MinHook Sample", MB_OK);
	*/

	/*

	// Disable the hook for MessageBoxW.
	if (MH_DisableHook(&MessageBoxW) != MH_OK)
	{
		return 1;
	}

	// Expected to tell "Not hooked...".
	MessageBoxW(NULL, L"Not hooked...", L"MinHook Sample", MB_OK);

	// Uninitialize MinHook.
	if (MH_Uninitialize() != MH_OK)
	{
		return 1;
	}

	*/

	return 0;

}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	//test_hook_win_api();
	
	switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		log_file = fopen("d:\\projects\\thefile6.txt", "a+");
		test_hook_win_api();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
		//fclose(log_file);
        break;
    }
    return TRUE;
}

