# Windows API Hooking and DLL Injection

This solution demonstrates Windows API hooking and DLL injection. The solution consists of these projects:

* injector
* Monitor_DLL
* Printing_DLL

# Usage

``` 
> injector.exe <target-exe> <dll>
```

* `<target-exe>` is the fully-qualified filename of a 64-bit executable that injector.exe will launch.
* `<dll>` is the fully-qualified filename of a DLL that injector.exe should inject into the executable. 

## Example

```
> injector.exe "C:\Program Files\Windows NT\Accessories\wordpad.exe" D:\Development\git\Win32_API_Hooking\x64\Debug\Printing_DLL.dll
```

# Monitor_DLL.dll

This DLL demonstrates hooking APIs related to starting a program and loading a DLL. 
* `CreateProcessInternalW`
* `LoadLibraryExW`
* `LoadLibraryW`
* `LoadLibraryA`
* `GetProcAddress`

Information about the DLL calls is loged to the path hard-coded in the `logger.h` header file. The path should be updated before compiling the DLL. 


# Printing_DLL.dll

This DLL demonstrates hooking the Windows printing subsystem. It hooks `ExtTextOutW` and logs information about the text content and position printed. The log file path is currently hard-coded and should be updated before compiling the DLL.

Example of a log message.

```
---------------------------------------- START
DetourExtTextOutW
hdc: 000000000E016495
lpString: ABCD
x: 119
y: 0
lprect address: 0000006F40FDDCD0
lprect->left: 0
lprect->right: 816
lprect->top: 0
lprect->bottom: 34

bmWidth: 816
bmHeight: 34
result bottom: 1

---------------------------------------- END
```


# Credits

## injector

Based on: https://github.com/gogcwy/win-api-monitor/blob/master/injector.cpp

## Monitor_DLL

Based on: https://github.com/gogcwy/win-api-monitor/blob/master/injector.cpp

This DLL uses _MinHook - The Minimalistic API Hooking Library for x64/x86_, https://github.com/TsudaKageyu/minhook 
