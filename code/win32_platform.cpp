#include <windows.h>
#include <wingdi.h>
#include <dwmapi.h>

#include <stdio.h>
#include <stdlib.h>

#include "Mathx.h"


#include "helper.h"
#include "ref.h"

#define global_var static;

//#include "game.h"
#include "game.cpp"


global_var extern refexport_t ref; // rendering API

global_var HWND  global_windowHandle;
// global_var HDC   global_deviceContext;
global_var bool running = true;
global_var LARGE_INTEGER performanceFrequency;

Rect get_window_dimensions()
{
    RECT rect;
    GetClientRect(global_windowHandle, &rect);
    Rect result;
    result.width  = rect.right;
    result.height = rect.bottom;
    
    return result;
}

void update_messages()
{
    MSG msg = { };
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool fileExists(char const * file)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(file, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("FindFirstFile failed (%d) for %s\n", GetLastError(), file);
        return false;
    }
    else
    {
        FindClose(hFind);
        return true;
    }
}

global_var HINSTANCE reflib_library;
global_var refexport_t re;

bool VID_LoadRefresh(char const * name)
{
    GetRefAPI_t GetRefAPI;
    
    reflib_library = LoadLibrary(name);
    GetRefAPI = (GetRefAPI_t)GetProcAddress(reflib_library, "GetRefAPI");
    
    re = GetRefAPI();
    
    return true;
}

LRESULT CALLBACK WindowProcCallback(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (uMsg)
    {
        // TODO(Michael): get uniform id from game layer
        case WM_SIZE:
        {
            RECT rect;
            GetClientRect(windowHandle, &rect);
            re.setViewport(rect.left, rect.top, rect.right, rect.bottom);
            re.setProjection(ORTHO);
            re.notify();
            /*
            for (int i = 0;
                 i < MAX_SHADERS;
                 ++i)
            {
                set_ortho(rect.right, rect.bottom, &shaders[i]);
            }
            glViewport(0, 0, rect.right, rect.bottom);
            
            //game_update_and_render(100000000.0f);
            //SwapBuffers(global_deviceContext);
            
            /*
        // recompute orthographic projection matrix
        float aspectRatio = (float)rect.right / (float)rect.bottom;
        float orthoMatrix[16] = { };
        ortho(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f, orthoMatrix);
        glUniformMatrix4fv(ortho_loc, 1, GL_FALSE, orthoMatrix);
*/
        }
        break;
        
        case WM_SIZING:
        {
            printf("WM_SIZING message received!\n");
            //game_update_and_render(0.0f);
            //SwapBuffers(global_deviceContext);
        }
        
        case WM_MOVE:
        {
            printf("WM_MOVE message received!\n");
            //game_update_and_render(100000000.0f);
            //SwapBuffers(global_deviceContext);
        }
        break;
        
        // TODO(Michael): What's the deal with WM_PAINT?
        case WM_PAINT:
        {
            PAINTSTRUCT pstr;
            HDC deviceContext = BeginPaint(windowHandle, &pstr);
            //game_update_and_render(100000000.0f);
            SwapBuffers(deviceContext); // same handle as global DC?
            EndPaint(windowHandle, &pstr);
            result = DefWindowProc(windowHandle, WM_ERASEBKGND, wParam, lParam);
        }
        break;
        
        
        case WM_CLOSE:
        {
            running = false;
            PostQuitMessage(0);
        }
        break;
        
        default:
        {
            result = DefWindowProc(windowHandle, uMsg, wParam, lParam);
        }
    }
    return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    WNDCLASS windowClass = { };
    
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc   = WindowProcCallback;
    windowClass.hInstance     = hInstance;
    windowClass.lpszClassName = "meg_oglcontext";
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    
    RegisterClass(&windowClass);
    
    global_windowHandle = CreateWindow(
        windowClass.lpszClassName,
        "Hello OpenGL",
        WS_OVERLAPPEDWINDOW, // style,so there are buttons like close,minimize,etc
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hInstance,
        0
        );
    
    if (global_windowHandle == NULL)
    {
        return 0;
    }
    
    // init console
    AllocConsole();
    FILE* pCin;
    FILE* pCout;
    FILE* pCerr;
    freopen_s(&pCin, "conin$", "r", stdin);
    freopen_s(&pCout, "conout$", "w", stdout);
    freopen_s(&pCerr, "conout$", "w", stderr);
    
    // init GL
    VID_LoadRefresh("win32_opengl.dll");
    re.init(&global_windowHandle, &windowClass);
    // initGL(&global_windowHandle, &windowClass);
    
    ShowWindow(global_windowHandle, nCmdShow);
    
    // show some GL info in window title-bar
    // TODO(Michael): query info via implementation in DLL
    /*
    GLubyte const * glVersion = glGetString(GL_VERSION);
    GLubyte const * glRenderer = glGetString(GL_RENDERER);
    meg_strbuf strbuf = meg_strbuf_create();
    meg_strbuf_write(&strbuf, "GL-VERSION: ");
    meg_strbuf_write(&strbuf, (char*)glVersion);
    meg_strbuf_write(&strbuf, " ::: RENDERER: ");
    meg_strbuf_write(&strbuf, (char*)glRenderer);
    SetWindowText(global_windowHandle, LPCSTR(strbuf.buffer));
    DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
    */
    
    //glEnableVertexAttribArray (0);
    //glEnableVertexAttribArray (1);
    
    // set viewport dimensions
    RECT rect;
    GetClientRect(global_windowHandle, &rect);
    //glViewport(0, 0, rect.right, rect.bottom); // TODO(Michael): do in renderer
    
    game_init(&re);
    
    // set up timing stuff
    QueryPerformanceFrequency(&performanceFrequency);
    LARGE_INTEGER startingTime, endingTime, elapsedTime;
    QueryPerformanceCounter(&startingTime);
    QueryPerformanceCounter(&endingTime);
    elapsedTime.QuadPart = 0;
    float fps = 0.0f;
    
    
    // if test
#if _WIN32
    printf("ON WIN32 PLATFORM!\n");
#endif
    
    while (running)
    {
        
        // Run the message loop.
        MSG msg = { };
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        game_update_and_render((float)elapsedTime.QuadPart, &re); 
        //SwapBuffers(global_deviceContext);
        
        QueryPerformanceCounter(&endingTime);
        elapsedTime.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
        fps = (float)performanceFrequency.QuadPart / (float)elapsedTime.QuadPart;
        elapsedTime.QuadPart *= 1000000; // microseconds
        elapsedTime.QuadPart /= performanceFrequency.QuadPart;
        
        //printf("%f\n", fps);
        //printf("elapsed time: %f\n", (float)elapsedTime.QuadPart / 1000.0f);
        QueryPerformanceCounter(&startingTime);
        //Sleep(500); // HACK(Michael): artificial time
    }
    
    fclose(pCin);
    fclose(pCout);
    fclose(pCerr);
    FreeConsole();
    
    //wglMakeCurrent(0, 0);
    //wglDeleteContext(global_oglRenderContext);
    //ReleaseDC(global_windowHandle, global_deviceContext);
    
    return 0;
}
