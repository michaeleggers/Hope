#include <windows.h>
#include <wingdi.h>
#include <dwmapi.h>
#include <xinput.h>
#include <shellscalingapi.h>

#include <stdio.h>
#include <stdlib.h>



#include "platform.h"
#include "ref.h"

#define global_var static;

//#include "game.h"
#include "game.cpp"


global_var extern refexport_t ref; // rendering API

global_var HWND  global_windowHandle;
// global_var HDC   global_deviceContext;
global_var bool running = true;
global_var LARGE_INTEGER performanceFrequency;
global_var InputDevice inputDevice;

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


// NOTE(Michael): whats the deal with overlapped file IO?
PLATFORM_READ_TEXT_FILE(win32ReadTextFile)
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
    char *buffer = (char*)VirtualAlloc(
        NULL,
        filesize * sizeof(char),
        MEM_RESERVE|MEM_COMMIT,
        PAGE_READWRITE
        );
    
    _OVERLAPPED ov = {0};
    DWORD numBytesRead = 0;
    DWORD error;
    if (ReadFile(fileHandle, buffer, filesize, &numBytesRead, NULL) == 0)
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

PLATFORM_GET_WINDOW_DIMENSIONS(win32GetWindowDimensions)
{
    Rect result;
    RECT rect;
    GetClientRect(global_windowHandle, &rect);
    result.width  = rect.right;
    result.height = rect.bottom;
    
    return result;
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
bool VID_LoadRefresh(char const * name, PlatformAPI* platform_api)
{
    GetRefAPI_t GetRefAPI;
    
    reflib_library = LoadLibrary(name);
    GetRefAPI = (GetRefAPI_t)GetProcAddress(reflib_library, "GetRefAPI");
    
    re = GetRefAPI(platform_api);
    
    return true;
}

global_var HINSTANCE xinput_library;
typedef DWORD (*xinput_get_state)(DWORD dwUserIndex, XINPUT_STATE *pState);
DWORD XInputGetStateStub(DWORD dwUserIndex, XINPUT_STATE *pState){ return 0; }
global_var xinput_get_state XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_
bool XInputInit()
{
    xinput_library = LoadLibrary("Xinput1_4.dll");
    if ( !xinput_library )
    {
        xinput_library = LoadLibrary("Xinput9_1_0.dll");
    }
    if ( !xinput_library )
    {
        return false;
    }
    else
    {
        XInputGetState = (xinput_get_state)GetProcAddress(xinput_library, "XInputGetState");
    }
    
    return true;
}

Keyboard keyboard = {};
Mouse mouse = {};

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
            //re.setProjection(ORTHO);
            re.notify();
        }
        break;
        
        case WM_SIZING:
        {
            printf("WM_SIZING message received!\n");
            //game_update_and_render(0.0f);
            //SwapBuffers(global_deviceContext);
        }
        break;
        
        case WM_MOVE:
        {
            printf("WM_MOVE message received!\n");
            //game_update_and_render(100000000.0f);
            //SwapBuffers(global_deviceContext);
        }
        break;
        
        // TODO(Michael): What's the deal with WM_PAINT?
        /*
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
        */
        case WM_CLOSE:
        {
            running = false;
            PostQuitMessage(0);
        }
        break;
        
        case WM_MOUSEMOVE:
        {
            mouse.x = lParam & 0x0000FFFF;
            mouse.y = (lParam & 0xFFFF0000) >> 16;
            //printf ("MOUSE X: %d\n", mouse.x);
            //printf ("MOUSE Y: %d\n", mouse.y);
            // NOTE(Michael): If an application processes this message, it should return zero. (MSDN)
        }
        break;
        
        case WM_LBUTTONDOWN:
        {
            if (inputDevice.deviceType != MOUSE)
            {
                inputDevice.deviceType = MOUSE;
                printf("input device: MOUSE\n");
            }
            switch(wParam)
            {
                case MK_LBUTTON:
                {
                    mouse.keycodes[LBUTTON_DOWN] = 1;
                    mouse.keycodes[LBUTTON_UP] = 0;
                    mouse.x = lParam & 0x0000FFFF;
                    mouse.y = (lParam & 0xFFFF0000) >> 16;
                    //printf ("MOUSE X: %d\n", mouse.x);
                    //printf ("MOUSE Y: %d\n", mouse.y);
                }
                break;
                
                default:
                {
                    mouse.keycodes[LBUTTON_DOWN] = 0;
                }
            }
        }
        break;
        
        case WM_LBUTTONUP:
        {
            //printf("MOUSE BUTTON IS UP\n");
            mouse.keycodes[LBUTTON_UP] = 1;
            mouse.keycodes[LBUTTON_DOWN] = 0;
        }
        break;
        
        case WM_KEYDOWN:
        {
            if (inputDevice.deviceType != KEYBOARD)
            {
                inputDevice.deviceType = KEYBOARD;
                printf("input device: KEYBOARD\n");
            }
            switch(wParam)
            {
                case VK_UP:
                {
                    keyboard.keycodes[ARROW_UP] = 1;
                }
                break;
                
                case VK_DOWN:
                {
                    keyboard.keycodes[ARROW_DOWN] = 1;
                }
                break;
                
                case VK_LEFT:
                {
                    keyboard.keycodes[ARROW_LEFT] = 1;
                }
                break;
                
                case VK_RIGHT:
                {
                    keyboard.keycodes[ARROW_RIGHT] = 1;
                }
                break;
                
                case 0x41: // A
                {
                    keyboard.keycodes[LETTER_A] = 1;
                }
                break;
                
                case VK_ESCAPE:
                {
                    keyboard.keycodes[ESCAPE] = 1;
                }
                break;
            }
        }
        break;
        
        case WM_KEYUP:
        {
            switch(wParam)
            {
                case VK_UP:
                {
                    keyboard.keycodes[ARROW_UP] = 0;
                }
                break;
                
                case VK_DOWN:
                {
                    keyboard.keycodes[ARROW_DOWN] = 0;
                }
                break;
                
                case VK_LEFT:
                {
                    keyboard.keycodes[ARROW_LEFT] = 0;
                }
                break;
                
                case VK_RIGHT:
                {
                    keyboard.keycodes[ARROW_RIGHT] = 0;
                }
                break;
                
                case 0x41: // A
                {
                    keyboard.keycodes[LETTER_A] = 0;
                }
                break;
                
                case VK_ESCAPE:
                {
                    keyboard.keycodes[ESCAPE] = 0;
                }
                break;
            }
        }
        break;
        
        default:
        {
            result = DefWindowProc(windowHandle, uMsg, wParam, lParam);
        }
    }
    return result;
}

