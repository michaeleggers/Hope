// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform independent

struct refexport_t
{
    int (*init)(HWND* windowHandle, WNDCLASS* windowClass);
    void (*renderFrame)(float* vertices, int numVertices);
};

typedef refexport_t (*GetRefAPI_t)();

