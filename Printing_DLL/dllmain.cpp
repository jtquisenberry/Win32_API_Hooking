// dllmain.cpp : Defines the entry point for the DLL application.


#pragma once

//Because the tests are simple, the declarations and definitions are in the header file.
#include "pch.h"
#include "MinHook.h"
#include <iostream>
#include <windows.h>
#include <winspool.h>

#pragma comment(lib, "libMinHook.x64.lib")
#pragma comment(lib, "winspool.lib")

using namespace std;


void WriteToPrinter();
void DrawStuff(HDC hdc, int i);
std::wstring current_time_to_stringW();
HDC StartWriteToPrinter();
HDC MakePrinterDc(LPDEVMODE devmode, wchar_t* printerName);
void EndRunPrintJob(HDC hDC);


FILE* log_file;

HDC memDC = NULL;
HBITMAP memBM = NULL;
int CaptureAnImagePrinter(HDC hdcMemDC, HBITMAP hbmScreen, HDC hdcWindow);
int CaptureAnImageDc(HDC hdcWindow);

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
	fputs("---------------------------------------- START\n", log_file);
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

	/*
	BOOL resultA = 0;
	if (memDC == NULL)
	{
		memDC = CreateCompatibleDC(NULL);
	}
	*/

	//HDC hdcScreen = GetDC(NULL);
	//fpExtTextOutW(hdcScreen, x, y, options, lprect, lpString, c, lpDx);
	memDC = CreateCompatibleDC(NULL);



	BITMAP structBitmapHeader;
	memset(&structBitmapHeader, 0, sizeof(BITMAP));
	HGDIOBJ hBitmap = GetCurrentObject(hdc, OBJ_BITMAP);
	GetObject(hBitmap, sizeof(BITMAP), &structBitmapHeader);
	int bmWidth = structBitmapHeader.bmWidth;
	int bmHeight = structBitmapHeader.bmHeight;

	fprintf(log_file, "bmWidth: %d\n", bmWidth);
	fprintf(log_file, "bmHeight: %d\n", bmHeight);
	fflush(log_file);

	if ((bmWidth < 2) && (bmHeight < 2) && (x<1000) && (y<1000) )
	{

		fwprintf(log_file, L"***lpString***: %s\n", lpString);
		HBITMAP hbmp = CreateCompatibleBitmap(memDC,
			3000, 3000);
		SelectObject(memDC, hbmp);
		
		// Paints most of the rectangle white
		// https://learn.microsoft.com/en-us/answers/questions/530936/createcompatabilebitmap-background-is-always-black
		PatBlt(memDC, 0, 0, 2500, 2500, WHITENESS);
		fpExtTextOutW(memDC, x, y, options, lprect, lpString, c, lpDx);
		CaptureAnImageDc(memDC);
		// TODO: Delete device context `memDC` and bitmap `hbmp`.

		HDC hDC2 = StartWriteToPrinter();
		//fwprintf(log_file, L"hDC2: %p\n", hdc);
		fprintf(log_file, "+++++hDC2+++++: %p\n", hDC2);
		
		fflush(log_file);
		BOOL result;
		result = fpExtTextOutW(hDC2, x, y, options, lprect, lpString, c, lpDx);
		fprintf(log_file, "result TOP: %d\n", result);
		fflush(log_file);


		EndRunPrintJob(hDC2);










		/* Write to printer*/
		// Works with wchar_t pDeviceName[MAX_PATH] = L"Bullzip PDF Printer";
		// For L"hp LaserJet 1320 PCL 6", a job is submitted but does not leave the spooler.
		/*
		fwprintf(log_file, L"AAAAAAAAAA*: %s\n", lpString);
		fflush(log_file);
		HANDLE phPrinter;
		fwprintf(log_file, L"BBBBBBBBB*: %s\n", lpString);
		fflush(log_file);
		HDC printerDC;
		fwprintf(log_file, L"CCCCCCCCCCC*: %s\n", lpString);
		fflush(log_file);
		// A pointer to a DEVMODE structure that receives the printer configuration data specified by the user.
		DEVMODE* pDevModeOutput;

		// A pointer to a null-terminated string that specifies the name of the device for which the 
		// printer-configuration property sheet is displayed.
		wchar_t pDeviceName[MAX_PATH] = L"Bullzip PDF Printer";

		OpenPrinter(pDeviceName, &phPrinter, NULL);

		int size = DocumentProperties(NULL, phPrinter, pDeviceName, NULL, NULL, 0);
		pDevModeOutput = (DEVMODE*)malloc(size);
		DocumentProperties(NULL, phPrinter, pDeviceName, pDevModeOutput, NULL, DM_OUT_BUFFER);
		printerDC = CreateDC(L"WINSPOOL", pDeviceName, NULL, pDevModeOutput);

		fprintf(log_file, "printerDC: %p\n", printerDC);
		fflush(log_file);

		int result = -999;

		DOCINFO info;
		memset(&info, 0, sizeof(info));
		info.cbSize = sizeof(info);
		int jobId = StartDoc(printerDC, &info);
		fprintf(log_file, "job: %d\n", jobId);
		fflush(log_file);
		result = StartPage(printerDC);
		fprintf(log_file, "StartPage: %d\n", result);
		fflush(log_file);
		result = fpExtTextOutW(printerDC, x, y, options, lprect, L"AAAAAAA", c, lpDx);
		
		fprintf(log_file, "fpExtTextOutW: %d\n", result);
		fflush(log_file);
		Sleep(3000);
		result = EndPage(printerDC);
		fprintf(log_file, "EndPage: %d\n", result);
		fflush(log_file);
		result = EndDoc(printerDC);
		fprintf(log_file, "EndDoc: %d\n", result);
		fflush(log_file);
		
		result = ClosePrinter(phPrinter);
		fprintf(log_file, "ClosePrinter: %d\n", result);
		fflush(log_file);
		*/

	}
	
	/*
	HDC hdc = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmp = CreateCompatibleBitmap(hdcScreen,
		400, 400);
	SelectObject(hdc, hbmp);
	
	//resultA = fpExtTextOutW(memDC, x, y, options, lprect, lpString, c, lpDx);
	resultA = fpExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	
	if (memBM == NULL)
	{
		memBM = CreateCompatibleBitmap(hdc, 400, 400);
	}

	
	SelectObject(memDC, memBM);

	
	if (y>0 && x>0)
	{
		CaptureAnImagePrinter(memDC, memBM, hdc);
	}
	*/

	//CaptureAnImagePrinter(memDC, memBM, hdc);

	
	//fclose(log2);
	




	BOOL result = 0;
	result = fpExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	fprintf(log_file, "result bottom: %d\n", result);
	fflush(log_file);

	fputs("---------------------------------------- END\n", log_file);
	fflush(log_file);
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