struct XBoxControllerState
{
    DWORD packetNumber;
    WORD digitalButtons;
    int connected;
};

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
        "EngineWorkInProgress...",
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
    
    // TODO(Michael): DPI scaling using this function only works from win 8.1 on!
#if 1    
    HRESULT scalingAwarenessResult = SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
    if (scalingAwarenessResult == S_OK)
        OutputDebugStringA("DPI scaling set successfully\n");
    else if (scalingAwarenessResult == E_ACCESSDENIED)
        OutputDebugStringA("DPI scaling failerd. It has already been set before\n");
    else
        OutputDebugStringA("DPI scaling failed. Illegal Value\n");
#endif
    
    // init console
    AllocConsole();
    FILE* pCin;
    FILE* pCout;
    FILE* pCerr;
    freopen_s(&pCin, "conin$", "r", stdin);
    freopen_s(&pCout, "conout$", "w", stdout);
    freopen_s(&pCerr, "conout$", "w", stderr);
    
    // init platform API
    PlatformAPI platformAPI;
    platformAPI.readTextFile = win32ReadTextFile;
    platformAPI.getWindowDimensions = win32GetWindowDimensions;
    
    // init GL
    VID_LoadRefresh("win32_opengl.dll", &platformAPI);
    if (re.init(&global_windowHandle, &windowClass) != 0)
    {
        printf("failed to initialize render-context.\n");
        if (MessageBox(
            global_windowHandle,
            "Failed to initialize render-context.",
            "init error",
            MB_OK) == IDOK)
        {
            return 1;
        }
    }
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
    
    
    XInputInit();
    // XBox Controller state
    XBoxControllerState controllerState1 = {};
    XBoxControllerState controllerState2 = {};
    Controller controller1 = {};
    Controller controller2 = {};
    // Generic input device. Holds pointer to a couple of input deivces such as
    // keyboard, mouse, controller...
    inputDevice.keyboard    = &keyboard;
    inputDevice.controller1 = &controller1;
    inputDevice.controller2 = &controller2;
    inputDevice.mouse       = &mouse;
    inputDevice.deviceType  = KEYBOARD;
    
    game_init(&platformAPI, &inputDevice, &re);
    OutputDebugStringA("game init done\n");
    
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
        if (keyDown(ESCAPE))
            running = 0;
        // Run the message loop.
        MSG msg = { };
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // XBox Controller
        
        DWORD dwResult;    
        for (DWORD i=0; i< XUSER_MAX_COUNT; i++ )
        {
            XINPUT_STATE state;
            ZeroMemory( &state, sizeof(XINPUT_STATE) );
            
            // Simply get the state of the controller from XInput.
            dwResult = XInputGetState( i, &state );
            
            Controller * controller = 0;
            XBoxControllerState * controllerState = 0;
            if( dwResult == ERROR_SUCCESS )
            {
                // Controller is connected 
                if (i == 0) {
                    controller = &controller1;
                    controllerState = &controllerState1;
                }
                else if (i == 1) {
                    controller = &controller2;
                    controllerState = &controllerState2;
                }
                
                if (!controller->connected) {
                    printf("controller%d connected\n", i);
                }
                controller->connected = 1;
                
                DWORD currentPacketNumber = state.dwPacketNumber;
                DWORD oldPacketNumber = controllerState->packetNumber;
                if (oldPacketNumber != currentPacketNumber && inputDevice.deviceType != CONTROLLER)
                {
                    printf("input device: CONTROLLER\n");
                    inputDevice.deviceType = CONTROLLER;
                }
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
                {
                    controller->keycodes[DPAD_UP] = 1;
                }
                else
                    controller->keycodes[DPAD_UP] = 0;
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                {
                    controller->keycodes[DPAD_DOWN] = 1;
                }
                else
                    controller->keycodes[DPAD_DOWN] = 0;
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                {
                    controller->keycodes[DPAD_LEFT] = 1;
                }
                else
                    controller->keycodes[DPAD_LEFT] = 0;
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                {
                    controller->keycodes[DPAD_RIGHT] = 1;
                }
                else
                    controller->keycodes[DPAD_RIGHT] = 0;
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
                    controller->keycodes[DPAD_A] = 1;
                else
                    controller->keycodes[DPAD_A] = 0;
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
                    controller->keycodes[DPAD_B] = 1;
                else
                    controller->keycodes[DPAD_B] = 0;
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
                    controller->keycodes[DPAD_X] = 1;
                else
                    controller->keycodes[DPAD_X] = 0;
                if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
                    controller->keycodes[DPAD_Y] = 1;
                else
                    controller->keycodes[DPAD_Y] = 0;
                controllerState->packetNumber = currentPacketNumber;
            }
            else
            {
                // Controller is not connected
                if (i == 0 && controller1.connected)
                {
                    printf("controller%d disconnected\n", i);
                    controller1.connected = 0;
                }
                if (i == 1 && controller2.connected)
                {
                    printf("controller%d disconnected\n", i);
                    controller2.connected = 0;
                }
            }
        }
        // end XBox Controller
        
        game_update_and_render((float)elapsedTime.QuadPart, &inputDevice, &re); 
        //SwapBuffers(global_deviceContext);
        
        QueryPerformanceCounter(&endingTime);
        elapsedTime.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
        fps = (float)performanceFrequency.QuadPart / (float)elapsedTime.QuadPart;
        elapsedTime.QuadPart *= 1000000; // microseconds
        elapsedTime.QuadPart /= performanceFrequency.QuadPart; // elapsed time is in microseconds now
        
        //printf("%f\n", fps);
        //printf("elapsed time: %f\n", (float)elapsedTime.QuadPart / 1000.0f);
        
        QueryPerformanceCounter(&startingTime);
        //Sleep(500); // HACK(Michael): artificial time
        
        //OutputDebugStringA("end game loop\n");
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
