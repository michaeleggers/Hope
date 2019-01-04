#include <windows.h>
#include <stdio.h>

#include "common_os.h"

// NOTE(Michael): whats the deal with overlapped file IO?
char* readTextFile(char const * file)
{
    char* result = 0;
    HANDLE fileHandle;
    fileHandle = CreateFile(file,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
    if (fileHandle == INVALID_HANDLE_VALUE)
        printf("unable to open file!\n");
    
    DWORD filesize = GetFileSize(fileHandle, NULL);
    
    // VirtualAlloc actually allocates a whole page (buffer will be 4k)
    char* buffer = (char*)VirtualAlloc(
        NULL,
        filesize * sizeof(char),
        MEM_COMMIT,
        PAGE_READWRITE
        );
    
    _OVERLAPPED ov = {0};
    LPDWORD numBytesRead = 0;
    DWORD error;
    if (ReadFile(fileHandle, buffer, filesize, numBytesRead, NULL) == 0)
    {
        error = GetLastError();
        char errorMsgBuf[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                      errorMsgBuf, (sizeof(errorMsgBuf) / sizeof(char)), NULL);
        printf("%s\n", errorMsgBuf);
    }
    else
    {
        //buffer[filesize] = '\0';
        result = buffer;
    }
    CloseHandle(fileHandle);
    
    return result;
}