int CaptureAnImagePrinter(HDC hdcMemDC, HBITMAP hbmScreen, HDC hdcWindow)
{
	//HDC hdcScreen;
	//HDC hdcWindow;
	//HDC hdcMemDC = NULL;
	//HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;
	DWORD dwBytesWritten = 0;
	DWORD dwSizeofDIB = 0;
	HANDLE hFile = NULL;
	char* lpbitmap = NULL;
	HANDLE hDIB = NULL;
	DWORD dwBmpSize = 0;

	int result = -999;

	// Bit block transfer into our compatible memory DC.
	BitBlt(hdcMemDC,
		0, 0,
		400, 400,
		hdcWindow,
		0, 0,
		SRCCOPY);

	// Get the BITMAP from the HBITMAP.
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	BITMAPFILEHEADER   bmfHeader;
	BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
	// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
	// have greater overhead than HeapAlloc.
	hDIB = GlobalAlloc(GHND, dwBmpSize);
	lpbitmap = (char*)GlobalLock(hDIB);

	// Gets the "bits" from the bitmap, and copies them into a buffer 
	// that's pointed to by lpbitmap.
	GetDIBits(hdcWindow, hbmScreen, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap,
		(BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// A file is created, this is where we will save the screen capture.
	hFile = CreateFile(L"d:\\projects\\captureqwsxA.bmp",
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	// Add the size of the headers to the size of the bitmap to get the total file size.
	dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	// Size of the file.
	bmfHeader.bfSize = dwSizeofDIB;

	// bfType must always be BM for Bitmaps.
	bmfHeader.bfType = 0x4D42; // BM.

	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

	// Unlock and Free the DIB from the heap.
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	// Close the handle for the file that was created.
	CloseHandle(hFile);

	// Clean up.
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);


	return 0;
}


int CaptureAnImageDc(HDC hdcWindow)
{
	HDC hdcMemDC = NULL;
	HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;
	DWORD dwBytesWritten = 0;
	DWORD dwSizeofDIB = 0;
	HANDLE hFile = NULL;
	char* lpbitmap = NULL;
	HANDLE hDIB = NULL;
	DWORD dwBmpSize = 0;

	int result = -999;


	BITMAP structBitmapHeader;
	memset(&structBitmapHeader, 0, sizeof(BITMAP));
	HGDIOBJ hBitmap = GetCurrentObject(hdcWindow, OBJ_BITMAP);
	GetObject(hBitmap, sizeof(BITMAP), &structBitmapHeader);
	int bmWidth = structBitmapHeader.bmWidth;
	int bmHeight = structBitmapHeader.bmHeight;

	// Create a compatible DC, which is used in a BitBlt from the window DC.
	hdcMemDC = CreateCompatibleDC(hdcWindow);

	BITMAP structBitmapHeader2;
	memset(&structBitmapHeader2, 0, sizeof(BITMAP));
	HGDIOBJ hBitmap2 = GetCurrentObject(hdcMemDC, OBJ_BITMAP);
	GetObject(hBitmap2, sizeof(BITMAP), &structBitmapHeader2);
	int bmWidth2 = structBitmapHeader2.bmWidth;
	int bmHeight2 = structBitmapHeader2.bmHeight;



	// Bit block transfer into our compatible memory DC.
	BitBlt(hdcMemDC,
		0, 0,
		bmWidth, bmHeight,
		hdcWindow,
		0, 0,
		SRCCOPY);

	// Create a compatible bitmap from the Window DC.
	hbmScreen = CreateCompatibleBitmap(hdcWindow, bmWidth, bmHeight);

	// Select the compatible bitmap into the compatible memory DC.
	SelectObject(hdcMemDC, hbmScreen);

	// Bit block transfer into our compatible memory DC.
	BitBlt(hdcMemDC,
		0, 0,
		bmWidth, bmHeight,
		hdcWindow,
		0, 0,
		SRCCOPY);

	// Get the BITMAP from the HBITMAP.
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	BITMAPFILEHEADER   bmfHeader;
	BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
	// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
	// have greater overhead than HeapAlloc.
	hDIB = GlobalAlloc(GHND, dwBmpSize);
	lpbitmap = (char*)GlobalLock(hDIB);

	// Gets the "bits" from the bitmap, and copies them into a buffer 
	// that's pointed to by lpbitmap.
	GetDIBits(hdcWindow, hbmScreen, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap,
		(BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// A file is created, this is where we will save the screen capture.
	hFile = CreateFile(L"d:\\projects\\captureqwsxB.bmp",
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	// Add the size of the headers to the size of the bitmap to get the total file size.
	dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	// Size of the file.
	bmfHeader.bfSize = dwSizeofDIB;

	// bfType must always be BM for Bitmaps.
	bmfHeader.bfType = 0x4D42; // BM.

	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

	// Unlock and Free the DIB from the heap.
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	// Close the handle for the file that was created.
	CloseHandle(hFile);

	// Clean up.
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);


	return 0;
}

LPDEVMODE GetLandscapeDevMode(HWND hWnd, wchar_t* pDevice)
{
	HANDLE hPrinter;
	LPDEVMODE pDevMode;
	DWORD dwNeeded, dwRet;

	/* Start by opening the printer */
	if (!OpenPrinter(pDevice, &hPrinter, NULL))
		return NULL;

	/*
	* Step 1:
	* Allocate a buffer of the correct size.
	*/
	dwNeeded = DocumentProperties(hWnd,
		hPrinter, /* Handle to our printer. */
		pDevice, /* Name of the printer. */
		NULL, /* Asking for size, so */
		NULL, /* these are not used. */
		0); /* Zero returns buffer size. */
	pDevMode = (LPDEVMODE)malloc(dwNeeded);

	/*
	* Step 2:
	* Get the default DevMode for the printer and
	* modify it for your needs.
	*/
	dwRet = DocumentProperties(hWnd,
		hPrinter,
		pDevice,
		pDevMode, /* The address of the buffer to fill. */
		NULL, /* Not using the input buffer. */
		DM_OUT_BUFFER); /* Have the output buffer filled. */
	if (dwRet != IDOK)
	{
		/* If failure, cleanup and return failure. */
		free(pDevMode);
		ClosePrinter(hPrinter);
		return NULL;
	}

	///* Make changes to the DevMode which are supported.
	//*/
	//if (pDevMode->dmFields & DM_ORIENTATION)
	//{
	//	/* If the printer supports paper orientation, set it.*/
	//	pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
	//}
	//
	//
	//
	//if (pDevMode->dmFields & DM_DUPLEX)
	//{
	//	/* If it supports duplex printing, use it. */
	//	pDevMode->dmDuplex = DMDUP_HORIZONTAL;
	//}
	//
	///*
	//* Step 3:
	//* Merge the new settings with the old.
	//* This gives the driver an opportunity to update any private
	//* portions of the DevMode structure.
	//*/
	//dwRet = DocumentProperties(hWnd,
	//	hPrinter,
	//	pDevice,
	//	pDevMode, /* Reuse our buffer for output. */
	//	pDevMode, /* Pass the driver our changes. */
	//	DM_IN_BUFFER | /* Commands to Merge our changes and */
	//	DM_OUT_BUFFER); /* write the result. */
	//
	///* Finished with the printer */
	//ClosePrinter(hPrinter);
	//
	//if (dwRet != IDOK)
	//{
	//	/* If failure, cleanup and return failure. */
	//	free(pDevMode);
	//	return NULL;
	//}

	/* Return the modified DevMode structure. */
	return pDevMode;
}



/*===============================*/
/* The Abort Procudure */
/* ==============================*/
BOOL CALLBACK AbortProc(HDC hDC, int Error)
{
	MSG   msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return TRUE;
}



/*===============================*/
/* Initialize DOCINFO structure */
/* ==============================*/
void InitPrintJobDoc(DOCINFO* di, wchar_t* docname)
{
	memset(di, 0, sizeof(DOCINFO));
	/* Fill in the required members. */
	di->cbSize = sizeof(DOCINFO);
	di->lpszDocName = docname;
}


/*==============================================*/
/* Sample code : Typical printing process */
/* =============================================*/
HDC XXXMakePrinterDc(LPDEVMODE devmode, wchar_t* printerName)
{
	HDC        hDC;
	DOCINFO    di;

	hDC = CreateDC(NULL, printerName, NULL, devmode);

	/* Did you get a good DC? */
	if (!hDC)
	{
		MessageBox(NULL,
			L"Error creating DC",
			L"Error",
			MB_APPLMODAL | MB_OK);
		return NULL;
	}

	/* You always have to use an AbortProc(). */
	if (SetAbortProc(hDC, AbortProc) == SP_ERROR)
	{
		MessageBox(NULL,
			L"Error setting up AbortProc",
			L"Error",
			MB_APPLMODAL | MB_OK);
		return NULL;
	}

	/* Init the DOCINFO and start the document. */
	InitPrintJobDoc(&di, (LPWSTR)L"MyDoc");
	StartDoc(hDC, &di);
	return hDC;
}


void EndRunPrintJob(HDC hDC)
{
	/* Print 1st page. */
	//StartPage(hDC);
	//DrawStuff(hDC, 1);
	EndPage(hDC);

	/* Indicate end of document.*/
	EndDoc(hDC);
	/* Clean up */
	DeleteDC(hDC);
}




/*==============================================*/
/* Sample code : Typical printing process */
/* =============================================*/
void RunPrintJob(LPDEVMODE devmode, wchar_t* printerName)
{
	HDC        hDC;
	DOCINFO    di;

	hDC = CreateDC(NULL, printerName, NULL, devmode);

	/* Did you get a good DC? */
	if (!hDC)
	{
		MessageBox(NULL,
			L"Error creating DC",
			L"Error",
			MB_APPLMODAL | MB_OK);
		return;
	}

	/* You always have to use an AbortProc(). */
	if (SetAbortProc(hDC, AbortProc) == SP_ERROR)
	{
		MessageBox(NULL,
			L"Error setting up AbortProc",
			L"Error",
			MB_APPLMODAL | MB_OK);
		return;
	}

	/* Init the DOCINFO and start the document. */
	InitPrintJobDoc(&di, (LPWSTR)L"MyDoc");
	StartDoc(hDC, &di);

	/* Print 1st page. */
	StartPage(hDC);
	DrawStuff(hDC, 1);
	EndPage(hDC);

	/* Indicate end of document.*/
	EndDoc(hDC);
	/* Clean up */
	DeleteDC(hDC);
}


HDC MakePrinterDc(LPDEVMODE devmode, wchar_t* printerName)
{
	HDC        hDC;
	DOCINFO    di;

	hDC = CreateDC(NULL, printerName, NULL, devmode);

	/* Did you get a good DC? */
	if (!hDC)
	{
		MessageBox(NULL,
			L"Error creating DC",
			L"Error",
			MB_APPLMODAL | MB_OK);
		return NULL;
	}

	/* You always have to use an AbortProc(). */
	if (SetAbortProc(hDC, AbortProc) == SP_ERROR)
	{
		MessageBox(NULL,
			L"Error setting up AbortProc",
			L"Error",
			MB_APPLMODAL | MB_OK);
		return NULL;
	}

	/* Init the DOCINFO and start the document. */
	InitPrintJobDoc(&di, (LPWSTR)L"MyDoc");
	StartDoc(hDC, &di);
	/* Print 1st page. */
	StartPage(hDC);
	return hDC;
}




void DrawStuff(HDC hdc, int i)
{
	// Get current date and time as Unicode string
	std::wstring timeString = current_time_to_stringW();

	// Initialize a RECT structure.
	RECT rect = { 600, 600, 700, 700 };

	// Output string
	std::wstring outString_temp1 = L"ExtTextOut ";
	std::wstring outString1 = outString_temp1 + timeString;
	std::wstring outString_temp2 = L"TextOut ";
	std::wstring outString2 = outString_temp2 + timeString;

	ExtTextOut(hdc, 300, 300, 0, &rect, outString1.c_str(), outString1.length(), NULL);
	TextOut(hdc, 600, 600, outString2.c_str(), outString2.length());
	return;
}

std::wstring current_time_to_stringW()
{
	struct tm newtime;
	//char am_pm[] = "AM";
	__time64_t long_time;
	// char timebuf[26];
	errno_t err;

	// Get current time as 64-bit integer.
	_time64(&long_time);

	// Convert to local time.
	err = _localtime64_s(&newtime, &long_time);

	wchar_t output[30];
	wcsftime(output, 30, L"%Y %m-%d %H-%M-%S", &newtime);

	// Convert to an ASCII representation.
	//err = asctime_s(timebuf, 26, &newtime);

	return output;
}


/* Print DC demo */
/*
int APIENTRY main(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	//std::wstring timeString = current_time_to_stringW();

	//wchar_t* printerName = const_cast<wchar_t*>(L"hp LaserJet 1320 PCL 6");
	//LPDEVMODE devmode = GetLandscapeDevMode(NULL, printerName);
	// RunPrintJob(devmode, printerName);

	return 0;
}
*/



void WriteToPrinter()
{
	wchar_t* printerName = const_cast<wchar_t*>(L"hp LaserJet 1320 PCL 6");
	LPDEVMODE devmode = GetLandscapeDevMode(NULL, printerName);
	RunPrintJob(devmode, printerName);
}

HDC StartWriteToPrinter()
{
	wchar_t* printerName = const_cast<wchar_t*>(L"Bullzip PDF Printer");
	//wchar_t* printerName = const_cast<wchar_t*>(L"hp LaserJet 1320 PCL 6");
	LPDEVMODE devmode = GetLandscapeDevMode(NULL, printerName);
	return MakePrinterDc(devmode, printerName);
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

