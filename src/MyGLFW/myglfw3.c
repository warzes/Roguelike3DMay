#define _CRT_SECURE_NO_WARNINGS
#include "myglfw3.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>

#define _GLFW_INSERT_FIRST      0
#define _GLFW_INSERT_LAST       1

#define _GLFW_MESSAGE_SIZE      1024

typedef int GLFWbool;
typedef void (*GLFWproc)(void);

typedef struct _GLFWerror       _GLFWerror;
typedef struct _GLFWinitconfig  _GLFWinitconfig;
typedef struct _GLFWwndconfig   _GLFWwndconfig;
typedef struct _GLFWctxconfig   _GLFWctxconfig;
typedef struct _GLFWfbconfig    _GLFWfbconfig;
typedef struct _GLFWcontext     _GLFWcontext;
typedef struct _GLFWwindow      _GLFWwindow;
typedef struct _GLFWlibrary     _GLFWlibrary;
typedef struct _GLFWmonitor     _GLFWmonitor;
typedef struct _GLFWcursor      _GLFWcursor;
typedef struct _GLFWtls         _GLFWtls;
typedef struct _GLFWmutex       _GLFWmutex;

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#if WINVER < 0x0601
#undef WINVER
#define WINVER 0x0601
#endif
#if _WIN32_WINNT < 0x0601
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#define OEMRESOURCE

#include <windows.h>
#include <dwmapi.h>
#include <dinput.h>
#include <dbt.h>
#include <windowsx.h>
#include <shellapi.h>

#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_GETDPISCALEDSIZE
#define WM_GETDPISCALEDSIZE 0x02e4
#endif

#ifndef DPI_ENUMS_DECLARED
typedef enum
{
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef enum
{
	MDT_EFFECTIVE_DPI = 0,
	MDT_ANGULAR_DPI = 1,
	MDT_RAW_DPI = 2,
	MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif 

#define IsWindows8OrGreater()                                         \
	_glfwIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN8),    \
	LOBYTE(_WIN32_WINNT_WIN8), 0)
#define IsWindows8Point1OrGreater()                                   \
	_glfwIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WINBLUE), \
	LOBYTE(_WIN32_WINNT_WINBLUE), 0)

#define _glfwIsWindows10Version1607OrGreaterWin32() \
	_glfwIsWindows10BuildOrGreaterWin32(14393)

#define _glfwIsWindows10Version1703OrGreaterWin32() \
	_glfwIsWindows10BuildOrGreaterWin32(15063)

#define GL_VERSION 0x1f02
#define GL_NONE 0
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_UNSIGNED_BYTE 0x1401
#define GL_EXTENSIONS 0x1f03
#define GL_NUM_EXTENSIONS 0x821d
#define GL_CONTEXT_FLAGS 0x821e
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define GL_CONTEXT_PROFILE_MASK 0x9126
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define GL_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define GL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define GL_NO_RESET_NOTIFICATION_ARB 0x8261
#define GL_CONTEXT_RELEASE_BEHAVIOR 0x82fb
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82fc
#define GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR 0x00000008

typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned char GLubyte;

typedef void (APIENTRY* PFNGLCLEARPROC)(GLbitfield);
typedef const GLubyte* (APIENTRY* PFNGLGETSTRINGPROC)(GLenum);
typedef void (APIENTRY* PFNGLGETINTEGERVPROC)(GLenum, GLint*);
typedef const GLubyte* (APIENTRY* PFNGLGETSTRINGIPROC)(GLenum, GLuint);

#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202b
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_RED_BITS_ARB 0x2015
#define WGL_RED_SHIFT_ARB 0x2016
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_GREEN_SHIFT_ARB 0x2018
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_BLUE_SHIFT_ARB 0x201a
#define WGL_ALPHA_BITS_ARB 0x201b
#define WGL_ALPHA_SHIFT_ARB 0x201c
#define WGL_ACCUM_BITS_ARB 0x201d
#define WGL_ACCUM_RED_BITS_ARB 0x201e
#define WGL_ACCUM_GREEN_BITS_ARB 0x201f
#define WGL_ACCUM_BLUE_BITS_ARB 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_AUX_BUFFERS_ARB 0x2024
#define WGL_STEREO_ARB 0x2012
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_SAMPLES_ARB 0x2042
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20a9
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
#define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
#define WGL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define WGL_NO_RESET_NOTIFICATION_ARB 0x8261
#define WGL_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
#define WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
#define WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
#define WGL_CONTEXT_OPENGL_NO_ERROR_ARB 0x31b3
#define WGL_COLORSPACE_EXT 0x309d
#define WGL_COLORSPACE_SRGB_EXT 0x3089

#define ERROR_INVALID_VERSION_ARB 0x2095
#define ERROR_INVALID_PROFILE_ARB 0x2096
#define ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB 0x2054

typedef UINT(WINAPI* PFN_GetDpiForWindow)(HWND);
typedef BOOL(WINAPI* PFN_AdjustWindowRectExForDpi)(LPRECT, DWORD, BOOL, DWORD, UINT);
typedef int (WINAPI* PFN_GetSystemMetricsForDpi)(int, UINT);
#define GetDpiForWindow _glfw.win32.user32.GetDpiForWindow_
#define AdjustWindowRectExForDpi _glfw.win32.user32.AdjustWindowRectExForDpi_

typedef HRESULT(WINAPI* PFN_DwmIsCompositionEnabled)(BOOL*);
typedef HRESULT(WINAPI* PFN_DwmFlush)(VOID);
typedef HRESULT(WINAPI* PFN_DwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND*);
typedef HRESULT(WINAPI* PFN_DwmGetColorizationColor)(DWORD*, BOOL*);
#define DwmIsCompositionEnabled _glfw.win32.dwmapi.IsCompositionEnabled
#define DwmFlush _glfw.win32.dwmapi.Flush
#define DwmEnableBlurBehindWindow _glfw.win32.dwmapi.EnableBlurBehindWindow
#define DwmGetColorizationColor _glfw.win32.dwmapi.GetColorizationColor

typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);
#define SetProcessDpiAwareness _glfw.win32.shcore.SetProcessDpiAwareness_
#define GetDpiForMonitor _glfw.win32.shcore.GetDpiForMonitor_

typedef LONG(WINAPI* PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*, ULONG, ULONGLONG);

typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
typedef BOOL(WINAPI* PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(HDC, int, int, UINT, const int*, int*);
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
#define wglSwapIntervalEXT _glfw.wgl.SwapIntervalEXT
#define wglGetPixelFormatAttribivARB _glfw.wgl.GetPixelFormatAttribivARB
#define wglGetExtensionsStringEXT _glfw.wgl.GetExtensionsStringEXT
#define wglGetExtensionsStringARB _glfw.wgl.GetExtensionsStringARB
#define wglCreateContextAttribsARB _glfw.wgl.CreateContextAttribsARB

typedef HGLRC(WINAPI* PFN_wglCreateContext)(HDC);
typedef BOOL(WINAPI* PFN_wglDeleteContext)(HGLRC);
typedef PROC(WINAPI* PFN_wglGetProcAddress)(LPCSTR);
typedef HDC(WINAPI* PFN_wglGetCurrentDC)(void);
typedef HGLRC(WINAPI* PFN_wglGetCurrentContext)(void);
typedef BOOL(WINAPI* PFN_wglMakeCurrent)(HDC, HGLRC);
typedef BOOL(WINAPI* PFN_wglShareLists)(HGLRC, HGLRC);
#define wglCreateContext _glfw.wgl.CreateContext
#define wglDeleteContext _glfw.wgl.DeleteContext
#define wglGetProcAddress _glfw.wgl.GetProcAddress
#define wglGetCurrentDC _glfw.wgl.GetCurrentDC
#define wglGetCurrentContext _glfw.wgl.GetCurrentContext
#define wglMakeCurrent _glfw.wgl.MakeCurrent
#define wglShareLists _glfw.wgl.ShareLists

typedef struct _GLFWcontextWGL
{
	HDC       dc;
	HGLRC     handle;
	int       interval;
} _GLFWcontextWGL;

typedef struct _GLFWlibraryWGL
{
	HINSTANCE                           instance;
	PFN_wglCreateContext                CreateContext;
	PFN_wglDeleteContext                DeleteContext;
	PFN_wglGetProcAddress               GetProcAddress;
	PFN_wglGetCurrentDC                 GetCurrentDC;
	PFN_wglGetCurrentContext            GetCurrentContext;
	PFN_wglMakeCurrent                  MakeCurrent;
	PFN_wglShareLists                   ShareLists;

	PFNWGLSWAPINTERVALEXTPROC           SwapIntervalEXT;
	PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
	PFNWGLGETEXTENSIONSSTRINGEXTPROC    GetExtensionsStringEXT;
	PFNWGLGETEXTENSIONSSTRINGARBPROC    GetExtensionsStringARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
	GLFWbool                            EXT_swap_control;
	GLFWbool                            EXT_colorspace;
	GLFWbool                            ARB_multisample;
	GLFWbool                            ARB_framebuffer_sRGB;
	GLFWbool                            EXT_framebuffer_sRGB;
	GLFWbool                            ARB_pixel_format;
	GLFWbool                            ARB_create_context;
	GLFWbool                            ARB_create_context_profile;
	GLFWbool                            EXT_create_context_es2_profile;
	GLFWbool                            ARB_create_context_robustness;
	GLFWbool                            ARB_create_context_no_error;
	GLFWbool                            ARB_context_flush_control;
} _GLFWlibraryWGL;

typedef struct _GLFWwindowWin32
{
	HWND                handle;
	HICON               bigIcon;
	HICON               smallIcon;

	GLFWbool            cursorTracked;
	GLFWbool            frameAction;
	GLFWbool            iconified;
	GLFWbool            maximized;

	GLFWbool            transparent;
	GLFWbool            scaleToMonitor;
	GLFWbool            keymenu;
	GLFWbool            showDefault;
	int                 width, height;
	int                 lastCursorPosX, lastCursorPosY;
	WCHAR               highSurrogate;
} _GLFWwindowWin32;

typedef struct _GLFWlibraryWin32
{
	HINSTANCE           instance;
	HWND                helperWindowHandle;
	ATOM                helperWindowClass;
	ATOM                mainWindowClass;
	HDEVNOTIFY          deviceNotificationHandle;
	int                 acquiredMonitorCount;
	char* clipboardString;
	short int           keycodes[512];
	short int           scancodes[GLFW_KEY_LAST + 1];
	char                keynames[GLFW_KEY_LAST + 1][5];

	double              restoreCursorPosX, restoreCursorPosY;

	_GLFWwindow* disabledCursorWindow;

	_GLFWwindow* capturedCursorWindow;
	RAWINPUT* rawInput;
	int                 rawInputSize;
	UINT                mouseTrailSize;

	HCURSOR             blankCursor;

	struct {
		HINSTANCE                       instance;
		PFN_GetDpiForWindow             GetDpiForWindow_;
		PFN_AdjustWindowRectExForDpi    AdjustWindowRectExForDpi_;
		PFN_GetSystemMetricsForDpi      GetSystemMetricsForDpi_;
	} user32;

	struct {
		HINSTANCE                       instance;
		PFN_DwmIsCompositionEnabled     IsCompositionEnabled;
		PFN_DwmFlush                    Flush;
		PFN_DwmEnableBlurBehindWindow   EnableBlurBehindWindow;
		PFN_DwmGetColorizationColor     GetColorizationColor;
	} dwmapi;

	struct {
		HINSTANCE                       instance;
		PFN_SetProcessDpiAwareness      SetProcessDpiAwareness_;
		PFN_GetDpiForMonitor            GetDpiForMonitor_;
	} shcore;

	struct {
		HINSTANCE                       instance;
		PFN_RtlVerifyVersionInfo        RtlVerifyVersionInfo_;
	} ntdll;
} _GLFWlibraryWin32;

typedef struct _GLFWmonitorWin32
{
	HMONITOR            handle;

	WCHAR               adapterName[32];
	WCHAR               displayName[32];
	char                publicAdapterName[32];
	char                publicDisplayName[32];
	GLFWbool            modesPruned;
	GLFWbool            modeChanged;
} _GLFWmonitorWin32;

typedef struct _GLFWcursorWin32
{
	HCURSOR             handle;
} _GLFWcursorWin32;

int _glfwInitWin32(void);
void _glfwTerminateWin32(void);

WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source);
char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source);
BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp);
BOOL _glfwIsWindows10BuildOrGreaterWin32(WORD build);
void _glfwInputErrorWin32(int error, const char* description);
void _glfwUpdateKeyNamesWin32(void);

void _glfwPollMonitorsWin32(void);
void _glfwSetVideoModeWin32(_GLFWmonitor* monitor, const GLFWvidmode* desired);
void _glfwRestoreVideoModeWin32(_GLFWmonitor* monitor);
void _glfwGetHMONITORContentScaleWin32(HMONITOR handle, float* xscale, float* yscale);

GLFWbool _glfwCreateWindowWin32(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);
void _glfwDestroyWindowWin32(_GLFWwindow* window);
void _glfwSetWindowTitleWin32(_GLFWwindow* window, const char* title);
void _glfwSetWindowIconWin32(_GLFWwindow* window, int count, const GLFWimage* images);
void _glfwGetWindowPosWin32(_GLFWwindow* window, int* xpos, int* ypos);
void _glfwSetWindowPosWin32(_GLFWwindow* window, int xpos, int ypos);
void _glfwGetWindowSizeWin32(_GLFWwindow* window, int* width, int* height);
void _glfwSetWindowSizeWin32(_GLFWwindow* window, int width, int height);
void _glfwSetWindowSizeLimitsWin32(_GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _glfwSetWindowAspectRatioWin32(_GLFWwindow* window, int numer, int denom);
void _glfwGetWindowFrameSizeWin32(_GLFWwindow* window, int* left, int* top, int* right, int* bottom);
void _glfwGetWindowContentScaleWin32(_GLFWwindow* window, float* xscale, float* yscale);
void _glfwIconifyWindowWin32(_GLFWwindow* window);
void _glfwRestoreWindowWin32(_GLFWwindow* window);
void _glfwMaximizeWindowWin32(_GLFWwindow* window);
void _glfwShowWindowWin32(_GLFWwindow* window);
void _glfwHideWindowWin32(_GLFWwindow* window);
void _glfwRequestWindowAttentionWin32(_GLFWwindow* window);
void _glfwFocusWindowWin32(_GLFWwindow* window);
void _glfwSetWindowMonitorWin32(_GLFWwindow* window, _GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
GLFWbool _glfwWindowFocusedWin32(_GLFWwindow* window);
GLFWbool _glfwWindowIconifiedWin32(_GLFWwindow* window);
GLFWbool _glfwWindowVisibleWin32(_GLFWwindow* window);
GLFWbool _glfwWindowMaximizedWin32(_GLFWwindow* window);
GLFWbool _glfwWindowHoveredWin32(_GLFWwindow* window);
GLFWbool _glfwFramebufferTransparentWin32(_GLFWwindow* window);
void _glfwSetWindowResizableWin32(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowDecoratedWin32(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowFloatingWin32(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowMousePassthroughWin32(_GLFWwindow* window, GLFWbool enabled);
float _glfwGetWindowOpacityWin32(_GLFWwindow* window);
void _glfwSetWindowOpacityWin32(_GLFWwindow* window, float opacity);

void _glfwSetRawMouseMotionWin32(_GLFWwindow* window, GLFWbool enabled);

void _glfwGetCursorPosWin32(_GLFWwindow* window, double* xpos, double* ypos);
void _glfwSetCursorPosWin32(_GLFWwindow* window, double xpos, double ypos);
void _glfwSetCursorModeWin32(_GLFWwindow* window, int mode);
const char* _glfwGetScancodeNameWin32(int scancode);
int _glfwGetKeyScancodeWin32(int key);
GLFWbool _glfwCreateCursorWin32(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot);
GLFWbool _glfwCreateStandardCursorWin32(_GLFWcursor* cursor, int shape);
void _glfwDestroyCursorWin32(_GLFWcursor* cursor);
void _glfwSetCursorWin32(_GLFWwindow* window, _GLFWcursor* cursor);
void _glfwSetClipboardStringWin32(const char* string);
const char* _glfwGetClipboardStringWin32(void);

void _glfwGetMonitorPosWin32(_GLFWmonitor* monitor, int* xpos, int* ypos);
void _glfwGetMonitorContentScaleWin32(_GLFWmonitor* monitor, float* xscale, float* yscale);
void _glfwGetMonitorWorkareaWin32(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GLFWvidmode* _glfwGetVideoModesWin32(_GLFWmonitor* monitor, int* count);
GLFWbool _glfwGetVideoModeWin32(_GLFWmonitor* monitor, GLFWvidmode* mode);
GLFWbool _glfwGetGammaRampWin32(_GLFWmonitor* monitor, GLFWgammaramp* ramp);
void _glfwSetGammaRampWin32(_GLFWmonitor* monitor, const GLFWgammaramp* ramp);

GLFWbool _glfwInitWGL(void);
void _glfwTerminateWGL(void);
GLFWbool _glfwCreateContextWGL(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);

typedef struct _GLFWtlsWin32
{
	GLFWbool            allocated;
	DWORD               index;
} _GLFWtlsWin32;

typedef struct _GLFWmutexWin32
{
	GLFWbool            allocated;
	CRITICAL_SECTION    section;
} _GLFWmutexWin32;

#define _GLFW_REQUIRE_INIT()                         \
	if (!_glfw.initialized)                          \
	{                                                \
		_glfwInputError(GLFW_NOT_INITIALIZED, NULL); \
		return;                                      \
	}
#define _GLFW_REQUIRE_INIT_OR_RETURN(x)              \
	if (!_glfw.initialized)                          \
	{                                                \
		_glfwInputError(GLFW_NOT_INITIALIZED, NULL); \
		return x;                                    \
	}

#define _GLFW_SWAP(type, x, y) \
	{                          \
		type t;                \
		t = x;                 \
		x = y;                 \
		y = t;                 \
	}

struct _GLFWerror
{
	_GLFWerror* next;
	int             code;
	char            description[_GLFW_MESSAGE_SIZE];
};

struct _GLFWwndconfig
{
	int           xpos;
	int           ypos;
	int           width;
	int           height;
	GLFWbool      resizable;
	GLFWbool      visible;
	GLFWbool      decorated;
	GLFWbool      focused;
	GLFWbool      autoIconify;
	GLFWbool      floating;
	GLFWbool      maximized;
	GLFWbool      centerCursor;
	GLFWbool      focusOnShow;
	GLFWbool      mousePassthrough;
	GLFWbool      scaleToMonitor;
	GLFWbool      scaleFramebuffer;
	struct {
		GLFWbool  keymenu;
		GLFWbool  showDefault;
	} win32;
};

struct _GLFWctxconfig
{
	int           major;
	int           minor;
	GLFWbool      forward;
	GLFWbool      debug;
	GLFWbool      noerror;
	int           profile;
	int           robustness;
	int           release;
	_GLFWwindow* share;
};

struct _GLFWfbconfig
{
	int         redBits;
	int         greenBits;
	int         blueBits;
	int         alphaBits;
	int         depthBits;
	int         stencilBits;
	int         accumRedBits;
	int         accumGreenBits;
	int         accumBlueBits;
	int         accumAlphaBits;
	int         auxBuffers;
	GLFWbool    stereo;
	int         samples;
	GLFWbool    sRGB;
	GLFWbool    doublebuffer;
	GLFWbool    transparent;
	uintptr_t   handle;
};

struct _GLFWcontext
{
	int                 major, minor, revision;
	GLFWbool            forward, debug, noerror;
	int                 profile;
	int                 robustness;
	int                 release;

	PFNGLGETSTRINGIPROC  GetStringi;
	PFNGLGETINTEGERVPROC GetIntegerv;
	PFNGLGETSTRINGPROC   GetString;

	void (*makeCurrent)(_GLFWwindow*);
	void (*swapBuffers)(_GLFWwindow*);
	void (*swapInterval)(int);
	int (*extensionSupported)(const char*);
	GLFWglproc(*getProcAddress)(const char*);
	void (*destroy)(_GLFWwindow*);

	_GLFWcontextWGL wgl;
};

struct _GLFWwindow
{
	struct _GLFWwindow* next;

	GLFWbool            resizable;
	GLFWbool            decorated;
	GLFWbool            autoIconify;
	GLFWbool            floating;
	GLFWbool            focusOnShow;
	GLFWbool            mousePassthrough;
	GLFWbool            shouldClose;
	void* userPointer;
	GLFWbool            doublebuffer;
	GLFWvidmode         videoMode;
	_GLFWmonitor* monitor;
	_GLFWcursor* cursor;
	char* title;

	int                 minwidth, minheight;
	int                 maxwidth, maxheight;
	int                 numer, denom;

	GLFWbool            stickyKeys;
	GLFWbool            stickyMouseButtons;
	GLFWbool            lockKeyMods;
	GLFWbool            disableMouseButtonLimit;
	int                 cursorMode;
	char                mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
	char                keys[GLFW_KEY_LAST + 1];

	double              virtualCursorPosX, virtualCursorPosY;
	GLFWbool            rawMouseMotion;

	_GLFWcontext        context;

	struct {
		GLFWwindowposfun          pos;
		GLFWwindowsizefun         size;
		GLFWwindowclosefun        close;
		GLFWwindowrefreshfun      refresh;
		GLFWwindowfocusfun        focus;
		GLFWwindowiconifyfun      iconify;
		GLFWwindowmaximizefun     maximize;
		GLFWframebuffersizefun    fbsize;
		GLFWwindowcontentscalefun scale;
		GLFWmousebuttonfun        mouseButton;
		GLFWcursorposfun          cursorPos;
		GLFWcursorenterfun        cursorEnter;
		GLFWscrollfun             scroll;
		GLFWkeyfun                key;
		GLFWcharfun               character;
		GLFWcharmodsfun           charmods;
		GLFWdropfun               drop;
	} callbacks;
	
	_GLFWwindowWin32  win32;
};

struct _GLFWmonitor
{
	char            name[128];
	void* userPointer;

	int             widthMM, heightMM;

	_GLFWwindow* window;

	GLFWvidmode* modes;
	int             modeCount;
	GLFWvidmode     currentMode;

	GLFWgammaramp   originalRamp;
	GLFWgammaramp   currentRamp;

	_GLFWmonitorWin32 win32;
};

struct _GLFWcursor
{
	_GLFWcursor* next;
	_GLFWcursorWin32  win32;
};

struct _GLFWtls
{
	_GLFWtlsWin32     win32;
};

struct _GLFWmutex
{
	_GLFWmutexWin32   win32;
};

struct _GLFWlibrary
{
	GLFWbool            initialized;

	struct {
		_GLFWfbconfig   framebuffer;
		_GLFWwndconfig  window;
		_GLFWctxconfig  context;
		int             refreshRate;
	} hints;

	_GLFWerror* errorListHead;
	_GLFWcursor* cursorListHead;
	_GLFWwindow* windowListHead;

	_GLFWmonitor** monitors;
	int                 monitorCount;

	_GLFWtls            errorSlot;
	_GLFWtls            contextSlot;
	_GLFWmutex          errorLock;

	struct {
		uint64_t        offset;
		uint64_t        frequency;
	} timer;

	struct {
		GLFWmonitorfun  monitor;
	} callbacks;

	_GLFWlibraryWin32 win32;
	_GLFWlibraryWGL wgl;
};

extern _GLFWlibrary _glfw;

void _glfwPlatformInitTimer(void);
uint64_t _glfwPlatformGetTimerValue(void);
uint64_t _glfwPlatformGetTimerFrequency(void);

GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls);
void _glfwPlatformDestroyTls(_GLFWtls* tls);
void* _glfwPlatformGetTls(_GLFWtls* tls);
void _glfwPlatformSetTls(_GLFWtls* tls, void* value);

GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex);
void _glfwPlatformDestroyMutex(_GLFWmutex* mutex);
void _glfwPlatformLockMutex(_GLFWmutex* mutex);
void _glfwPlatformUnlockMutex(_GLFWmutex* mutex);

void* _glfwPlatformLoadModule(const char* path);
void _glfwPlatformFreeModule(void* module);
GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name);

void _glfwInputWindowFocus(_GLFWwindow* window, GLFWbool focused);
void _glfwInputWindowPos(_GLFWwindow* window, int xpos, int ypos);
void _glfwInputWindowSize(_GLFWwindow* window, int width, int height);
void _glfwInputFramebufferSize(_GLFWwindow* window, int width, int height);
void _glfwInputWindowContentScale(_GLFWwindow* window,
	float xscale, float yscale);
void _glfwInputWindowIconify(_GLFWwindow* window, GLFWbool iconified);
void _glfwInputWindowMaximize(_GLFWwindow* window, GLFWbool maximized);
void _glfwInputWindowDamage(_GLFWwindow* window);
void _glfwInputWindowCloseRequest(_GLFWwindow* window);
void _glfwInputWindowMonitor(_GLFWwindow* window, _GLFWmonitor* monitor);

void _glfwInputKey(_GLFWwindow* window,
	int key, int scancode, int action, int mods);
void _glfwInputChar(_GLFWwindow* window,
	uint32_t codepoint, int mods, GLFWbool plain);
void _glfwInputScroll(_GLFWwindow* window, double xoffset, double yoffset);
void _glfwInputMouseClick(_GLFWwindow* window, int button, int action, int mods);
void _glfwInputCursorPos(_GLFWwindow* window, double xpos, double ypos);
void _glfwInputCursorEnter(_GLFWwindow* window, GLFWbool entered);
void _glfwInputDrop(_GLFWwindow* window, int count, const char** names);

void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement);
void _glfwInputMonitorWindow(_GLFWmonitor* monitor, _GLFWwindow* window);

void _glfwInputError(int code, const char* format, ...);

GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions);
const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* desired,
	const _GLFWfbconfig* alternatives,
	unsigned int count);
GLFWbool _glfwRefreshContextAttribs(_GLFWwindow* window,
	const _GLFWctxconfig* ctxconfig);
GLFWbool _glfwIsValidContextConfig(const _GLFWctxconfig* ctxconfig);

const GLFWvidmode* _glfwChooseVideoMode(_GLFWmonitor* monitor,
	const GLFWvidmode* desired);
int _glfwCompareVideoModes(const GLFWvidmode* first, const GLFWvidmode* second);
_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM);
void _glfwFreeMonitor(_GLFWmonitor* monitor);
void _glfwAllocGammaArrays(GLFWgammaramp* ramp, unsigned int size);
void _glfwFreeGammaArrays(GLFWgammaramp* ramp);
void _glfwSplitBPP(int bpp, int* red, int* green, int* blue);

void _glfwCenterCursorInContentArea(_GLFWwindow* window);

size_t _glfwEncodeUTF8(char* s, uint32_t codepoint);

char* _glfw_strdup(const char* source);
int _glfw_min(int a, int b);
int _glfw_max(int a, int b);

void* _glfw_calloc(size_t count, size_t size);
void* _glfw_realloc(void* pointer, size_t size);
void _glfw_free(void* pointer);

GLFWbool _glfwIsValidContextConfig(const _GLFWctxconfig* ctxconfig)
{
	{
		if ((ctxconfig->major < 1 || ctxconfig->minor < 0) ||
			(ctxconfig->major == 1 && ctxconfig->minor > 5) ||
			(ctxconfig->major == 2 && ctxconfig->minor > 1) ||
			(ctxconfig->major == 3 && ctxconfig->minor > 3))
		{
			_glfwInputError(GLFW_INVALID_VALUE,
				"Invalid OpenGL version %i.%i",
				ctxconfig->major, ctxconfig->minor);
			return GLFW_FALSE;
		}

		if (ctxconfig->profile)
		{
			if (ctxconfig->profile != GLFW_OPENGL_CORE_PROFILE &&
				ctxconfig->profile != GLFW_OPENGL_COMPAT_PROFILE)
			{
				_glfwInputError(GLFW_INVALID_ENUM,
					"Invalid OpenGL profile 0x%08X",
					ctxconfig->profile);
				return GLFW_FALSE;
			}

			if (ctxconfig->major <= 2 ||
				(ctxconfig->major == 3 && ctxconfig->minor < 2))
			{
				_glfwInputError(GLFW_INVALID_VALUE,
					"Context profiles are only defined for OpenGL version 3.2 and above");
				return GLFW_FALSE;
			}
		}

		if (ctxconfig->forward && ctxconfig->major <= 2)
		{

			_glfwInputError(GLFW_INVALID_VALUE,
				"Forward-compatibility is only defined for OpenGL version 3.0 and above");
			return GLFW_FALSE;
		}
	}
	if (ctxconfig->robustness)
	{
		if (ctxconfig->robustness != GLFW_NO_RESET_NOTIFICATION &&
			ctxconfig->robustness != GLFW_LOSE_CONTEXT_ON_RESET)
		{
			_glfwInputError(GLFW_INVALID_ENUM,
				"Invalid context robustness mode 0x%08X",
				ctxconfig->robustness);
			return GLFW_FALSE;
		}
	}

	if (ctxconfig->release)
	{
		if (ctxconfig->release != GLFW_RELEASE_BEHAVIOR_NONE &&
			ctxconfig->release != GLFW_RELEASE_BEHAVIOR_FLUSH)
		{
			_glfwInputError(GLFW_INVALID_ENUM,
				"Invalid context release behavior 0x%08X",
				ctxconfig->release);
			return GLFW_FALSE;
		}
	}

	return GLFW_TRUE;
}

const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* desired,
	const _GLFWfbconfig* alternatives,
	unsigned int count)
{
	unsigned int i;
	unsigned int missing, leastMissing = UINT_MAX;
	unsigned int colorDiff, leastColorDiff = UINT_MAX;
	unsigned int extraDiff, leastExtraDiff = UINT_MAX;
	const _GLFWfbconfig* current;
	const _GLFWfbconfig* closest = NULL;

	for (i = 0; i < count; i++)
	{
		current = alternatives + i;

		if (desired->stereo > 0 && current->stereo == 0)
		{

			continue;
		}

		{
			missing = 0;

			if (desired->alphaBits > 0 && current->alphaBits == 0)
				missing++;

			if (desired->depthBits > 0 && current->depthBits == 0)
				missing++;

			if (desired->stencilBits > 0 && current->stencilBits == 0)
				missing++;

			if (desired->auxBuffers > 0 &&
				current->auxBuffers < desired->auxBuffers)
			{
				missing += desired->auxBuffers - current->auxBuffers;
			}

			if (desired->samples > 0 && current->samples == 0)
			{
				missing++;
			}

			if (desired->transparent != current->transparent)
				missing++;
		}

		{
			colorDiff = 0;

			if (desired->redBits != GLFW_DONT_CARE)
			{
				colorDiff += (desired->redBits - current->redBits) *
					(desired->redBits - current->redBits);
			}

			if (desired->greenBits != GLFW_DONT_CARE)
			{
				colorDiff += (desired->greenBits - current->greenBits) *
					(desired->greenBits - current->greenBits);
			}

			if (desired->blueBits != GLFW_DONT_CARE)
			{
				colorDiff += (desired->blueBits - current->blueBits) *
					(desired->blueBits - current->blueBits);
			}
		}

		{
			extraDiff = 0;

			if (desired->alphaBits != GLFW_DONT_CARE)
			{
				extraDiff += (desired->alphaBits - current->alphaBits) *
					(desired->alphaBits - current->alphaBits);
			}

			if (desired->depthBits != GLFW_DONT_CARE)
			{
				extraDiff += (desired->depthBits - current->depthBits) *
					(desired->depthBits - current->depthBits);
			}

			if (desired->stencilBits != GLFW_DONT_CARE)
			{
				extraDiff += (desired->stencilBits - current->stencilBits) *
					(desired->stencilBits - current->stencilBits);
			}

			if (desired->accumRedBits != GLFW_DONT_CARE)
			{
				extraDiff += (desired->accumRedBits - current->accumRedBits) *
					(desired->accumRedBits - current->accumRedBits);
			}

			if (desired->accumGreenBits != GLFW_DONT_CARE)
			{
				extraDiff += (desired->accumGreenBits - current->accumGreenBits) *
					(desired->accumGreenBits - current->accumGreenBits);
			}

			if (desired->accumBlueBits != GLFW_DONT_CARE)
			{
				extraDiff += (desired->accumBlueBits - current->accumBlueBits) *
					(desired->accumBlueBits - current->accumBlueBits);
			}

			if (desired->accumAlphaBits != GLFW_DONT_CARE)
			{
				extraDiff += (desired->accumAlphaBits - current->accumAlphaBits) *
					(desired->accumAlphaBits - current->accumAlphaBits);
			}

			if (desired->samples != GLFW_DONT_CARE)
			{
				extraDiff += (desired->samples - current->samples) *
					(desired->samples - current->samples);
			}

			if (desired->sRGB && !current->sRGB)
				extraDiff++;
		}

		if (missing < leastMissing)
			closest = current;
		else if (missing == leastMissing)
		{
			if ((colorDiff < leastColorDiff) ||
				(colorDiff == leastColorDiff && extraDiff < leastExtraDiff))
			{
				closest = current;
			}
		}

		if (current == closest)
		{
			leastMissing = missing;
			leastColorDiff = colorDiff;
			leastExtraDiff = extraDiff;
		}
	}

	return closest;
}

GLFWbool _glfwRefreshContextAttribs(_GLFWwindow* window,
	const _GLFWctxconfig* ctxconfig)
{
	int i;
	_GLFWwindow* previous;
	const char* version;

	previous = _glfwPlatformGetTls(&_glfw.contextSlot);
	glfwMakeContextCurrent((GLFWwindow*)window);
	if (_glfwPlatformGetTls(&_glfw.contextSlot) != window)
		return GLFW_FALSE;

	window->context.GetIntegerv = (PFNGLGETINTEGERVPROC)
		window->context.getProcAddress("glGetIntegerv");
	window->context.GetString = (PFNGLGETSTRINGPROC)
		window->context.getProcAddress("glGetString");
	if (!window->context.GetIntegerv || !window->context.GetString)
	{
		_glfwInputError(GLFW_PLATFORM_ERROR, "Entry point retrieval is broken");
		glfwMakeContextCurrent((GLFWwindow*)previous);
		return GLFW_FALSE;
	}

	version = (const char*)window->context.GetString(GL_VERSION);
	if (!version)
	{
		{
			_glfwInputError(GLFW_PLATFORM_ERROR,
				"OpenGL version string retrieval is broken");
		}

		glfwMakeContextCurrent((GLFWwindow*)previous);
		return GLFW_FALSE;
	}

	if (!sscanf(version, "%d.%d.%d",
		&window->context.major,
		&window->context.minor,
		&window->context.revision))
	{
		_glfwInputError(GLFW_PLATFORM_ERROR, "No version found in OpenGL version string");

		glfwMakeContextCurrent((GLFWwindow*)previous);
		return GLFW_FALSE;
	}

	if (window->context.major < ctxconfig->major ||
		(window->context.major == ctxconfig->major &&
			window->context.minor < ctxconfig->minor))
	{
		_glfwInputError(GLFW_VERSION_UNAVAILABLE,
			"Requested OpenGL version %i.%i, got version %i.%i",
			ctxconfig->major, ctxconfig->minor,
			window->context.major, window->context.minor);

		glfwMakeContextCurrent((GLFWwindow*)previous);
		return GLFW_FALSE;
	}

	if (window->context.major >= 3)
	{
		window->context.GetStringi = (PFNGLGETSTRINGIPROC)
			window->context.getProcAddress("glGetStringi");
		if (!window->context.GetStringi)
		{
			_glfwInputError(GLFW_PLATFORM_ERROR,
				"Entry point retrieval is broken");
			glfwMakeContextCurrent((GLFWwindow*)previous);
			return GLFW_FALSE;
		}
	}

	if (window->context.major >= 3)
	{
		GLint flags;
		window->context.GetIntegerv(GL_CONTEXT_FLAGS, &flags);

		if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
			window->context.forward = GLFW_TRUE;

		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
			window->context.debug = GLFW_TRUE;
		else if (glfwExtensionSupported("GL_ARB_debug_output") &&
			ctxconfig->debug)
		{
			window->context.debug = GLFW_TRUE;
		}

		if (flags & GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR)
			window->context.noerror = GLFW_TRUE;
	}

	if (window->context.major >= 4 ||
		(window->context.major == 3 && window->context.minor >= 2))
	{
		GLint mask;
		window->context.GetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);

		if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
			window->context.profile = GLFW_OPENGL_COMPAT_PROFILE;
		else if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
			window->context.profile = GLFW_OPENGL_CORE_PROFILE;
		else if (glfwExtensionSupported("GL_ARB_compatibility"))
		{
			window->context.profile = GLFW_OPENGL_COMPAT_PROFILE;
		}
	}

	if (glfwExtensionSupported("GL_ARB_robustness"))
	{
		GLint strategy;
		window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB,
			&strategy);

		if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
			window->context.robustness = GLFW_LOSE_CONTEXT_ON_RESET;
		else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
			window->context.robustness = GLFW_NO_RESET_NOTIFICATION;
	}

	if (glfwExtensionSupported("GL_KHR_context_flush_control"))
	{
		GLint behavior;
		window->context.GetIntegerv(GL_CONTEXT_RELEASE_BEHAVIOR, &behavior);

		if (behavior == GL_NONE)
			window->context.release = GLFW_RELEASE_BEHAVIOR_NONE;
		else if (behavior == GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH)
			window->context.release = GLFW_RELEASE_BEHAVIOR_FLUSH;
	}

	{
		PFNGLCLEARPROC glClear = (PFNGLCLEARPROC)
			window->context.getProcAddress("glClear");
		glClear(GL_COLOR_BUFFER_BIT);

		if (window->doublebuffer)
			window->context.swapBuffers(window);
	}

	glfwMakeContextCurrent((GLFWwindow*)previous);
	return GLFW_TRUE;
}

GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions)
{
	const char* start = extensions;

	for (;;)
	{
		const char* where;
		const char* terminator;

		where = strstr(start, string);
		if (!where)
			return GLFW_FALSE;

		terminator = where + strlen(string);
		if (where == start || *(where - 1) == ' ')
		{
			if (*terminator == ' ' || *terminator == '\0')
				break;
		}

		start = terminator;
	}

	return GLFW_TRUE;
}

void glfwMakeContextCurrent(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	_GLFWwindow* previous;

	previous = _glfwPlatformGetTls(&_glfw.contextSlot);

	if (previous)
	{
		if (!window)
			previous->context.makeCurrent(NULL);
	}

	if (window)
		window->context.makeCurrent(window);
}

GLFWwindow* glfwGetCurrentContext(void)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);
	return _glfwPlatformGetTls(&_glfw.contextSlot);
}

void glfwSwapBuffers(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	window->context.swapBuffers(window);
}

void glfwSwapInterval(int interval)
{
	_GLFWwindow* window;

	_GLFW_REQUIRE_INIT();

	window = _glfwPlatformGetTls(&_glfw.contextSlot);
	if (!window)
	{
		_glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot set swap interval without a current OpenGL or OpenGL ES context");
		return;
	}

	window->context.swapInterval(interval);
}

int glfwExtensionSupported(const char* extension)
{
	_GLFWwindow* window;
	assert(extension != NULL);

	_GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);

	window = _glfwPlatformGetTls(&_glfw.contextSlot);
	if (!window)
	{
		_glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot query extension without a current OpenGL or OpenGL ES context");
		return GLFW_FALSE;
	}

	if (*extension == '\0')
	{
		_glfwInputError(GLFW_INVALID_VALUE, "Extension name cannot be an empty string");
		return GLFW_FALSE;
	}

	if (window->context.major >= 3)
	{
		int i;
		GLint count;
		window->context.GetIntegerv(GL_NUM_EXTENSIONS, &count);

		for (i = 0; i < count; i++)
		{
			const char* en = (const char*)
				window->context.GetStringi(GL_EXTENSIONS, i);
			if (!en)
			{
				_glfwInputError(GLFW_PLATFORM_ERROR,
					"Extension string retrieval is broken");
				return GLFW_FALSE;
			}

			if (strcmp(en, extension) == 0)
				return GLFW_TRUE;
		}
	}
	else
	{
		const char* extensions = (const char*)
			window->context.GetString(GL_EXTENSIONS);
		if (!extensions)
		{
			_glfwInputError(GLFW_PLATFORM_ERROR, "Extension string retrieval is broken");
			return GLFW_FALSE;
		}

		if (_glfwStringInExtensionString(extension, extensions))
			return GLFW_TRUE;
	}

	return window->context.extensionSupported(extension);
}

GLFWglproc glfwGetProcAddress(const char* procname)
{
	_GLFWwindow* window;
	assert(procname != NULL);

	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	window = _glfwPlatformGetTls(&_glfw.contextSlot);
	if (!window)
	{
		_glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot query entry point without a current OpenGL or OpenGL ES context");
		return NULL;
	}

	return window->context.getProcAddress(procname);
}

_GLFWlibrary _glfw = { GLFW_FALSE };

static _GLFWerror _glfwMainThreadError;
static GLFWerrorfun _glfwErrorCallback;

static void terminate(void)
{
	int i;

	memset(&_glfw.callbacks, 0, sizeof(_glfw.callbacks));

	while (_glfw.windowListHead)
		glfwDestroyWindow((GLFWwindow*)_glfw.windowListHead);

	while (_glfw.cursorListHead)
		glfwDestroyCursor((GLFWcursor*)_glfw.cursorListHead);

	for (i = 0; i < _glfw.monitorCount; i++)
	{
		_GLFWmonitor* monitor = _glfw.monitors[i];
		if (monitor->originalRamp.size)
			_glfwSetGammaRampWin32(monitor, &monitor->originalRamp);
		_glfwFreeMonitor(monitor);
	}

	_glfw_free(_glfw.monitors);
	_glfw.monitors = NULL;
	_glfw.monitorCount = 0;

	_glfwTerminateWin32();

	_glfw.initialized = GLFW_FALSE;

	while (_glfw.errorListHead)
	{
		_GLFWerror* error = _glfw.errorListHead;
		_glfw.errorListHead = error->next;
		_glfw_free(error);
	}

	_glfwPlatformDestroyTls(&_glfw.contextSlot);
	_glfwPlatformDestroyTls(&_glfw.errorSlot);
	_glfwPlatformDestroyMutex(&_glfw.errorLock);

	memset(&_glfw, 0, sizeof(_glfw));
}

size_t _glfwEncodeUTF8(char* s, uint32_t codepoint)
{
	size_t count = 0;

	if (codepoint < 0x80)
		s[count++] = (char)codepoint;
	else if (codepoint < 0x800)
	{
		s[count++] = (codepoint >> 6) | 0xc0;
		s[count++] = (codepoint & 0x3f) | 0x80;
	}
	else if (codepoint < 0x10000)
	{
		s[count++] = (codepoint >> 12) | 0xe0;
		s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
		s[count++] = (codepoint & 0x3f) | 0x80;
	}
	else if (codepoint < 0x110000)
	{
		s[count++] = (codepoint >> 18) | 0xf0;
		s[count++] = ((codepoint >> 12) & 0x3f) | 0x80;
		s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
		s[count++] = (codepoint & 0x3f) | 0x80;
	}

	return count;
}

char* _glfw_strdup(const char* source)
{
	const size_t length = strlen(source);
	char* result = _glfw_calloc(length + 1, 1);
	strcpy(result, source);
	return result;
}

int _glfw_min(int a, int b)
{
	return a < b ? a : b;
}

int _glfw_max(int a, int b)
{
	return a > b ? a : b;
}

void* _glfw_calloc(size_t count, size_t size)
{
	if (count && size)
	{
		void* block;

		if (count > SIZE_MAX / size)
		{
			_glfwInputError(GLFW_INVALID_VALUE, "Allocation size overflow");
			return NULL;
		}

		block = malloc(count * size);
		if (block)
			return memset(block, 0, count * size);
		else
		{
			_glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
			return NULL;
		}
	}
	else
		return NULL;
}

void* _glfw_realloc(void* block, size_t size)
{
	if (block && size)
	{
		void* resized = realloc(block, size);
		if (resized)
			return resized;
		else
		{
			_glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
			return NULL;
		}
	}
	else if (block)
	{
		_glfw_free(block);
		return NULL;
	}
	else
		return _glfw_calloc(1, size);
}

void _glfw_free(void* block)
{
	if (block)
		free(block);
}

void _glfwInputError(int code, const char* format, ...)
{
	_GLFWerror* error;
	char description[_GLFW_MESSAGE_SIZE];

	if (format)
	{
		va_list vl;

		va_start(vl, format);
		vsnprintf(description, sizeof(description), format, vl);
		va_end(vl);

		description[sizeof(description) - 1] = '\0';
	}
	else
	{
		if (code == GLFW_NOT_INITIALIZED)
			strcpy(description, "The GLFW library is not initialized");
		else if (code == GLFW_NO_CURRENT_CONTEXT)
			strcpy(description, "There is no current context");
		else if (code == GLFW_INVALID_ENUM)
			strcpy(description, "Invalid argument for enum parameter");
		else if (code == GLFW_INVALID_VALUE)
			strcpy(description, "Invalid value for parameter");
		else if (code == GLFW_OUT_OF_MEMORY)
			strcpy(description, "Out of memory");
		else if (code == GLFW_API_UNAVAILABLE)
			strcpy(description, "The requested API is unavailable");
		else if (code == GLFW_VERSION_UNAVAILABLE)
			strcpy(description, "The requested API version is unavailable");
		else if (code == GLFW_PLATFORM_ERROR)
			strcpy(description, "A platform-specific error occurred");
		else if (code == GLFW_FORMAT_UNAVAILABLE)
			strcpy(description, "The requested format is unavailable");
		else
			strcpy(description, "ERROR: UNKNOWN GLFW ERROR");
	}

	if (_glfw.initialized)
	{
		error = _glfwPlatformGetTls(&_glfw.errorSlot);
		if (!error)
		{
			error = _glfw_calloc(1, sizeof(_GLFWerror));
			_glfwPlatformSetTls(&_glfw.errorSlot, error);
			_glfwPlatformLockMutex(&_glfw.errorLock);
			error->next = _glfw.errorListHead;
			_glfw.errorListHead = error;
			_glfwPlatformUnlockMutex(&_glfw.errorLock);
		}
	}
	else
		error = &_glfwMainThreadError;

	error->code = code;
	strcpy(error->description, description);

	if (_glfwErrorCallback)
		_glfwErrorCallback(code, description);
}

int glfwInit(void)
{
	if (_glfw.initialized)
		return GLFW_TRUE;

	memset(&_glfw, 0, sizeof(_glfw));

	if (!_glfwInitWin32())
	{
		terminate();
		return GLFW_FALSE;
	}

	if (!_glfwPlatformCreateMutex(&_glfw.errorLock) ||
		!_glfwPlatformCreateTls(&_glfw.errorSlot) ||
		!_glfwPlatformCreateTls(&_glfw.contextSlot))
	{
		terminate();
		return GLFW_FALSE;
	}

	_glfwPlatformSetTls(&_glfw.errorSlot, &_glfwMainThreadError);

	_glfwPlatformInitTimer();
	_glfw.timer.offset = _glfwPlatformGetTimerValue();

	_glfw.initialized = GLFW_TRUE;

	glfwDefaultWindowHints();
	return GLFW_TRUE;
}

void glfwTerminate(void)
{
	if (!_glfw.initialized)
		return;
	terminate();
}

void glfwSetErrorCallback(GLFWerrorfun cbfun)
{
	_glfwErrorCallback = cbfun;
}

#define _GLFW_STICK 3

#define GLFW_MOD_MASK (GLFW_MOD_SHIFT | \
					GLFW_MOD_CONTROL | \
					GLFW_MOD_ALT | \
					GLFW_MOD_SUPER | \
					GLFW_MOD_CAPS_LOCK | \
					GLFW_MOD_NUM_LOCK)

void _glfwInputKey(_GLFWwindow* window, int key, int scancode, int action, int mods)
{
	assert(window != NULL);
	assert(key >= 0 || key == GLFW_KEY_UNKNOWN);
	assert(key <= GLFW_KEY_LAST);
	assert(action == GLFW_PRESS || action == GLFW_RELEASE);
	assert(mods == (mods & GLFW_MOD_MASK));

	if (key >= 0 && key <= GLFW_KEY_LAST)
	{
		GLFWbool repeated = GLFW_FALSE;

		if (action == GLFW_RELEASE && window->keys[key] == GLFW_RELEASE)
			return;

		if (action == GLFW_PRESS && window->keys[key] == GLFW_PRESS)
			repeated = GLFW_TRUE;

		if (action == GLFW_RELEASE && window->stickyKeys)
			window->keys[key] = _GLFW_STICK;
		else
			window->keys[key] = (char)action;

		if (repeated)
			action = GLFW_REPEAT;
	}

	if (!window->lockKeyMods)
		mods &= ~(GLFW_MOD_CAPS_LOCK | GLFW_MOD_NUM_LOCK);

	if (window->callbacks.key)
		window->callbacks.key((GLFWwindow*)window, key, scancode, action, mods);
}

void _glfwInputChar(_GLFWwindow* window, uint32_t codepoint, int mods, GLFWbool plain)
{
	assert(window != NULL);
	assert(mods == (mods & GLFW_MOD_MASK));
	assert(plain == GLFW_TRUE || plain == GLFW_FALSE);

	if (codepoint < 32 || (codepoint > 126 && codepoint < 160))
		return;

	if (!window->lockKeyMods)
		mods &= ~(GLFW_MOD_CAPS_LOCK | GLFW_MOD_NUM_LOCK);

	if (window->callbacks.charmods)
		window->callbacks.charmods((GLFWwindow*)window, codepoint, mods);

	if (plain)
	{
		if (window->callbacks.character)
			window->callbacks.character((GLFWwindow*)window, codepoint);
	}
}

void _glfwInputScroll(_GLFWwindow* window, double xoffset, double yoffset)
{
	assert(window != NULL);
	assert(xoffset > -FLT_MAX);
	assert(xoffset < FLT_MAX);
	assert(yoffset > -FLT_MAX);
	assert(yoffset < FLT_MAX);

	if (window->callbacks.scroll)
		window->callbacks.scroll((GLFWwindow*)window, xoffset, yoffset);
}

void _glfwInputMouseClick(_GLFWwindow* window, int button, int action, int mods)
{
	assert(window != NULL);
	assert(button >= 0);
	assert(action == GLFW_PRESS || action == GLFW_RELEASE);
	assert(mods == (mods & GLFW_MOD_MASK));

	if (button < 0 || (!window->disableMouseButtonLimit && button > GLFW_MOUSE_BUTTON_LAST))
		return;

	if (!window->lockKeyMods)
		mods &= ~(GLFW_MOD_CAPS_LOCK | GLFW_MOD_NUM_LOCK);

	if (button <= GLFW_MOUSE_BUTTON_LAST)
	{
		if (action == GLFW_RELEASE && window->stickyMouseButtons)
			window->mouseButtons[button] = _GLFW_STICK;
		else
			window->mouseButtons[button] = (char)action;
	}

	if (window->callbacks.mouseButton)
		window->callbacks.mouseButton((GLFWwindow*)window, button, action, mods);
}

void _glfwInputCursorPos(_GLFWwindow* window, double xpos, double ypos)
{
	assert(window != NULL);
	assert(xpos > -FLT_MAX);
	assert(xpos < FLT_MAX);
	assert(ypos > -FLT_MAX);
	assert(ypos < FLT_MAX);

	if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos)
		return;

	window->virtualCursorPosX = xpos;
	window->virtualCursorPosY = ypos;

	if (window->callbacks.cursorPos)
		window->callbacks.cursorPos((GLFWwindow*)window, xpos, ypos);
}

void _glfwInputCursorEnter(_GLFWwindow* window, GLFWbool entered)
{
	assert(window != NULL);
	assert(entered == GLFW_TRUE || entered == GLFW_FALSE);

	if (window->callbacks.cursorEnter)
		window->callbacks.cursorEnter((GLFWwindow*)window, entered);
}

void _glfwInputDrop(_GLFWwindow* window, int count, const char** paths)
{
	assert(window != NULL);
	assert(count > 0);
	assert(paths != NULL);

	if (window->callbacks.drop)
		window->callbacks.drop((GLFWwindow*)window, count, paths);
}

void _glfwCenterCursorInContentArea(_GLFWwindow* window)
{
	int width, height;

	_glfwGetWindowSizeWin32(window, &width, &height);
	_glfwSetCursorPosWin32(window, width / 2.0, height / 2.0);
}

int glfwGetInputMode(GLFWwindow* handle, int mode)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(0);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	switch (mode)
	{
	case GLFW_CURSOR:
		return window->cursorMode;
	case GLFW_STICKY_KEYS:
		return window->stickyKeys;
	case GLFW_STICKY_MOUSE_BUTTONS:
		return window->stickyMouseButtons;
	case GLFW_LOCK_KEY_MODS:
		return window->lockKeyMods;
	case GLFW_RAW_MOUSE_MOTION:
		return window->rawMouseMotion;
	case GLFW_UNLIMITED_MOUSE_BUTTONS:
		return window->disableMouseButtonLimit;
	}

	_glfwInputError(GLFW_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
	return 0;
}

void glfwSetInputMode(GLFWwindow* handle, int mode, int value)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	switch (mode)
	{
	case GLFW_CURSOR:
	{
		if (value != GLFW_CURSOR_NORMAL &&
			value != GLFW_CURSOR_HIDDEN &&
			value != GLFW_CURSOR_DISABLED &&
			value != GLFW_CURSOR_CAPTURED)
		{
			_glfwInputError(GLFW_INVALID_ENUM,
				"Invalid cursor mode 0x%08X",
				value);
			return;
		}

		if (window->cursorMode == value)
			return;

		window->cursorMode = value;

		_glfwGetCursorPosWin32(window,
			&window->virtualCursorPosX,
			&window->virtualCursorPosY);
		_glfwSetCursorModeWin32(window, value);
		return;
	}

	case GLFW_STICKY_KEYS:
	{
		value = value ? GLFW_TRUE : GLFW_FALSE;
		if (window->stickyKeys == value)
			return;

		if (!value)
		{
			int i;


			for (i = 0; i <= GLFW_KEY_LAST; i++)
			{
				if (window->keys[i] == _GLFW_STICK)
					window->keys[i] = GLFW_RELEASE;
			}
		}

		window->stickyKeys = value;
		return;
	}

	case GLFW_STICKY_MOUSE_BUTTONS:
	{
		value = value ? GLFW_TRUE : GLFW_FALSE;
		if (window->stickyMouseButtons == value)
			return;

		if (!value)
		{
			int i;


			for (i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++)
			{
				if (window->mouseButtons[i] == _GLFW_STICK)
					window->mouseButtons[i] = GLFW_RELEASE;
			}
		}

		window->stickyMouseButtons = value;
		return;
	}

	case GLFW_LOCK_KEY_MODS:
	{
		window->lockKeyMods = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	}

	case GLFW_RAW_MOUSE_MOTION:
	{
		value = value ? GLFW_TRUE : GLFW_FALSE;
		if (window->rawMouseMotion == value)
			return;

		window->rawMouseMotion = value;
		_glfwSetRawMouseMotionWin32(window, value);
		return;
	}

	case GLFW_UNLIMITED_MOUSE_BUTTONS:
	{
		window->disableMouseButtonLimit = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	}
	}

	_glfwInputError(GLFW_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
}

const char* glfwGetKeyName(int key, int scancode)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	if (key != GLFW_KEY_UNKNOWN)
	{
		if (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST)
		{
			_glfwInputError(GLFW_INVALID_ENUM, "Invalid key %i", key);
			return NULL;
		}

		if (key != GLFW_KEY_KP_EQUAL &&
			(key < GLFW_KEY_KP_0 || key > GLFW_KEY_KP_ADD) &&
			(key < GLFW_KEY_APOSTROPHE || key > GLFW_KEY_WORLD_2))
		{
			return NULL;
		}

		scancode = _glfwGetKeyScancodeWin32(key);
	}

	return _glfwGetScancodeNameWin32(scancode);
}

int glfwGetKeyScancode(int key)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(0);

	if (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST)
	{
		_glfwInputError(GLFW_INVALID_ENUM, "Invalid key %i", key);
		return -1;
	}

	return _glfwGetKeyScancodeWin32(key);
}

int glfwGetKey(GLFWwindow* handle, int key)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(GLFW_RELEASE);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST)
	{
		_glfwInputError(GLFW_INVALID_ENUM, "Invalid key %i", key);
		return GLFW_RELEASE;
	}

	if (window->keys[key] == _GLFW_STICK)
	{

		window->keys[key] = GLFW_RELEASE;
		return GLFW_PRESS;
	}

	return (int)window->keys[key];
}

int glfwGetMouseButton(GLFWwindow* handle, int button)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(GLFW_RELEASE);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST)
	{
		_glfwInputError(GLFW_INVALID_ENUM, "Invalid mouse button %i", button);
		return GLFW_RELEASE;
	}

	if (window->mouseButtons[button] == _GLFW_STICK)
	{

		window->mouseButtons[button] = GLFW_RELEASE;
		return GLFW_PRESS;
	}

	return (int)window->mouseButtons[button];
}

void glfwGetCursorPos(GLFWwindow* handle, double* xpos, double* ypos)
{
	if (xpos)
		*xpos = 0;
	if (ypos)
		*ypos = 0;

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (window->cursorMode == GLFW_CURSOR_DISABLED)
	{
		if (xpos)
			*xpos = window->virtualCursorPosX;
		if (ypos)
			*ypos = window->virtualCursorPosY;
	}
	else
		_glfwGetCursorPosWin32(window, xpos, ypos);
}

void glfwSetCursorPos(GLFWwindow* handle, double xpos, double ypos)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (xpos != xpos || xpos < -DBL_MAX || xpos > DBL_MAX ||
		ypos != ypos || ypos < -DBL_MAX || ypos > DBL_MAX)
	{
		_glfwInputError(GLFW_INVALID_VALUE,
			"Invalid cursor position %f %f",
			xpos, ypos);
		return;
	}

	if (!_glfwWindowFocusedWin32(window))
		return;

	if (window->cursorMode == GLFW_CURSOR_DISABLED)
	{

		window->virtualCursorPosX = xpos;
		window->virtualCursorPosY = ypos;
	}
	else
	{
		_glfwSetCursorPosWin32(window, xpos, ypos);
	}
}

GLFWcursor* glfwCreateCursor(const GLFWimage* image, int xhot, int yhot)
{
	_GLFWcursor* cursor;

	assert(image != NULL);
	assert(image->pixels != NULL);

	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	if (image->width <= 0 || image->height <= 0)
	{
		_glfwInputError(GLFW_INVALID_VALUE, "Invalid image dimensions for cursor");
		return NULL;
	}

	cursor = _glfw_calloc(1, sizeof(_GLFWcursor));
	cursor->next = _glfw.cursorListHead;
	_glfw.cursorListHead = cursor;

	if (!_glfwCreateCursorWin32(cursor, image, xhot, yhot))
	{
		glfwDestroyCursor((GLFWcursor*)cursor);
		return NULL;
	}

	return (GLFWcursor*)cursor;
}

GLFWcursor* glfwCreateStandardCursor(int shape)
{
	_GLFWcursor* cursor;

	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	if (shape != GLFW_ARROW_CURSOR &&
		shape != GLFW_IBEAM_CURSOR &&
		shape != GLFW_CROSSHAIR_CURSOR &&
		shape != GLFW_POINTING_HAND_CURSOR &&
		shape != GLFW_RESIZE_EW_CURSOR &&
		shape != GLFW_RESIZE_NS_CURSOR &&
		shape != GLFW_RESIZE_NWSE_CURSOR &&
		shape != GLFW_RESIZE_NESW_CURSOR &&
		shape != GLFW_RESIZE_ALL_CURSOR &&
		shape != GLFW_NOT_ALLOWED_CURSOR)
	{
		_glfwInputError(GLFW_INVALID_ENUM, "Invalid standard cursor 0x%08X", shape);
		return NULL;
	}

	cursor = _glfw_calloc(1, sizeof(_GLFWcursor));
	cursor->next = _glfw.cursorListHead;
	_glfw.cursorListHead = cursor;

	if (!_glfwCreateStandardCursorWin32(cursor, shape))
	{
		glfwDestroyCursor((GLFWcursor*)cursor);
		return NULL;
	}

	return (GLFWcursor*)cursor;
}

void glfwDestroyCursor(GLFWcursor* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWcursor* cursor = (_GLFWcursor*)handle;

	if (cursor == NULL)
		return;


	{
		_GLFWwindow* window;

		for (window = _glfw.windowListHead; window; window = window->next)
		{
			if (window->cursor == cursor)
				glfwSetCursor((GLFWwindow*)window, NULL);
		}
	}

	_glfwDestroyCursorWin32(cursor);


	{
		_GLFWcursor** prev = &_glfw.cursorListHead;

		while (*prev != cursor)
			prev = &((*prev)->next);

		*prev = cursor->next;
	}

	_glfw_free(cursor);
}

void glfwSetCursor(GLFWwindow* windowHandle, GLFWcursor* cursorHandle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)windowHandle;
	_GLFWcursor* cursor = (_GLFWcursor*)cursorHandle;
	assert(window != NULL);

	window->cursor = cursor;

	_glfwSetCursorWin32(window, cursor);
}

GLFWkeyfun glfwSetKeyCallback(GLFWwindow* handle, GLFWkeyfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWkeyfun, window->callbacks.key, cbfun);
	return cbfun;
}

GLFWcharfun glfwSetCharCallback(GLFWwindow* handle, GLFWcharfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWcharfun, window->callbacks.character, cbfun);
	return cbfun;
}

GLFWcharmodsfun glfwSetCharModsCallback(GLFWwindow* handle, GLFWcharmodsfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWcharmodsfun, window->callbacks.charmods, cbfun);
	return cbfun;
}

GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow* handle,
	GLFWmousebuttonfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWmousebuttonfun, window->callbacks.mouseButton, cbfun);
	return cbfun;
}

GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* handle,
	GLFWcursorposfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWcursorposfun, window->callbacks.cursorPos, cbfun);
	return cbfun;
}

GLFWcursorenterfun glfwSetCursorEnterCallback(GLFWwindow* handle,
	GLFWcursorenterfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWcursorenterfun, window->callbacks.cursorEnter, cbfun);
	return cbfun;
}

GLFWscrollfun glfwSetScrollCallback(GLFWwindow* handle,
	GLFWscrollfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWscrollfun, window->callbacks.scroll, cbfun);
	return cbfun;
}

GLFWdropfun glfwSetDropCallback(GLFWwindow* handle, GLFWdropfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWdropfun, window->callbacks.drop, cbfun);
	return cbfun;
}

void glfwSetClipboardString(GLFWwindow* handle, const char* string)
{
	assert(string != NULL);

	_GLFW_REQUIRE_INIT();
	_glfwSetClipboardStringWin32(string);
}

const char* glfwGetClipboardString(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);
	return _glfwGetClipboardStringWin32();
}

double glfwGetTime(void)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(0.0);
	return (double)(_glfwPlatformGetTimerValue() - _glfw.timer.offset) /
		_glfwPlatformGetTimerFrequency();
}

void glfwSetTime(double time)
{
	_GLFW_REQUIRE_INIT();

	if (time != time || time < 0.0 || time > 18446744073.0)
	{
		_glfwInputError(GLFW_INVALID_VALUE, "Invalid time %f", time);
		return;
	}

	_glfw.timer.offset = _glfwPlatformGetTimerValue() -
		(uint64_t)(time * _glfwPlatformGetTimerFrequency());
}

uint64_t glfwGetTimerValue(void)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(0);
	return _glfwPlatformGetTimerValue();
}

uint64_t glfwGetTimerFrequency(void)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(0);
	return _glfwPlatformGetTimerFrequency();
}

static int compareVideoModes(const void* fp, const void* sp)
{
	const GLFWvidmode* fm = fp;
	const GLFWvidmode* sm = sp;
	const int fbpp = fm->redBits + fm->greenBits + fm->blueBits;
	const int sbpp = sm->redBits + sm->greenBits + sm->blueBits;
	const int farea = fm->width * fm->height;
	const int sarea = sm->width * sm->height;


	if (fbpp != sbpp)
		return fbpp - sbpp;


	if (farea != sarea)
		return farea - sarea;


	if (fm->width != sm->width)
		return fm->width - sm->width;


	return fm->refreshRate - sm->refreshRate;
}

static GLFWbool refreshVideoModes(_GLFWmonitor* monitor)
{
	int modeCount;
	GLFWvidmode* modes;

	if (monitor->modes)
		return GLFW_TRUE;

	modes = _glfwGetVideoModesWin32(monitor, &modeCount);
	if (!modes)
		return GLFW_FALSE;

	qsort(modes, modeCount, sizeof(GLFWvidmode), compareVideoModes);

	_glfw_free(monitor->modes);
	monitor->modes = modes;
	monitor->modeCount = modeCount;

	return GLFW_TRUE;
}

void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement)
{
	assert(monitor != NULL);
	assert(action == GLFW_CONNECTED || action == GLFW_DISCONNECTED);
	assert(placement == _GLFW_INSERT_FIRST || placement == _GLFW_INSERT_LAST);

	if (action == GLFW_CONNECTED)
	{
		_glfw.monitorCount++;
		_glfw.monitors =
			_glfw_realloc(_glfw.monitors,
				sizeof(_GLFWmonitor*) * _glfw.monitorCount);

		if (placement == _GLFW_INSERT_FIRST)
		{
			memmove(_glfw.monitors + 1,
				_glfw.monitors,
				((size_t)_glfw.monitorCount - 1) * sizeof(_GLFWmonitor*));
			_glfw.monitors[0] = monitor;
		}
		else
			_glfw.monitors[_glfw.monitorCount - 1] = monitor;
	}
	else if (action == GLFW_DISCONNECTED)
	{
		int i;
		_GLFWwindow* window;

		for (window = _glfw.windowListHead; window; window = window->next)
		{
			if (window->monitor == monitor)
			{
				int width, height, xoff, yoff;
				_glfwGetWindowSizeWin32(window, &width, &height);
				_glfwSetWindowMonitorWin32(window, NULL, 0, 0, width, height, 0);
				_glfwGetWindowFrameSizeWin32(window, &xoff, &yoff, NULL, NULL);
				_glfwSetWindowPosWin32(window, xoff, yoff);
			}
		}

		for (i = 0; i < _glfw.monitorCount; i++)
		{
			if (_glfw.monitors[i] == monitor)
			{
				_glfw.monitorCount--;
				memmove(_glfw.monitors + i,
					_glfw.monitors + i + 1,
					((size_t)_glfw.monitorCount - i) * sizeof(_GLFWmonitor*));
				break;
			}
		}
	}

	if (_glfw.callbacks.monitor)
		_glfw.callbacks.monitor((GLFWmonitor*)monitor, action);

	if (action == GLFW_DISCONNECTED)
		_glfwFreeMonitor(monitor);
}

void _glfwInputMonitorWindow(_GLFWmonitor* monitor, _GLFWwindow* window)
{
	assert(monitor != NULL);
	monitor->window = window;
}

_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM)
{
	_GLFWmonitor* monitor = _glfw_calloc(1, sizeof(_GLFWmonitor));
	monitor->widthMM = widthMM;
	monitor->heightMM = heightMM;

	strncpy(monitor->name, name, sizeof(monitor->name) - 1);

	return monitor;
}

void _glfwFreeMonitor(_GLFWmonitor* monitor)
{
	if (monitor == NULL)
		return;

	_glfwFreeGammaArrays(&monitor->originalRamp);
	_glfwFreeGammaArrays(&monitor->currentRamp);

	_glfw_free(monitor->modes);
	_glfw_free(monitor);
}

void _glfwAllocGammaArrays(GLFWgammaramp* ramp, unsigned int size)
{
	ramp->red = _glfw_calloc(size, sizeof(unsigned short));
	ramp->green = _glfw_calloc(size, sizeof(unsigned short));
	ramp->blue = _glfw_calloc(size, sizeof(unsigned short));
	ramp->size = size;
}

void _glfwFreeGammaArrays(GLFWgammaramp* ramp)
{
	_glfw_free(ramp->red);
	_glfw_free(ramp->green);
	_glfw_free(ramp->blue);

	memset(ramp, 0, sizeof(GLFWgammaramp));
}

const GLFWvidmode* _glfwChooseVideoMode(_GLFWmonitor* monitor,
	const GLFWvidmode* desired)
{
	int i;
	unsigned int sizeDiff, leastSizeDiff = UINT_MAX;
	unsigned int rateDiff, leastRateDiff = UINT_MAX;
	unsigned int colorDiff, leastColorDiff = UINT_MAX;
	const GLFWvidmode* current;
	const GLFWvidmode* closest = NULL;

	if (!refreshVideoModes(monitor))
		return NULL;

	for (i = 0; i < monitor->modeCount; i++)
	{
		current = monitor->modes + i;

		colorDiff = 0;

		if (desired->redBits != GLFW_DONT_CARE)
			colorDiff += abs(current->redBits - desired->redBits);
		if (desired->greenBits != GLFW_DONT_CARE)
			colorDiff += abs(current->greenBits - desired->greenBits);
		if (desired->blueBits != GLFW_DONT_CARE)
			colorDiff += abs(current->blueBits - desired->blueBits);

		sizeDiff = abs((current->width - desired->width) *
			(current->width - desired->width) +
			(current->height - desired->height) *
			(current->height - desired->height));

		if (desired->refreshRate != GLFW_DONT_CARE)
			rateDiff = abs(current->refreshRate - desired->refreshRate);
		else
			rateDiff = UINT_MAX - current->refreshRate;

		if ((colorDiff < leastColorDiff) ||
			(colorDiff == leastColorDiff && sizeDiff < leastSizeDiff) ||
			(colorDiff == leastColorDiff && sizeDiff == leastSizeDiff && rateDiff < leastRateDiff))
		{
			closest = current;
			leastSizeDiff = sizeDiff;
			leastRateDiff = rateDiff;
			leastColorDiff = colorDiff;
		}
	}

	return closest;
}

int _glfwCompareVideoModes(const GLFWvidmode* fm, const GLFWvidmode* sm)
{
	return compareVideoModes(fm, sm);
}

void _glfwSplitBPP(int bpp, int* red, int* green, int* blue)
{
	int delta;

	if (bpp == 32)
		bpp = 24;

	*red = *green = *blue = bpp / 3;
	delta = bpp - (*red * 3);
	if (delta >= 1)
		*green = *green + 1;

	if (delta == 2)
		*red = *red + 1;
}

GLFWmonitor** glfwGetMonitors(int* count)
{
	assert(count != NULL);

	*count = 0;

	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	*count = _glfw.monitorCount;
	return (GLFWmonitor**)_glfw.monitors;
}

GLFWmonitor* glfwGetPrimaryMonitor(void)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	if (!_glfw.monitorCount)
		return NULL;

	return (GLFWmonitor*)_glfw.monitors[0];
}

void glfwGetMonitorPos(GLFWmonitor* handle, int* xpos, int* ypos)
{
	if (xpos)
		*xpos = 0;
	if (ypos)
		*ypos = 0;

	_GLFW_REQUIRE_INIT();

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	_glfwGetMonitorPosWin32(monitor, xpos, ypos);
}

void glfwGetMonitorWorkarea(GLFWmonitor* handle,
	int* xpos, int* ypos,
	int* width, int* height)
{
	if (xpos)
		*xpos = 0;
	if (ypos)
		*ypos = 0;
	if (width)
		*width = 0;
	if (height)
		*height = 0;

	_GLFW_REQUIRE_INIT();

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	_glfwGetMonitorWorkareaWin32(monitor, xpos, ypos, width, height);
}

void glfwGetMonitorPhysicalSize(GLFWmonitor* handle, int* widthMM, int* heightMM)
{
	if (widthMM)
		*widthMM = 0;
	if (heightMM)
		*heightMM = 0;

	_GLFW_REQUIRE_INIT();

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	if (widthMM)
		*widthMM = monitor->widthMM;
	if (heightMM)
		*heightMM = monitor->heightMM;
}

void glfwGetMonitorContentScale(GLFWmonitor* handle,
	float* xscale, float* yscale)
{
	if (xscale)
		*xscale = 0.f;
	if (yscale)
		*yscale = 0.f;

	_GLFW_REQUIRE_INIT();

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	_glfwGetMonitorContentScaleWin32(monitor, xscale, yscale);
}

const char* glfwGetMonitorName(GLFWmonitor* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	return monitor->name;
}

void glfwSetMonitorUserPointer(GLFWmonitor* handle, void* pointer)
{
	_GLFW_REQUIRE_INIT();

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	monitor->userPointer = pointer;
}

void* glfwGetMonitorUserPointer(GLFWmonitor* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	return monitor->userPointer;
}

GLFWmonitorfun glfwSetMonitorCallback(GLFWmonitorfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);
	_GLFW_SWAP(GLFWmonitorfun, _glfw.callbacks.monitor, cbfun);
	return cbfun;
}

const GLFWvidmode* glfwGetVideoModes(GLFWmonitor* handle, int* count)
{
	assert(count != NULL);

	*count = 0;

	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	if (!refreshVideoModes(monitor))
		return NULL;

	*count = monitor->modeCount;
	return monitor->modes;
}

const GLFWvidmode* glfwGetVideoMode(GLFWmonitor* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	if (!_glfwGetVideoModeWin32(monitor, &monitor->currentMode))
		return NULL;

	return &monitor->currentMode;
}

void glfwSetGamma(GLFWmonitor* handle, float gamma)
{
	unsigned int i;
	unsigned short* values;
	GLFWgammaramp ramp;
	const GLFWgammaramp* original;
	assert(gamma > 0.f);
	assert(gamma <= FLT_MAX);

	_GLFW_REQUIRE_INIT();

	assert(handle != NULL);

	if (gamma != gamma || gamma <= 0.f || gamma > FLT_MAX)
	{
		_glfwInputError(GLFW_INVALID_VALUE, "Invalid gamma value %f", gamma);
		return;
	}

	original = glfwGetGammaRamp(handle);
	if (!original)
		return;

	values = _glfw_calloc(original->size, sizeof(unsigned short));

	for (i = 0; i < original->size; i++)
	{
		float value;


		value = i / (float)(original->size - 1);

		value = powf(value, 1.f / gamma) * 65535.f + 0.5f;

		value = fminf(value, 65535.f);

		values[i] = (unsigned short)value;
	}

	ramp.red = values;
	ramp.green = values;
	ramp.blue = values;
	ramp.size = original->size;

	glfwSetGammaRamp(handle, &ramp);
	_glfw_free(values);
}

const GLFWgammaramp* glfwGetGammaRamp(GLFWmonitor* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	_glfwFreeGammaArrays(&monitor->currentRamp);
	if (!_glfwGetGammaRampWin32(monitor, &monitor->currentRamp))
		return NULL;

	return &monitor->currentRamp;
}

void glfwSetGammaRamp(GLFWmonitor* handle, const GLFWgammaramp* ramp)
{
	assert(ramp != NULL);
	assert(ramp->size > 0);
	assert(ramp->red != NULL);
	assert(ramp->green != NULL);
	assert(ramp->blue != NULL);

	_GLFW_REQUIRE_INIT();

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	if (ramp->size <= 0)
	{
		_glfwInputError(GLFW_INVALID_VALUE,
			"Invalid gamma ramp size %i",
			ramp->size);
		return;
	}

	if (!monitor->originalRamp.size)
	{
		if (!_glfwGetGammaRampWin32(monitor, &monitor->originalRamp))
			return;
	}

	_glfwSetGammaRampWin32(monitor, ramp);
}

void _glfwInputWindowFocus(_GLFWwindow* window, GLFWbool focused)
{
	assert(window != NULL);
	assert(focused == GLFW_TRUE || focused == GLFW_FALSE);

	if (window->callbacks.focus)
		window->callbacks.focus((GLFWwindow*)window, focused);

	if (!focused)
	{
		int key, button;

		for (key = 0; key <= GLFW_KEY_LAST; key++)
		{
			if (window->keys[key] == GLFW_PRESS)
			{
				const int scancode = _glfwGetKeyScancodeWin32(key);
				_glfwInputKey(window, key, scancode, GLFW_RELEASE, 0);
			}
		}

		for (button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++)
		{
			if (window->mouseButtons[button] == GLFW_PRESS)
				_glfwInputMouseClick(window, button, GLFW_RELEASE, 0);
		}
	}
}

void _glfwInputWindowPos(_GLFWwindow* window, int x, int y)
{
	assert(window != NULL);

	if (window->callbacks.pos)
		window->callbacks.pos((GLFWwindow*)window, x, y);
}

void _glfwInputWindowSize(_GLFWwindow* window, int width, int height)
{
	assert(window != NULL);
	assert(width >= 0);
	assert(height >= 0);

	if (window->callbacks.size)
		window->callbacks.size((GLFWwindow*)window, width, height);
}

void _glfwInputWindowIconify(_GLFWwindow* window, GLFWbool iconified)
{
	assert(window != NULL);
	assert(iconified == GLFW_TRUE || iconified == GLFW_FALSE);

	if (window->callbacks.iconify)
		window->callbacks.iconify((GLFWwindow*)window, iconified);
}

void _glfwInputWindowMaximize(_GLFWwindow* window, GLFWbool maximized)
{
	assert(window != NULL);
	assert(maximized == GLFW_TRUE || maximized == GLFW_FALSE);

	if (window->callbacks.maximize)
		window->callbacks.maximize((GLFWwindow*)window, maximized);
}

void _glfwInputFramebufferSize(_GLFWwindow* window, int width, int height)
{
	assert(window != NULL);
	assert(width >= 0);
	assert(height >= 0);

	if (window->callbacks.fbsize)
		window->callbacks.fbsize((GLFWwindow*)window, width, height);
}

void _glfwInputWindowContentScale(_GLFWwindow* window, float xscale, float yscale)
{
	assert(window != NULL);
	assert(xscale > 0.f);
	assert(xscale < FLT_MAX);
	assert(yscale > 0.f);
	assert(yscale < FLT_MAX);

	if (window->callbacks.scale)
		window->callbacks.scale((GLFWwindow*)window, xscale, yscale);
}

void _glfwInputWindowDamage(_GLFWwindow* window)
{
	assert(window != NULL);

	if (window->callbacks.refresh)
		window->callbacks.refresh((GLFWwindow*)window);
}

void _glfwInputWindowCloseRequest(_GLFWwindow* window)
{
	assert(window != NULL);

	window->shouldClose = GLFW_TRUE;

	if (window->callbacks.close)
		window->callbacks.close((GLFWwindow*)window);
}

void _glfwInputWindowMonitor(_GLFWwindow* window, _GLFWmonitor* monitor)
{
	assert(window != NULL);
	window->monitor = monitor;
}

GLFWwindow* glfwCreateWindow(int width, int height,
	const char* title,
	GLFWmonitor* monitor,
	GLFWwindow* share)
{
	_GLFWfbconfig fbconfig;
	_GLFWctxconfig ctxconfig;
	_GLFWwndconfig wndconfig;
	_GLFWwindow* window;

	assert(title != NULL);
	assert(width >= 0);
	assert(height >= 0);

	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	if (width <= 0 || height <= 0)
	{
		_glfwInputError(GLFW_INVALID_VALUE, "Invalid window size %ix%i", width, height);

		return NULL;
	}

	fbconfig = _glfw.hints.framebuffer;
	ctxconfig = _glfw.hints.context;
	wndconfig = _glfw.hints.window;

	wndconfig.width = width;
	wndconfig.height = height;
	ctxconfig.share = (_GLFWwindow*)share;

	if (!_glfwIsValidContextConfig(&ctxconfig))
		return NULL;

	window = _glfw_calloc(1, sizeof(_GLFWwindow));
	window->next = _glfw.windowListHead;
	_glfw.windowListHead = window;

	window->videoMode.width = width;
	window->videoMode.height = height;
	window->videoMode.redBits = fbconfig.redBits;
	window->videoMode.greenBits = fbconfig.greenBits;
	window->videoMode.blueBits = fbconfig.blueBits;
	window->videoMode.refreshRate = _glfw.hints.refreshRate;

	window->monitor = (_GLFWmonitor*)monitor;
	window->resizable = wndconfig.resizable;
	window->decorated = wndconfig.decorated;
	window->autoIconify = wndconfig.autoIconify;
	window->floating = wndconfig.floating;
	window->focusOnShow = wndconfig.focusOnShow;
	window->mousePassthrough = wndconfig.mousePassthrough;
	window->cursorMode = GLFW_CURSOR_NORMAL;

	window->doublebuffer = fbconfig.doublebuffer;

	window->minwidth = GLFW_DONT_CARE;
	window->minheight = GLFW_DONT_CARE;
	window->maxwidth = GLFW_DONT_CARE;
	window->maxheight = GLFW_DONT_CARE;
	window->numer = GLFW_DONT_CARE;
	window->denom = GLFW_DONT_CARE;
	window->title = _glfw_strdup(title);

	if (!_glfwCreateWindowWin32(window, &wndconfig, &ctxconfig, &fbconfig))
	{
		glfwDestroyWindow((GLFWwindow*)window);
		return NULL;
	}

	return (GLFWwindow*)window;
}

void glfwDefaultWindowHints(void)
{
	_GLFW_REQUIRE_INIT();

	memset(&_glfw.hints.context, 0, sizeof(_glfw.hints.context));
	_glfw.hints.context.major = 1;
	_glfw.hints.context.minor = 0;

	memset(&_glfw.hints.window, 0, sizeof(_glfw.hints.window));
	_glfw.hints.window.resizable = GLFW_TRUE;
	_glfw.hints.window.visible = GLFW_TRUE;
	_glfw.hints.window.decorated = GLFW_TRUE;
	_glfw.hints.window.focused = GLFW_TRUE;
	_glfw.hints.window.autoIconify = GLFW_TRUE;
	_glfw.hints.window.centerCursor = GLFW_TRUE;
	_glfw.hints.window.focusOnShow = GLFW_TRUE;
	_glfw.hints.window.xpos = GLFW_ANY_POSITION;
	_glfw.hints.window.ypos = GLFW_ANY_POSITION;
	_glfw.hints.window.scaleFramebuffer = GLFW_TRUE;

	memset(&_glfw.hints.framebuffer, 0, sizeof(_glfw.hints.framebuffer));
	_glfw.hints.framebuffer.redBits = 8;
	_glfw.hints.framebuffer.greenBits = 8;
	_glfw.hints.framebuffer.blueBits = 8;
	_glfw.hints.framebuffer.alphaBits = 8;
	_glfw.hints.framebuffer.depthBits = 24;
	_glfw.hints.framebuffer.stencilBits = 8;
	_glfw.hints.framebuffer.doublebuffer = GLFW_TRUE;

	_glfw.hints.refreshRate = GLFW_DONT_CARE;
}

void glfwWindowHint(int hint, int value)
{
	_GLFW_REQUIRE_INIT();

	switch (hint)
	{
	case GLFW_RED_BITS:
		_glfw.hints.framebuffer.redBits = value;
		return;
	case GLFW_GREEN_BITS:
		_glfw.hints.framebuffer.greenBits = value;
		return;
	case GLFW_BLUE_BITS:
		_glfw.hints.framebuffer.blueBits = value;
		return;
	case GLFW_ALPHA_BITS:
		_glfw.hints.framebuffer.alphaBits = value;
		return;
	case GLFW_DEPTH_BITS:
		_glfw.hints.framebuffer.depthBits = value;
		return;
	case GLFW_STENCIL_BITS:
		_glfw.hints.framebuffer.stencilBits = value;
		return;
	case GLFW_ACCUM_RED_BITS:
		_glfw.hints.framebuffer.accumRedBits = value;
		return;
	case GLFW_ACCUM_GREEN_BITS:
		_glfw.hints.framebuffer.accumGreenBits = value;
		return;
	case GLFW_ACCUM_BLUE_BITS:
		_glfw.hints.framebuffer.accumBlueBits = value;
		return;
	case GLFW_ACCUM_ALPHA_BITS:
		_glfw.hints.framebuffer.accumAlphaBits = value;
		return;
	case GLFW_AUX_BUFFERS:
		_glfw.hints.framebuffer.auxBuffers = value;
		return;
	case GLFW_STEREO:
		_glfw.hints.framebuffer.stereo = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_DOUBLEBUFFER:
		_glfw.hints.framebuffer.doublebuffer = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_TRANSPARENT_FRAMEBUFFER:
		_glfw.hints.framebuffer.transparent = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_SAMPLES:
		_glfw.hints.framebuffer.samples = value;
		return;
	case GLFW_SRGB_CAPABLE:
		_glfw.hints.framebuffer.sRGB = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_RESIZABLE:
		_glfw.hints.window.resizable = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_DECORATED:
		_glfw.hints.window.decorated = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_FOCUSED:
		_glfw.hints.window.focused = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_AUTO_ICONIFY:
		_glfw.hints.window.autoIconify = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_FLOATING:
		_glfw.hints.window.floating = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_MAXIMIZED:
		_glfw.hints.window.maximized = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_VISIBLE:
		_glfw.hints.window.visible = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_POSITION_X:
		_glfw.hints.window.xpos = value;
		return;
	case GLFW_POSITION_Y:
		_glfw.hints.window.ypos = value;
		return;
	case GLFW_WIN32_KEYBOARD_MENU:
		_glfw.hints.window.win32.keymenu = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_WIN32_SHOWDEFAULT:
		_glfw.hints.window.win32.showDefault = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_SCALE_TO_MONITOR:
		_glfw.hints.window.scaleToMonitor = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_SCALE_FRAMEBUFFER:
	case GLFW_CENTER_CURSOR:
		_glfw.hints.window.centerCursor = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_FOCUS_ON_SHOW:
		_glfw.hints.window.focusOnShow = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_MOUSE_PASSTHROUGH:
		_glfw.hints.window.mousePassthrough = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_CONTEXT_VERSION_MAJOR:
		_glfw.hints.context.major = value;
		return;
	case GLFW_CONTEXT_VERSION_MINOR:
		_glfw.hints.context.minor = value;
		return;
	case GLFW_CONTEXT_ROBUSTNESS:
		_glfw.hints.context.robustness = value;
		return;
	case GLFW_OPENGL_FORWARD_COMPAT:
		_glfw.hints.context.forward = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_CONTEXT_DEBUG:
		_glfw.hints.context.debug = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_CONTEXT_NO_ERROR:
		_glfw.hints.context.noerror = value ? GLFW_TRUE : GLFW_FALSE;
		return;
	case GLFW_OPENGL_PROFILE:
		_glfw.hints.context.profile = value;
		return;
	case GLFW_CONTEXT_RELEASE_BEHAVIOR:
		_glfw.hints.context.release = value;
		return;
	case GLFW_REFRESH_RATE:
		_glfw.hints.refreshRate = value;
		return;
	}

	_glfwInputError(GLFW_INVALID_ENUM, "Invalid window hint 0x%08X", hint);
}

void glfwDestroyWindow(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;

	if (window == NULL)
		return;

	memset(&window->callbacks, 0, sizeof(window->callbacks));

	if (window == _glfwPlatformGetTls(&_glfw.contextSlot))
		glfwMakeContextCurrent(NULL);

	_glfwDestroyWindowWin32(window);

	{
		_GLFWwindow** prev = &_glfw.windowListHead;

		while (*prev != window)
			prev = &((*prev)->next);

		*prev = window->next;
	}

	_glfw_free(window->title);
	_glfw_free(window);
}

int glfwWindowShouldClose(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(0);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	return window->shouldClose;
}

void glfwSetWindowShouldClose(GLFWwindow* handle, int value)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	window->shouldClose = value;
}

const char* glfwGetWindowTitle(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	return window->title;
}

void glfwSetWindowTitle(GLFWwindow* handle, const char* title)
{
	assert(title != NULL);

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	char* prev = window->title;
	window->title = _glfw_strdup(title);

	_glfwSetWindowTitleWin32(window, title);
	_glfw_free(prev);
}

void glfwSetWindowIcon(GLFWwindow* handle,
	int count, const GLFWimage* images)
{
	int i;

	assert(count >= 0);
	assert(count == 0 || images != NULL);

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (count < 0)
	{
		_glfwInputError(GLFW_INVALID_VALUE, "Invalid image count for window icon");
		return;
	}

	for (i = 0; i < count; i++)
	{
		assert(images[i].pixels != NULL);

		if (images[i].width <= 0 || images[i].height <= 0)
		{
			_glfwInputError(GLFW_INVALID_VALUE,
				"Invalid image dimensions for window icon");
			return;
		}
	}

	_glfwSetWindowIconWin32(window, count, images);
}

void glfwGetWindowPos(GLFWwindow* handle, int* xpos, int* ypos)
{
	if (xpos)
		*xpos = 0;
	if (ypos)
		*ypos = 0;

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwGetWindowPosWin32(window, xpos, ypos);
}

void glfwSetWindowPos(GLFWwindow* handle, int xpos, int ypos)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (window->monitor)
		return;

	_glfwSetWindowPosWin32(window, xpos, ypos);
}

void glfwGetWindowSize(GLFWwindow* handle, int* width, int* height)
{
	if (width)
		*width = 0;
	if (height)
		*height = 0;

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwGetWindowSizeWin32(window, width, height);
}

void glfwSetWindowSize(GLFWwindow* handle, int width, int height)
{
	assert(width >= 0);
	assert(height >= 0);

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	window->videoMode.width = width;
	window->videoMode.height = height;

	_glfwSetWindowSizeWin32(window, width, height);
}

void glfwSetWindowSizeLimits(GLFWwindow* handle,
	int minwidth, int minheight,
	int maxwidth, int maxheight)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (minwidth != GLFW_DONT_CARE && minheight != GLFW_DONT_CARE)
	{
		if (minwidth < 0 || minheight < 0)
		{
			_glfwInputError(GLFW_INVALID_VALUE,
				"Invalid window minimum size %ix%i",
				minwidth, minheight);
			return;
		}
	}

	if (maxwidth != GLFW_DONT_CARE && maxheight != GLFW_DONT_CARE)
	{
		if (maxwidth < 0 || maxheight < 0 ||
			maxwidth < minwidth || maxheight < minheight)
		{
			_glfwInputError(GLFW_INVALID_VALUE,
				"Invalid window maximum size %ix%i",
				maxwidth, maxheight);
			return;
		}
	}

	window->minwidth = minwidth;
	window->minheight = minheight;
	window->maxwidth = maxwidth;
	window->maxheight = maxheight;

	if (window->monitor || !window->resizable)
		return;

	_glfwSetWindowSizeLimitsWin32(window,
		minwidth, minheight,
		maxwidth, maxheight);
}

void glfwSetWindowAspectRatio(GLFWwindow* handle, int numer, int denom)
{
	assert(numer != 0);
	assert(denom != 0);

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (numer != GLFW_DONT_CARE && denom != GLFW_DONT_CARE)
	{
		if (numer <= 0 || denom <= 0)
		{
			_glfwInputError(GLFW_INVALID_VALUE,
				"Invalid window aspect ratio %i:%i",
				numer, denom);
			return;
		}
	}

	window->numer = numer;
	window->denom = denom;

	if (window->monitor || !window->resizable)
		return;

	_glfwSetWindowAspectRatioWin32(window, numer, denom);
}

void glfwGetFramebufferSize(GLFWwindow* handle, int* width, int* height)
{
	if (width)
		*width = 0;
	if (height)
		*height = 0;

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwGetWindowSizeWin32(window, width, height);
}

void glfwGetWindowFrameSize(GLFWwindow* handle,
	int* left, int* top,
	int* right, int* bottom)
{
	if (left)
		*left = 0;
	if (top)
		*top = 0;
	if (right)
		*right = 0;
	if (bottom)
		*bottom = 0;

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwGetWindowFrameSizeWin32(window, left, top, right, bottom);
}

void glfwGetWindowContentScale(GLFWwindow* handle,
	float* xscale, float* yscale)
{
	if (xscale)
		*xscale = 0.f;
	if (yscale)
		*yscale = 0.f;

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwGetWindowContentScaleWin32(window, xscale, yscale);
}

float glfwGetWindowOpacity(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(0.f);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	return _glfwGetWindowOpacityWin32(window);
}

void glfwSetWindowOpacity(GLFWwindow* handle, float opacity)
{
	assert(opacity == opacity);
	assert(opacity >= 0.f);
	assert(opacity <= 1.f);

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (opacity != opacity || opacity < 0.f || opacity > 1.f)
	{
		_glfwInputError(GLFW_INVALID_VALUE, "Invalid window opacity %f", opacity);
		return;
	}

	_glfwSetWindowOpacityWin32(window, opacity);
}

void glfwIconifyWindow(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwIconifyWindowWin32(window);
}

void glfwRestoreWindow(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwRestoreWindowWin32(window);
}

void glfwMaximizeWindow(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (window->monitor)
		return;

	_glfwMaximizeWindowWin32(window);
}

void glfwShowWindow(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (window->monitor)
		return;

	_glfwShowWindowWin32(window);

	if (window->focusOnShow)
		_glfwFocusWindowWin32(window);
}

void glfwRequestWindowAttention(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwRequestWindowAttentionWin32(window);
}

void glfwHideWindow(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	if (window->monitor)
		return;

	_glfwHideWindowWin32(window);
}

void glfwFocusWindow(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_glfwFocusWindowWin32(window);
}

int glfwGetWindowAttrib(GLFWwindow* handle, int attrib)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(0);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	switch (attrib)
	{
	case GLFW_FOCUSED:
		return _glfwWindowFocusedWin32(window);
	case GLFW_ICONIFIED:
		return _glfwWindowIconifiedWin32(window);
	case GLFW_VISIBLE:
		return _glfwWindowVisibleWin32(window);
	case GLFW_MAXIMIZED:
		return _glfwWindowMaximizedWin32(window);
	case GLFW_HOVERED:
		return _glfwWindowHoveredWin32(window);
	case GLFW_FOCUS_ON_SHOW:
		return window->focusOnShow;
	case GLFW_MOUSE_PASSTHROUGH:
		return window->mousePassthrough;
	case GLFW_TRANSPARENT_FRAMEBUFFER:
		return _glfwFramebufferTransparentWin32(window);
	case GLFW_RESIZABLE:
		return window->resizable;
	case GLFW_DECORATED:
		return window->decorated;
	case GLFW_FLOATING:
		return window->floating;
	case GLFW_AUTO_ICONIFY:
		return window->autoIconify;
	case GLFW_DOUBLEBUFFER:
		return window->doublebuffer;
	case GLFW_CONTEXT_VERSION_MAJOR:
		return window->context.major;
	case GLFW_CONTEXT_VERSION_MINOR:
		return window->context.minor;
	case GLFW_CONTEXT_REVISION:
		return window->context.revision;
	case GLFW_CONTEXT_ROBUSTNESS:
		return window->context.robustness;
	case GLFW_OPENGL_FORWARD_COMPAT:
		return window->context.forward;
	case GLFW_CONTEXT_DEBUG:
		return window->context.debug;
	case GLFW_OPENGL_PROFILE:
		return window->context.profile;
	case GLFW_CONTEXT_RELEASE_BEHAVIOR:
		return window->context.release;
	case GLFW_CONTEXT_NO_ERROR:
		return window->context.noerror;
	}

	_glfwInputError(GLFW_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
	return 0;
}

void glfwSetWindowAttrib(GLFWwindow* handle, int attrib, int value)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	value = value ? GLFW_TRUE : GLFW_FALSE;

	switch (attrib)
	{
	case GLFW_AUTO_ICONIFY:
		window->autoIconify = value;
		return;

	case GLFW_RESIZABLE:
		window->resizable = value;
		if (!window->monitor)
			_glfwSetWindowResizableWin32(window, value);
		return;

	case GLFW_DECORATED:
		window->decorated = value;
		if (!window->monitor)
			_glfwSetWindowDecoratedWin32(window, value);
		return;

	case GLFW_FLOATING:
		window->floating = value;
		if (!window->monitor)
			_glfwSetWindowFloatingWin32(window, value);
		return;

	case GLFW_FOCUS_ON_SHOW:
		window->focusOnShow = value;
		return;

	case GLFW_MOUSE_PASSTHROUGH:
		window->mousePassthrough = value;
		_glfwSetWindowMousePassthroughWin32(window, value);
		return;
	}

	_glfwInputError(GLFW_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
}

GLFWmonitor* glfwGetWindowMonitor(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	return (GLFWmonitor*)window->monitor;
}

void glfwSetWindowMonitor(GLFWwindow* wh,
	GLFWmonitor* mh,
	int xpos, int ypos,
	int width, int height,
	int refreshRate)
{
	assert(width >= 0);
	assert(height >= 0);

	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)wh;
	_GLFWmonitor* monitor = (_GLFWmonitor*)mh;
	assert(window != NULL);

	if (width <= 0 || height <= 0)
	{
		_glfwInputError(GLFW_INVALID_VALUE,
			"Invalid window size %ix%i",
			width, height);
		return;
	}

	if (refreshRate < 0 && refreshRate != GLFW_DONT_CARE)
	{
		_glfwInputError(GLFW_INVALID_VALUE,
			"Invalid refresh rate %i",
			refreshRate);
		return;
	}

	window->videoMode.width = width;
	window->videoMode.height = height;
	window->videoMode.refreshRate = refreshRate;

	_glfwSetWindowMonitorWin32(window, monitor,
		xpos, ypos, width, height,
		refreshRate);
}

void glfwSetWindowUserPointer(GLFWwindow* handle, void* pointer)
{
	_GLFW_REQUIRE_INIT();

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	window->userPointer = pointer;
}

void* glfwGetWindowUserPointer(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	return window->userPointer;
}

GLFWwindowposfun glfwSetWindowPosCallback(GLFWwindow* handle,
	GLFWwindowposfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWwindowposfun, window->callbacks.pos, cbfun);
	return cbfun;
}

GLFWwindowsizefun glfwSetWindowSizeCallback(GLFWwindow* handle,
	GLFWwindowsizefun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWwindowsizefun, window->callbacks.size, cbfun);
	return cbfun;
}

GLFWwindowclosefun glfwSetWindowCloseCallback(GLFWwindow* handle,
	GLFWwindowclosefun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWwindowclosefun, window->callbacks.close, cbfun);
	return cbfun;
}

GLFWwindowrefreshfun glfwSetWindowRefreshCallback(GLFWwindow* handle,
	GLFWwindowrefreshfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWwindowrefreshfun, window->callbacks.refresh, cbfun);
	return cbfun;
}

GLFWwindowfocusfun glfwSetWindowFocusCallback(GLFWwindow* handle,
	GLFWwindowfocusfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWwindowfocusfun, window->callbacks.focus, cbfun);
	return cbfun;
}

GLFWwindowiconifyfun glfwSetWindowIconifyCallback(GLFWwindow* handle,
	GLFWwindowiconifyfun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWwindowiconifyfun, window->callbacks.iconify, cbfun);
	return cbfun;
}

GLFWwindowmaximizefun glfwSetWindowMaximizeCallback(GLFWwindow* handle,
	GLFWwindowmaximizefun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWwindowmaximizefun, window->callbacks.maximize, cbfun);
	return cbfun;
}

GLFWframebuffersizefun glfwSetFramebufferSizeCallback(GLFWwindow* handle,
	GLFWframebuffersizefun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWframebuffersizefun, window->callbacks.fbsize, cbfun);
	return cbfun;
}

GLFWwindowcontentscalefun glfwSetWindowContentScaleCallback(GLFWwindow* handle,
	GLFWwindowcontentscalefun cbfun)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);

	_GLFW_SWAP(GLFWwindowcontentscalefun, window->callbacks.scale, cbfun);
	return cbfun;
}

void glfwPollEvents(void)
{
	_GLFW_REQUIRE_INIT();

	MSG msg;
	HWND handle;
	_GLFWwindow* window;

	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			window = _glfw.windowListHead;
			while (window)
			{
				_glfwInputWindowCloseRequest(window);
				window = window->next;
			}
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	handle = GetActiveWindow();
	if (handle)
	{
		window = GetPropW(handle, L"GLFW");
		if (window)
		{
			int i;
			const int keys[4][2] =
			{
			{ VK_LSHIFT, GLFW_KEY_LEFT_SHIFT },
			{ VK_RSHIFT, GLFW_KEY_RIGHT_SHIFT },
			{ VK_LWIN, GLFW_KEY_LEFT_SUPER },
			{ VK_RWIN, GLFW_KEY_RIGHT_SUPER }
			};

			for (i = 0; i < 4; i++)
			{
				const int vk = keys[i][0];
				const int key = keys[i][1];
				const int scancode = _glfw.win32.scancodes[key];

				if ((GetKeyState(vk) & 0x8000))
					continue;
				if (window->keys[key] != GLFW_PRESS)
					continue;

				_glfwInputKey(window, key, scancode, GLFW_RELEASE, getKeyMods());
			}
		}
	}

	window = _glfw.win32.disabledCursorWindow;
	if (window)
	{
		int width, height;
		_glfwGetWindowSizeWin32(window, &width, &height);

		if (window->win32.lastCursorPosX != width / 2 ||
			window->win32.lastCursorPosY != height / 2)
		{
			_glfwSetCursorPosWin32(window, width / 2, height / 2);
		}
	}
}

static const GUID _glfw_GUID_DEVINTERFACE_HID =
{ 0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30} };

#define GUID_DEVINTERFACE_HID _glfw_GUID_DEVINTERFACE_HID

static GLFWbool loadLibraries(void)
{
	if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(const WCHAR*)&_glfw,
		(HMODULE*)&_glfw.win32.instance))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR, "Win32: Failed to retrieve own module handle");
		return GLFW_FALSE;
	}

	_glfw.win32.user32.instance = _glfwPlatformLoadModule("user32.dll");
	if (!_glfw.win32.user32.instance)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR, "Win32: Failed to load user32.dll");
		return GLFW_FALSE;
	}

	_glfw.win32.user32.GetDpiForWindow_ = (PFN_GetDpiForWindow)
		_glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "GetDpiForWindow");
	_glfw.win32.user32.AdjustWindowRectExForDpi_ = (PFN_AdjustWindowRectExForDpi)
		_glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "AdjustWindowRectExForDpi");
	_glfw.win32.user32.GetSystemMetricsForDpi_ = (PFN_GetSystemMetricsForDpi)
		_glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "GetSystemMetricsForDpi");

	_glfw.win32.dwmapi.instance = _glfwPlatformLoadModule("dwmapi.dll");
	if (_glfw.win32.dwmapi.instance)
	{
		_glfw.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)
			_glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmIsCompositionEnabled");
		_glfw.win32.dwmapi.Flush = (PFN_DwmFlush)
			_glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmFlush");
		_glfw.win32.dwmapi.EnableBlurBehindWindow = (PFN_DwmEnableBlurBehindWindow)
			_glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmEnableBlurBehindWindow");
		_glfw.win32.dwmapi.GetColorizationColor = (PFN_DwmGetColorizationColor)
			_glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmGetColorizationColor");
	}

	_glfw.win32.shcore.instance = _glfwPlatformLoadModule("shcore.dll");
	if (_glfw.win32.shcore.instance)
	{
		_glfw.win32.shcore.SetProcessDpiAwareness_ = (PFN_SetProcessDpiAwareness)
			_glfwPlatformGetModuleSymbol(_glfw.win32.shcore.instance, "SetProcessDpiAwareness");
		_glfw.win32.shcore.GetDpiForMonitor_ = (PFN_GetDpiForMonitor)
			_glfwPlatformGetModuleSymbol(_glfw.win32.shcore.instance, "GetDpiForMonitor");
	}

	_glfw.win32.ntdll.instance = _glfwPlatformLoadModule("ntdll.dll");
	if (_glfw.win32.ntdll.instance)
	{
		_glfw.win32.ntdll.RtlVerifyVersionInfo_ = (PFN_RtlVerifyVersionInfo)
			_glfwPlatformGetModuleSymbol(_glfw.win32.ntdll.instance, "RtlVerifyVersionInfo");
	}

	return GLFW_TRUE;
}

static void freeLibraries(void)
{
	if (_glfw.win32.user32.instance)
		_glfwPlatformFreeModule(_glfw.win32.user32.instance);

	if (_glfw.win32.dwmapi.instance)
		_glfwPlatformFreeModule(_glfw.win32.dwmapi.instance);

	if (_glfw.win32.shcore.instance)
		_glfwPlatformFreeModule(_glfw.win32.shcore.instance);

	if (_glfw.win32.ntdll.instance)
		_glfwPlatformFreeModule(_glfw.win32.ntdll.instance);
}

static void createKeyTables(void)
{
	int scancode;

	memset(_glfw.win32.keycodes, -1, sizeof(_glfw.win32.keycodes));
	memset(_glfw.win32.scancodes, -1, sizeof(_glfw.win32.scancodes));

	_glfw.win32.keycodes[0x00B] = GLFW_KEY_0;
	_glfw.win32.keycodes[0x002] = GLFW_KEY_1;
	_glfw.win32.keycodes[0x003] = GLFW_KEY_2;
	_glfw.win32.keycodes[0x004] = GLFW_KEY_3;
	_glfw.win32.keycodes[0x005] = GLFW_KEY_4;
	_glfw.win32.keycodes[0x006] = GLFW_KEY_5;
	_glfw.win32.keycodes[0x007] = GLFW_KEY_6;
	_glfw.win32.keycodes[0x008] = GLFW_KEY_7;
	_glfw.win32.keycodes[0x009] = GLFW_KEY_8;
	_glfw.win32.keycodes[0x00A] = GLFW_KEY_9;
	_glfw.win32.keycodes[0x01E] = GLFW_KEY_A;
	_glfw.win32.keycodes[0x030] = GLFW_KEY_B;
	_glfw.win32.keycodes[0x02E] = GLFW_KEY_C;
	_glfw.win32.keycodes[0x020] = GLFW_KEY_D;
	_glfw.win32.keycodes[0x012] = GLFW_KEY_E;
	_glfw.win32.keycodes[0x021] = GLFW_KEY_F;
	_glfw.win32.keycodes[0x022] = GLFW_KEY_G;
	_glfw.win32.keycodes[0x023] = GLFW_KEY_H;
	_glfw.win32.keycodes[0x017] = GLFW_KEY_I;
	_glfw.win32.keycodes[0x024] = GLFW_KEY_J;
	_glfw.win32.keycodes[0x025] = GLFW_KEY_K;
	_glfw.win32.keycodes[0x026] = GLFW_KEY_L;
	_glfw.win32.keycodes[0x032] = GLFW_KEY_M;
	_glfw.win32.keycodes[0x031] = GLFW_KEY_N;
	_glfw.win32.keycodes[0x018] = GLFW_KEY_O;
	_glfw.win32.keycodes[0x019] = GLFW_KEY_P;
	_glfw.win32.keycodes[0x010] = GLFW_KEY_Q;
	_glfw.win32.keycodes[0x013] = GLFW_KEY_R;
	_glfw.win32.keycodes[0x01F] = GLFW_KEY_S;
	_glfw.win32.keycodes[0x014] = GLFW_KEY_T;
	_glfw.win32.keycodes[0x016] = GLFW_KEY_U;
	_glfw.win32.keycodes[0x02F] = GLFW_KEY_V;
	_glfw.win32.keycodes[0x011] = GLFW_KEY_W;
	_glfw.win32.keycodes[0x02D] = GLFW_KEY_X;
	_glfw.win32.keycodes[0x015] = GLFW_KEY_Y;
	_glfw.win32.keycodes[0x02C] = GLFW_KEY_Z;

	_glfw.win32.keycodes[0x028] = GLFW_KEY_APOSTROPHE;
	_glfw.win32.keycodes[0x02B] = GLFW_KEY_BACKSLASH;
	_glfw.win32.keycodes[0x033] = GLFW_KEY_COMMA;
	_glfw.win32.keycodes[0x00D] = GLFW_KEY_EQUAL;
	_glfw.win32.keycodes[0x029] = GLFW_KEY_GRAVE_ACCENT;
	_glfw.win32.keycodes[0x01A] = GLFW_KEY_LEFT_BRACKET;
	_glfw.win32.keycodes[0x00C] = GLFW_KEY_MINUS;
	_glfw.win32.keycodes[0x034] = GLFW_KEY_PERIOD;
	_glfw.win32.keycodes[0x01B] = GLFW_KEY_RIGHT_BRACKET;
	_glfw.win32.keycodes[0x027] = GLFW_KEY_SEMICOLON;
	_glfw.win32.keycodes[0x035] = GLFW_KEY_SLASH;
	_glfw.win32.keycodes[0x056] = GLFW_KEY_WORLD_2;

	_glfw.win32.keycodes[0x00E] = GLFW_KEY_BACKSPACE;
	_glfw.win32.keycodes[0x153] = GLFW_KEY_DELETE;
	_glfw.win32.keycodes[0x14F] = GLFW_KEY_END;
	_glfw.win32.keycodes[0x01C] = GLFW_KEY_ENTER;
	_glfw.win32.keycodes[0x001] = GLFW_KEY_ESCAPE;
	_glfw.win32.keycodes[0x147] = GLFW_KEY_HOME;
	_glfw.win32.keycodes[0x152] = GLFW_KEY_INSERT;
	_glfw.win32.keycodes[0x15D] = GLFW_KEY_MENU;
	_glfw.win32.keycodes[0x151] = GLFW_KEY_PAGE_DOWN;
	_glfw.win32.keycodes[0x149] = GLFW_KEY_PAGE_UP;
	_glfw.win32.keycodes[0x045] = GLFW_KEY_PAUSE;
	_glfw.win32.keycodes[0x039] = GLFW_KEY_SPACE;
	_glfw.win32.keycodes[0x00F] = GLFW_KEY_TAB;
	_glfw.win32.keycodes[0x03A] = GLFW_KEY_CAPS_LOCK;
	_glfw.win32.keycodes[0x145] = GLFW_KEY_NUM_LOCK;
	_glfw.win32.keycodes[0x046] = GLFW_KEY_SCROLL_LOCK;
	_glfw.win32.keycodes[0x03B] = GLFW_KEY_F1;
	_glfw.win32.keycodes[0x03C] = GLFW_KEY_F2;
	_glfw.win32.keycodes[0x03D] = GLFW_KEY_F3;
	_glfw.win32.keycodes[0x03E] = GLFW_KEY_F4;
	_glfw.win32.keycodes[0x03F] = GLFW_KEY_F5;
	_glfw.win32.keycodes[0x040] = GLFW_KEY_F6;
	_glfw.win32.keycodes[0x041] = GLFW_KEY_F7;
	_glfw.win32.keycodes[0x042] = GLFW_KEY_F8;
	_glfw.win32.keycodes[0x043] = GLFW_KEY_F9;
	_glfw.win32.keycodes[0x044] = GLFW_KEY_F10;
	_glfw.win32.keycodes[0x057] = GLFW_KEY_F11;
	_glfw.win32.keycodes[0x058] = GLFW_KEY_F12;
	_glfw.win32.keycodes[0x064] = GLFW_KEY_F13;
	_glfw.win32.keycodes[0x065] = GLFW_KEY_F14;
	_glfw.win32.keycodes[0x066] = GLFW_KEY_F15;
	_glfw.win32.keycodes[0x067] = GLFW_KEY_F16;
	_glfw.win32.keycodes[0x068] = GLFW_KEY_F17;
	_glfw.win32.keycodes[0x069] = GLFW_KEY_F18;
	_glfw.win32.keycodes[0x06A] = GLFW_KEY_F19;
	_glfw.win32.keycodes[0x06B] = GLFW_KEY_F20;
	_glfw.win32.keycodes[0x06C] = GLFW_KEY_F21;
	_glfw.win32.keycodes[0x06D] = GLFW_KEY_F22;
	_glfw.win32.keycodes[0x06E] = GLFW_KEY_F23;
	_glfw.win32.keycodes[0x076] = GLFW_KEY_F24;
	_glfw.win32.keycodes[0x038] = GLFW_KEY_LEFT_ALT;
	_glfw.win32.keycodes[0x01D] = GLFW_KEY_LEFT_CONTROL;
	_glfw.win32.keycodes[0x02A] = GLFW_KEY_LEFT_SHIFT;
	_glfw.win32.keycodes[0x15B] = GLFW_KEY_LEFT_SUPER;
	_glfw.win32.keycodes[0x137] = GLFW_KEY_PRINT_SCREEN;
	_glfw.win32.keycodes[0x138] = GLFW_KEY_RIGHT_ALT;
	_glfw.win32.keycodes[0x11D] = GLFW_KEY_RIGHT_CONTROL;
	_glfw.win32.keycodes[0x036] = GLFW_KEY_RIGHT_SHIFT;
	_glfw.win32.keycodes[0x15C] = GLFW_KEY_RIGHT_SUPER;
	_glfw.win32.keycodes[0x150] = GLFW_KEY_DOWN;
	_glfw.win32.keycodes[0x14B] = GLFW_KEY_LEFT;
	_glfw.win32.keycodes[0x14D] = GLFW_KEY_RIGHT;
	_glfw.win32.keycodes[0x148] = GLFW_KEY_UP;

	_glfw.win32.keycodes[0x052] = GLFW_KEY_KP_0;
	_glfw.win32.keycodes[0x04F] = GLFW_KEY_KP_1;
	_glfw.win32.keycodes[0x050] = GLFW_KEY_KP_2;
	_glfw.win32.keycodes[0x051] = GLFW_KEY_KP_3;
	_glfw.win32.keycodes[0x04B] = GLFW_KEY_KP_4;
	_glfw.win32.keycodes[0x04C] = GLFW_KEY_KP_5;
	_glfw.win32.keycodes[0x04D] = GLFW_KEY_KP_6;
	_glfw.win32.keycodes[0x047] = GLFW_KEY_KP_7;
	_glfw.win32.keycodes[0x048] = GLFW_KEY_KP_8;
	_glfw.win32.keycodes[0x049] = GLFW_KEY_KP_9;
	_glfw.win32.keycodes[0x04E] = GLFW_KEY_KP_ADD;
	_glfw.win32.keycodes[0x053] = GLFW_KEY_KP_DECIMAL;
	_glfw.win32.keycodes[0x135] = GLFW_KEY_KP_DIVIDE;
	_glfw.win32.keycodes[0x11C] = GLFW_KEY_KP_ENTER;
	_glfw.win32.keycodes[0x059] = GLFW_KEY_KP_EQUAL;
	_glfw.win32.keycodes[0x037] = GLFW_KEY_KP_MULTIPLY;
	_glfw.win32.keycodes[0x04A] = GLFW_KEY_KP_SUBTRACT;

	for (scancode = 0; scancode < 512; scancode++)
	{
		if (_glfw.win32.keycodes[scancode] > 0)
			_glfw.win32.scancodes[_glfw.win32.keycodes[scancode]] = scancode;
	}
}

static LRESULT CALLBACK helperWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DISPLAYCHANGE:
		_glfwPollMonitorsWin32();
		break;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static GLFWbool createHelperWindow(void)
{
	MSG msg;
	WNDCLASSEXW wc = { sizeof(wc) };

	wc.style = CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)helperWindowProc;
	wc.hInstance = _glfw.win32.instance;
	wc.lpszClassName = L"GLFW3 Helper";

	_glfw.win32.helperWindowClass = RegisterClassExW(&wc);
	if (!_glfw.win32.helperWindowClass)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR, "Win32: Failed to register helper window class");
		return GLFW_FALSE;
	}

	_glfw.win32.helperWindowHandle =
		CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
			MAKEINTATOM(_glfw.win32.helperWindowClass),
			L"GLFW message window",
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, 1, 1,
			NULL, NULL,
			_glfw.win32.instance,
			NULL);

	if (!_glfw.win32.helperWindowHandle)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR, "Win32: Failed to create helper window");
		return GLFW_FALSE;
	}

	ShowWindow(_glfw.win32.helperWindowHandle, SW_HIDE);

	{
		DEV_BROADCAST_DEVICEINTERFACE_W dbi;
		ZeroMemory(&dbi, sizeof(dbi));
		dbi.dbcc_size = sizeof(dbi);
		dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		dbi.dbcc_classguid = GUID_DEVINTERFACE_HID;

		_glfw.win32.deviceNotificationHandle =
			RegisterDeviceNotificationW(_glfw.win32.helperWindowHandle,
				(DEV_BROADCAST_HDR*)&dbi,
				DEVICE_NOTIFY_WINDOW_HANDLE);
	}

	while (PeekMessageW(&msg, _glfw.win32.helperWindowHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return GLFW_TRUE;
}

WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source)
{
	WCHAR* target;
	int count;

	count = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
	if (!count)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to convert string from UTF-8");
		return NULL;
	}

	target = _glfw_calloc(count, sizeof(WCHAR));

	if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to convert string from UTF-8");
		_glfw_free(target);
		return NULL;
	}

	return target;
}

char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source)
{
	char* target;
	int size;

	size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
	if (!size)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to convert string to UTF-8");
		return NULL;
	}

	target = _glfw_calloc(size, 1);

	if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to convert string to UTF-8");
		_glfw_free(target);
		return NULL;
	}

	return target;
}

void _glfwInputErrorWin32(int error, const char* description)
{
	WCHAR buffer[_GLFW_MESSAGE_SIZE] = L"";
	char message[_GLFW_MESSAGE_SIZE] = "";

	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL,
		GetLastError() & 0xffff,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buffer,
		sizeof(buffer) / sizeof(WCHAR),
		NULL);
	WideCharToMultiByte(CP_UTF8, 0, buffer, -1, message, sizeof(message), NULL, NULL);

	_glfwInputError(error, "%s: %s", description, message);
}

void _glfwUpdateKeyNamesWin32(void)
{
	int key;
	BYTE state[256] = { 0 };

	memset(_glfw.win32.keynames, 0, sizeof(_glfw.win32.keynames));

	for (key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++)
	{
		UINT vk;
		int scancode, length;
		WCHAR chars[16];

		scancode = _glfw.win32.scancodes[key];
		if (scancode == -1)
			continue;

		if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_ADD)
		{
			const UINT vks[] = {
			VK_NUMPAD0,  VK_NUMPAD1,  VK_NUMPAD2, VK_NUMPAD3,
			VK_NUMPAD4,  VK_NUMPAD5,  VK_NUMPAD6, VK_NUMPAD7,
			VK_NUMPAD8,  VK_NUMPAD9,  VK_DECIMAL, VK_DIVIDE,
			VK_MULTIPLY, VK_SUBTRACT, VK_ADD
			};

			vk = vks[key - GLFW_KEY_KP_0];
		}
		else
			vk = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK);

		length = ToUnicode(vk, scancode, state,
			chars, sizeof(chars) / sizeof(WCHAR),
			0);

		if (length == -1)
		{


			length = ToUnicode(vk, scancode, state,
				chars, sizeof(chars) / sizeof(WCHAR),
				0);
		}

		if (length < 1)
			continue;

		WideCharToMultiByte(CP_UTF8, 0, chars, 1,
			_glfw.win32.keynames[key],
			sizeof(_glfw.win32.keynames[key]),
			NULL, NULL);
	}
}

BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp)
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), major, minor, 0, 0, {0}, sp };
	DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
	ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
	cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
	cond = VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	return _glfw.win32.ntdll.RtlVerifyVersionInfo_(&osvi, mask, cond) == 0;
}

BOOL _glfwIsWindows10BuildOrGreaterWin32(WORD build)
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 10, 0, build };
	DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
	ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
	cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
	cond = VerSetConditionMask(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	return _glfw.win32.ntdll.RtlVerifyVersionInfo_(&osvi, mask, cond) == 0;
}

int _glfwInitWin32(void)
{
	if (!loadLibraries())
		return GLFW_FALSE;

	createKeyTables();
	_glfwUpdateKeyNamesWin32();

	if (_glfwIsWindows10Version1703OrGreaterWin32())
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	else if (IsWindows8Point1OrGreater())
		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
	else
		SetProcessDPIAware();

	if (!createHelperWindow())
		return GLFW_FALSE;

	_glfwPollMonitorsWin32();
	return GLFW_TRUE;
}

void _glfwTerminateWin32(void)
{
	if (_glfw.win32.blankCursor)
		DestroyIcon((HICON)_glfw.win32.blankCursor);

	if (_glfw.win32.deviceNotificationHandle)
		UnregisterDeviceNotification(_glfw.win32.deviceNotificationHandle);

	if (_glfw.win32.helperWindowHandle)
		DestroyWindow(_glfw.win32.helperWindowHandle);
	if (_glfw.win32.helperWindowClass)
		UnregisterClassW(MAKEINTATOM(_glfw.win32.helperWindowClass), _glfw.win32.instance);
	if (_glfw.win32.mainWindowClass)
		UnregisterClassW(MAKEINTATOM(_glfw.win32.mainWindowClass), _glfw.win32.instance);

	_glfw_free(_glfw.win32.clipboardString);
	_glfw_free(_glfw.win32.rawInput);

	_glfwTerminateWGL();
	freeLibraries();
}

void* _glfwPlatformLoadModule(const char* path)
{
	return LoadLibraryA(path);
}

void _glfwPlatformFreeModule(void* module)
{
	FreeLibrary((HMODULE)module);
}

GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name)
{
	return (GLFWproc)GetProcAddress((HMODULE)module, name);
}

static BOOL CALLBACK monitorCallback(HMONITOR handle,
	HDC dc,
	RECT* rect,
	LPARAM data)
{
	MONITORINFOEXW mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);

	if (GetMonitorInfoW(handle, (MONITORINFO*)&mi))
	{
		_GLFWmonitor* monitor = (_GLFWmonitor*)data;
		if (wcscmp(mi.szDevice, monitor->win32.adapterName) == 0)
			monitor->win32.handle = handle;
	}

	return TRUE;
}

static _GLFWmonitor* createMonitor(DISPLAY_DEVICEW* adapter,
	DISPLAY_DEVICEW* display)
{
	_GLFWmonitor* monitor;
	int widthMM, heightMM;
	char* name;
	HDC dc;
	DEVMODEW dm;
	RECT rect;

	if (display)
		name = _glfwCreateUTF8FromWideStringWin32(display->DeviceString);
	else
		name = _glfwCreateUTF8FromWideStringWin32(adapter->DeviceString);
	if (!name)
		return NULL;

	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);
	EnumDisplaySettingsW(adapter->DeviceName, ENUM_CURRENT_SETTINGS, &dm);

	dc = CreateDCW(L"DISPLAY", adapter->DeviceName, NULL, NULL);

	if (IsWindows8Point1OrGreater())
	{
		widthMM = GetDeviceCaps(dc, HORZSIZE);
		heightMM = GetDeviceCaps(dc, VERTSIZE);
	}
	else
	{
		widthMM = (int)(dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc, LOGPIXELSX));
		heightMM = (int)(dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc, LOGPIXELSY));
	}

	DeleteDC(dc);

	monitor = _glfwAllocMonitor(name, widthMM, heightMM);
	_glfw_free(name);

	if (adapter->StateFlags & DISPLAY_DEVICE_MODESPRUNED)
		monitor->win32.modesPruned = GLFW_TRUE;

	wcscpy(monitor->win32.adapterName, adapter->DeviceName);
	WideCharToMultiByte(CP_UTF8, 0,
		adapter->DeviceName, -1,
		monitor->win32.publicAdapterName,
		sizeof(monitor->win32.publicAdapterName),
		NULL, NULL);

	if (display)
	{
		wcscpy(monitor->win32.displayName, display->DeviceName);
		WideCharToMultiByte(CP_UTF8, 0,
			display->DeviceName, -1,
			monitor->win32.publicDisplayName,
			sizeof(monitor->win32.publicDisplayName),
			NULL, NULL);
	}

	rect.left = dm.dmPosition.x;
	rect.top = dm.dmPosition.y;
	rect.right = dm.dmPosition.x + dm.dmPelsWidth;
	rect.bottom = dm.dmPosition.y + dm.dmPelsHeight;

	EnumDisplayMonitors(NULL, &rect, monitorCallback, (LPARAM)monitor);
	return monitor;
}

void _glfwPollMonitorsWin32(void)
{
	int i, disconnectedCount;
	_GLFWmonitor** disconnected = NULL;
	DWORD adapterIndex, displayIndex;
	DISPLAY_DEVICEW adapter, display;
	_GLFWmonitor* monitor;

	disconnectedCount = _glfw.monitorCount;
	if (disconnectedCount)
	{
		disconnected = _glfw_calloc(_glfw.monitorCount, sizeof(_GLFWmonitor*));
		memcpy(disconnected,
			_glfw.monitors,
			_glfw.monitorCount * sizeof(_GLFWmonitor*));
	}

	for (adapterIndex = 0; ; adapterIndex++)
	{
		int type = _GLFW_INSERT_LAST;

		ZeroMemory(&adapter, sizeof(adapter));
		adapter.cb = sizeof(adapter);

		if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0))
			break;

		if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
			continue;

		if (adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			type = _GLFW_INSERT_FIRST;

		for (displayIndex = 0; ; displayIndex++)
		{
			ZeroMemory(&display, sizeof(display));
			display.cb = sizeof(display);

			if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0))
				break;

			if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE))
				continue;

			for (i = 0; i < disconnectedCount; i++)
			{
				if (disconnected[i] &&
					wcscmp(disconnected[i]->win32.displayName,
						display.DeviceName) == 0)
				{
					disconnected[i] = NULL;

					EnumDisplayMonitors(NULL, NULL, monitorCallback, (LPARAM)_glfw.monitors[i]);
					break;
				}
			}

			if (i < disconnectedCount)
				continue;

			monitor = createMonitor(&adapter, &display);
			if (!monitor)
			{
				_glfw_free(disconnected);
				return;
			}

			_glfwInputMonitor(monitor, GLFW_CONNECTED, type);

			type = _GLFW_INSERT_LAST;
		}



		if (displayIndex == 0)
		{
			for (i = 0; i < disconnectedCount; i++)
			{
				if (disconnected[i] &&
					wcscmp(disconnected[i]->win32.adapterName,
						adapter.DeviceName) == 0)
				{
					disconnected[i] = NULL;
					break;
				}
			}

			if (i < disconnectedCount)
				continue;

			monitor = createMonitor(&adapter, NULL);
			if (!monitor)
			{
				_glfw_free(disconnected);
				return;
			}

			_glfwInputMonitor(monitor, GLFW_CONNECTED, type);
		}
	}

	for (i = 0; i < disconnectedCount; i++)
	{
		if (disconnected[i])
			_glfwInputMonitor(disconnected[i], GLFW_DISCONNECTED, 0);
	}

	_glfw_free(disconnected);
}

void _glfwSetVideoModeWin32(_GLFWmonitor* monitor, const GLFWvidmode* desired)
{
	GLFWvidmode current;
	const GLFWvidmode* best;
	DEVMODEW dm;
	LONG result;

	best = _glfwChooseVideoMode(monitor, desired);
	_glfwGetVideoModeWin32(monitor, &current);
	if (_glfwCompareVideoModes(&current, best) == 0)
		return;

	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL |
		DM_DISPLAYFREQUENCY;
	dm.dmPelsWidth = best->width;
	dm.dmPelsHeight = best->height;
	dm.dmBitsPerPel = best->redBits + best->greenBits + best->blueBits;
	dm.dmDisplayFrequency = best->refreshRate;

	if (dm.dmBitsPerPel < 15 || dm.dmBitsPerPel >= 24)
		dm.dmBitsPerPel = 32;

	result = ChangeDisplaySettingsExW(monitor->win32.adapterName,
		&dm,
		NULL,
		CDS_FULLSCREEN,
		NULL);
	if (result == DISP_CHANGE_SUCCESSFUL)
		monitor->win32.modeChanged = GLFW_TRUE;
	else
	{
		const char* description = "Unknown error";

		if (result == DISP_CHANGE_BADDUALVIEW)
			description = "The system uses DualView";
		else if (result == DISP_CHANGE_BADFLAGS)
			description = "Invalid flags";
		else if (result == DISP_CHANGE_BADMODE)
			description = "Graphics mode not supported";
		else if (result == DISP_CHANGE_BADPARAM)
			description = "Invalid parameter";
		else if (result == DISP_CHANGE_FAILED)
			description = "Graphics mode failed";
		else if (result == DISP_CHANGE_NOTUPDATED)
			description = "Failed to write to registry";
		else if (result == DISP_CHANGE_RESTART)
			description = "Computer restart required";

		_glfwInputError(GLFW_PLATFORM_ERROR,
			"Win32: Failed to set video mode: %s",
			description);
	}
}

void _glfwRestoreVideoModeWin32(_GLFWmonitor* monitor)
{
	if (monitor->win32.modeChanged)
	{
		ChangeDisplaySettingsExW(monitor->win32.adapterName,
			NULL, NULL, CDS_FULLSCREEN, NULL);
		monitor->win32.modeChanged = GLFW_FALSE;
	}
}

void _glfwGetHMONITORContentScaleWin32(HMONITOR handle, float* xscale, float* yscale)
{
	UINT xdpi, ydpi;

	if (xscale)
		*xscale = 0.f;
	if (yscale)
		*yscale = 0.f;

	if (IsWindows8Point1OrGreater())
	{
		if (GetDpiForMonitor(handle, MDT_EFFECTIVE_DPI, &xdpi, &ydpi) != S_OK)
		{
			_glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to query monitor DPI");
			return;
		}
	}
	else
	{
		const HDC dc = GetDC(NULL);
		xdpi = GetDeviceCaps(dc, LOGPIXELSX);
		ydpi = GetDeviceCaps(dc, LOGPIXELSY);
		ReleaseDC(NULL, dc);
	}

	if (xscale)
		*xscale = xdpi / (float)USER_DEFAULT_SCREEN_DPI;
	if (yscale)
		*yscale = ydpi / (float)USER_DEFAULT_SCREEN_DPI;
}

void _glfwGetMonitorPosWin32(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
	DEVMODEW dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);

	EnumDisplaySettingsExW(monitor->win32.adapterName,
		ENUM_CURRENT_SETTINGS,
		&dm,
		EDS_ROTATEDMODE);

	if (xpos)
		*xpos = dm.dmPosition.x;
	if (ypos)
		*ypos = dm.dmPosition.y;
}

void _glfwGetMonitorContentScaleWin32(_GLFWmonitor* monitor,
	float* xscale, float* yscale)
{
	_glfwGetHMONITORContentScaleWin32(monitor->win32.handle, xscale, yscale);
}

void _glfwGetMonitorWorkareaWin32(_GLFWmonitor* monitor,
	int* xpos, int* ypos,
	int* width, int* height)
{
	MONITORINFO mi = { sizeof(mi) };
	GetMonitorInfoW(monitor->win32.handle, &mi);

	if (xpos)
		*xpos = mi.rcWork.left;
	if (ypos)
		*ypos = mi.rcWork.top;
	if (width)
		*width = mi.rcWork.right - mi.rcWork.left;
	if (height)
		*height = mi.rcWork.bottom - mi.rcWork.top;
}

GLFWvidmode* _glfwGetVideoModesWin32(_GLFWmonitor* monitor, int* count)
{
	int modeIndex = 0, size = 0;
	GLFWvidmode* result = NULL;

	*count = 0;

	for (;;)
	{
		int i;
		GLFWvidmode mode;
		DEVMODEW dm;

		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);

		if (!EnumDisplaySettingsW(monitor->win32.adapterName, modeIndex, &dm))
			break;

		modeIndex++;


		if (dm.dmBitsPerPel < 15)
			continue;

		mode.width = dm.dmPelsWidth;
		mode.height = dm.dmPelsHeight;
		mode.refreshRate = dm.dmDisplayFrequency;
		_glfwSplitBPP(dm.dmBitsPerPel,
			&mode.redBits,
			&mode.greenBits,
			&mode.blueBits);

		for (i = 0; i < *count; i++)
		{
			if (_glfwCompareVideoModes(result + i, &mode) == 0)
				break;
		}


		if (i < *count)
			continue;

		if (monitor->win32.modesPruned)
		{

			if (ChangeDisplaySettingsExW(monitor->win32.adapterName,
				&dm,
				NULL,
				CDS_TEST,
				NULL) != DISP_CHANGE_SUCCESSFUL)
			{
				continue;
			}
		}

		if (*count == size)
		{
			size += 128;
			result = (GLFWvidmode*)_glfw_realloc(result, size * sizeof(GLFWvidmode));
		}

		(*count)++;
		result[*count - 1] = mode;
	}

	if (!*count)
	{

		result = _glfw_calloc(1, sizeof(GLFWvidmode));
		_glfwGetVideoModeWin32(monitor, result);
		*count = 1;
	}

	return result;
}

GLFWbool _glfwGetVideoModeWin32(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
	DEVMODEW dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);

	if (!EnumDisplaySettingsW(monitor->win32.adapterName, ENUM_CURRENT_SETTINGS, &dm))
	{
		_glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to query display settings");
		return GLFW_FALSE;
	}

	mode->width = dm.dmPelsWidth;
	mode->height = dm.dmPelsHeight;
	mode->refreshRate = dm.dmDisplayFrequency;
	_glfwSplitBPP(dm.dmBitsPerPel,
		&mode->redBits,
		&mode->greenBits,
		&mode->blueBits);

	return GLFW_TRUE;
}

GLFWbool _glfwGetGammaRampWin32(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
	HDC dc;
	WORD values[3][256];

	dc = CreateDCW(L"DISPLAY", monitor->win32.adapterName, NULL, NULL);
	GetDeviceGammaRamp(dc, values);
	DeleteDC(dc);

	_glfwAllocGammaArrays(ramp, 256);

	memcpy(ramp->red, values[0], sizeof(values[0]));
	memcpy(ramp->green, values[1], sizeof(values[1]));
	memcpy(ramp->blue, values[2], sizeof(values[2]));

	return GLFW_TRUE;
}

void _glfwSetGammaRampWin32(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
	HDC dc;
	WORD values[3][256];

	if (ramp->size != 256)
	{
		_glfwInputError(GLFW_PLATFORM_ERROR,
			"Win32: Gamma ramp size must be 256");
		return;
	}

	memcpy(values[0], ramp->red, sizeof(values[0]));
	memcpy(values[1], ramp->green, sizeof(values[1]));
	memcpy(values[2], ramp->blue, sizeof(values[2]));

	dc = CreateDCW(L"DISPLAY", monitor->win32.adapterName, NULL, NULL);
	SetDeviceGammaRamp(dc, values);
	DeleteDC(dc);
}

const char* glfwGetWin32Adapter(GLFWmonitor* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	return monitor->win32.publicAdapterName;
}

const char* glfwGetWin32Monitor(GLFWmonitor* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

	_GLFWmonitor* monitor = (_GLFWmonitor*)handle;
	assert(monitor != NULL);

	return monitor->win32.publicDisplayName;
}

void _glfwPlatformInitTimer(void)
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&_glfw.timer.frequency);
}

uint64_t _glfwPlatformGetTimerValue(void)
{
	uint64_t value;
	QueryPerformanceCounter((LARGE_INTEGER*)&value);
	return value;
}

uint64_t _glfwPlatformGetTimerFrequency(void)
{
	return _glfw.timer.frequency;
}

GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls)
{
	assert(tls->win32.allocated == GLFW_FALSE);

	tls->win32.index = TlsAlloc();
	if (tls->win32.index == TLS_OUT_OF_INDEXES)
	{
		_glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to allocate TLS index");
		return GLFW_FALSE;
	}

	tls->win32.allocated = GLFW_TRUE;
	return GLFW_TRUE;
}

void _glfwPlatformDestroyTls(_GLFWtls* tls)
{
	if (tls->win32.allocated)
		TlsFree(tls->win32.index);
	memset(tls, 0, sizeof(_GLFWtls));
}

void* _glfwPlatformGetTls(_GLFWtls* tls)
{
	assert(tls->win32.allocated == GLFW_TRUE);
	return TlsGetValue(tls->win32.index);
}

void _glfwPlatformSetTls(_GLFWtls* tls, void* value)
{
	assert(tls->win32.allocated == GLFW_TRUE);
	TlsSetValue(tls->win32.index, value);
}

GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex)
{
	assert(mutex->win32.allocated == GLFW_FALSE);
	InitializeCriticalSection(&mutex->win32.section);
	return mutex->win32.allocated = GLFW_TRUE;
}

void _glfwPlatformDestroyMutex(_GLFWmutex* mutex)
{
	if (mutex->win32.allocated)
		DeleteCriticalSection(&mutex->win32.section);
	memset(mutex, 0, sizeof(_GLFWmutex));
}

void _glfwPlatformLockMutex(_GLFWmutex* mutex)
{
	assert(mutex->win32.allocated == GLFW_TRUE);
	EnterCriticalSection(&mutex->win32.section);
}

void _glfwPlatformUnlockMutex(_GLFWmutex* mutex)
{
	assert(mutex->win32.allocated == GLFW_TRUE);
	LeaveCriticalSection(&mutex->win32.section);
}

static DWORD getWindowStyle(const _GLFWwindow* window)
{
	DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (window->monitor)
		style |= WS_POPUP;
	else
	{
		style |= WS_SYSMENU | WS_MINIMIZEBOX;

		if (window->decorated)
		{
			style |= WS_CAPTION;

			if (window->resizable)
				style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
		}
		else
			style |= WS_POPUP;
	}

	return style;
}

static DWORD getWindowExStyle(const _GLFWwindow* window)
{
	DWORD style = WS_EX_APPWINDOW;

	if (window->monitor || window->floating)
		style |= WS_EX_TOPMOST;

	return style;
}

static const GLFWimage* chooseImage(int count, const GLFWimage* images,
	int width, int height)
{
	int i, leastDiff = INT_MAX;
	const GLFWimage* closest = NULL;

	for (i = 0; i < count; i++)
	{
		const int currDiff = abs(images[i].width * images[i].height -
			width * height);
		if (currDiff < leastDiff)
		{
			closest = images + i;
			leastDiff = currDiff;
		}
	}

	return closest;
}

static HICON createIcon(const GLFWimage* image, int xhot, int yhot, GLFWbool icon)
{
	int i;
	HDC dc;
	HICON handle;
	HBITMAP color, mask;
	BITMAPV5HEADER bi;
	ICONINFO ii;
	unsigned char* target = NULL;
	unsigned char* source = image->pixels;

	ZeroMemory(&bi, sizeof(bi));
	bi.bV5Size = sizeof(bi);
	bi.bV5Width = image->width;
	bi.bV5Height = -image->height;
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	bi.bV5RedMask = 0x00ff0000;
	bi.bV5GreenMask = 0x0000ff00;
	bi.bV5BlueMask = 0x000000ff;
	bi.bV5AlphaMask = 0xff000000;

	dc = GetDC(NULL);
	color = CreateDIBSection(dc,
		(BITMAPINFO*)&bi,
		DIB_RGB_COLORS,
		(void**)&target,
		NULL,
		(DWORD)0);
	ReleaseDC(NULL, dc);

	if (!color)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to create RGBA bitmap");
		return NULL;
	}

	mask = CreateBitmap(image->width, image->height, 1, 1, NULL);
	if (!mask)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to create mask bitmap");
		DeleteObject(color);
		return NULL;
	}

	for (i = 0; i < image->width * image->height; i++)
	{
		target[0] = source[2];
		target[1] = source[1];
		target[2] = source[0];
		target[3] = source[3];
		target += 4;
		source += 4;
	}

	ZeroMemory(&ii, sizeof(ii));
	ii.fIcon = icon;
	ii.xHotspot = xhot;
	ii.yHotspot = yhot;
	ii.hbmMask = mask;
	ii.hbmColor = color;

	handle = CreateIconIndirect(&ii);

	DeleteObject(color);
	DeleteObject(mask);

	if (!handle)
	{
		if (icon)
		{
			_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
				"Win32: Failed to create icon");
		}
		else
		{
			_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
				"Win32: Failed to create cursor");
		}
	}

	return handle;
}

static void applyAspectRatio(_GLFWwindow* window, int edge, RECT* area)
{
	RECT frame = { 0 };
	const float ratio = (float)window->numer / (float)window->denom;
	const DWORD style = getWindowStyle(window);
	const DWORD exStyle = getWindowExStyle(window);

	if (_glfwIsWindows10Version1607OrGreaterWin32())
	{
		AdjustWindowRectExForDpi(&frame, style, FALSE, exStyle,
			GetDpiForWindow(window->win32.handle));
	}
	else
		AdjustWindowRectEx(&frame, style, FALSE, exStyle);

	if (edge == WMSZ_LEFT || edge == WMSZ_BOTTOMLEFT ||
		edge == WMSZ_RIGHT || edge == WMSZ_BOTTOMRIGHT)
	{
		area->bottom = area->top + (frame.bottom - frame.top) +
			(int)(((area->right - area->left) - (frame.right - frame.left)) / ratio);
	}
	else if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
	{
		area->top = area->bottom - (frame.bottom - frame.top) -
			(int)(((area->right - area->left) - (frame.right - frame.left)) / ratio);
	}
	else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM)
	{
		area->right = area->left + (frame.right - frame.left) +
			(int)(((area->bottom - area->top) - (frame.bottom - frame.top)) * ratio);
	}
}

static void updateCursorImage(_GLFWwindow* window)
{
	if (window->cursorMode == GLFW_CURSOR_NORMAL ||
		window->cursorMode == GLFW_CURSOR_CAPTURED)
	{
		if (window->cursor)
			SetCursor(window->cursor->win32.handle);
		else
			SetCursor(LoadCursorW(NULL, IDC_ARROW));
	}
	else
	{
		SetCursor(_glfw.win32.blankCursor);
	}
}

static void captureCursor(_GLFWwindow* window)
{
	RECT clipRect;
	GetClientRect(window->win32.handle, &clipRect);
	ClientToScreen(window->win32.handle, (POINT*)&clipRect.left);
	ClientToScreen(window->win32.handle, (POINT*)&clipRect.right);
	ClipCursor(&clipRect);
	_glfw.win32.capturedCursorWindow = window;
}

static void releaseCursor(void)
{
	ClipCursor(NULL);
	_glfw.win32.capturedCursorWindow = NULL;
}

static void enableRawMouseMotion(_GLFWwindow* window)
{
	const RAWINPUTDEVICE rid = { 0x01, 0x02, 0, window->win32.handle };

	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to register raw input device");
	}
}

static void disableRawMouseMotion(_GLFWwindow* window)
{
	const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_REMOVE, NULL };

	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to remove raw input device");
	}
}

static void disableCursor(_GLFWwindow* window)
{
	_glfw.win32.disabledCursorWindow = window;
	_glfwGetCursorPosWin32(window,
		&_glfw.win32.restoreCursorPosX,
		&_glfw.win32.restoreCursorPosY);
	updateCursorImage(window);
	_glfwCenterCursorInContentArea(window);
	captureCursor(window);

	if (window->rawMouseMotion)
		enableRawMouseMotion(window);
}

static void enableCursor(_GLFWwindow* window)
{
	if (window->rawMouseMotion)
		disableRawMouseMotion(window);

	_glfw.win32.disabledCursorWindow = NULL;
	releaseCursor();
	_glfwSetCursorPosWin32(window,
		_glfw.win32.restoreCursorPosX,
		_glfw.win32.restoreCursorPosY);
	updateCursorImage(window);
}

static GLFWbool cursorInContentArea(_GLFWwindow* window)
{
	RECT area;
	POINT pos;

	if (!GetCursorPos(&pos))
		return GLFW_FALSE;

	if (WindowFromPoint(pos) != window->win32.handle)
		return GLFW_FALSE;

	GetClientRect(window->win32.handle, &area);
	ClientToScreen(window->win32.handle, (POINT*)&area.left);
	ClientToScreen(window->win32.handle, (POINT*)&area.right);

	return PtInRect(&area, pos);
}

static void updateWindowStyles(const _GLFWwindow* window)
{
	RECT rect;
	DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
	style &= ~(WS_OVERLAPPEDWINDOW | WS_POPUP);
	style |= getWindowStyle(window);

	GetClientRect(window->win32.handle, &rect);

	if (_glfwIsWindows10Version1607OrGreaterWin32())
	{
		AdjustWindowRectExForDpi(&rect, style, FALSE,
			getWindowExStyle(window),
			GetDpiForWindow(window->win32.handle));
	}
	else
		AdjustWindowRectEx(&rect, style, FALSE, getWindowExStyle(window));

	ClientToScreen(window->win32.handle, (POINT*)&rect.left);
	ClientToScreen(window->win32.handle, (POINT*)&rect.right);
	SetWindowLongW(window->win32.handle, GWL_STYLE, style);
	SetWindowPos(window->win32.handle, HWND_TOP,
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOZORDER);
}

static void updateFramebufferTransparency(const _GLFWwindow* window)
{
	BOOL composition, opaque;
	DWORD color;

	if (FAILED(DwmIsCompositionEnabled(&composition)) || !composition)
		return;

	if (IsWindows8OrGreater() ||
		(SUCCEEDED(DwmGetColorizationColor(&color, &opaque)) && !opaque))
	{
		HRGN region = CreateRectRgn(0, 0, -1, -1);
		DWM_BLURBEHIND bb = { 0 };
		bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
		bb.hRgnBlur = region;
		bb.fEnable = TRUE;

		DwmEnableBlurBehindWindow(window->win32.handle, &bb);
		DeleteObject(region);
	}
	else
	{
		DWM_BLURBEHIND bb = { 0 };
		bb.dwFlags = DWM_BB_ENABLE;
		DwmEnableBlurBehindWindow(window->win32.handle, &bb);
	}
}

static int getKeyMods(void)
{
	int mods = 0;

	if (GetKeyState(VK_SHIFT) & 0x8000)
		mods |= GLFW_MOD_SHIFT;
	if (GetKeyState(VK_CONTROL) & 0x8000)
		mods |= GLFW_MOD_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		mods |= GLFW_MOD_ALT;
	if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
		mods |= GLFW_MOD_SUPER;
	if (GetKeyState(VK_CAPITAL) & 1)
		mods |= GLFW_MOD_CAPS_LOCK;
	if (GetKeyState(VK_NUMLOCK) & 1)
		mods |= GLFW_MOD_NUM_LOCK;

	return mods;
}

static void fitToMonitor(_GLFWwindow* window)
{
	MONITORINFO mi = { sizeof(mi) };
	GetMonitorInfoW(window->monitor->win32.handle, &mi);
	SetWindowPos(window->win32.handle, HWND_TOPMOST,
		mi.rcMonitor.left,
		mi.rcMonitor.top,
		mi.rcMonitor.right - mi.rcMonitor.left,
		mi.rcMonitor.bottom - mi.rcMonitor.top,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

static void acquireMonitor(_GLFWwindow* window)
{
	if (!_glfw.win32.acquiredMonitorCount)
	{
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);

		SystemParametersInfoW(SPI_GETMOUSETRAILS, 0, &_glfw.win32.mouseTrailSize, 0);
		SystemParametersInfoW(SPI_SETMOUSETRAILS, 0, 0, 0);
	}

	if (!window->monitor->window)
		_glfw.win32.acquiredMonitorCount++;

	_glfwSetVideoModeWin32(window->monitor, &window->videoMode);
	_glfwInputMonitorWindow(window->monitor, window);
}

static void releaseMonitor(_GLFWwindow* window)
{
	if (window->monitor->window != window)
		return;

	_glfw.win32.acquiredMonitorCount--;
	if (!_glfw.win32.acquiredMonitorCount)
	{
		SetThreadExecutionState(ES_CONTINUOUS);


		SystemParametersInfoW(SPI_SETMOUSETRAILS, _glfw.win32.mouseTrailSize, 0, 0);
	}

	_glfwInputMonitorWindow(window->monitor, NULL);
	_glfwRestoreVideoModeWin32(window->monitor);
}

static void maximizeWindowManually(_GLFWwindow* window)
{
	RECT rect;
	DWORD style;
	MONITORINFO mi = { sizeof(mi) };

	GetMonitorInfoW(MonitorFromWindow(window->win32.handle,
		MONITOR_DEFAULTTONEAREST), &mi);

	rect = mi.rcWork;

	if (window->maxwidth != GLFW_DONT_CARE && window->maxheight != GLFW_DONT_CARE)
	{
		rect.right = _glfw_min(rect.right, rect.left + window->maxwidth);
		rect.bottom = _glfw_min(rect.bottom, rect.top + window->maxheight);
	}

	style = GetWindowLongW(window->win32.handle, GWL_STYLE);
	style |= WS_MAXIMIZE;
	SetWindowLongW(window->win32.handle, GWL_STYLE, style);

	if (window->decorated)
	{
		const DWORD exStyle = GetWindowLongW(window->win32.handle, GWL_EXSTYLE);

		if (_glfwIsWindows10Version1607OrGreaterWin32())
		{
			const UINT dpi = GetDpiForWindow(window->win32.handle);
			AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle, dpi);
			OffsetRect(&rect, 0, _glfw.win32.user32.GetSystemMetricsForDpi_(SM_CYCAPTION, dpi));
		}
		else
		{
			AdjustWindowRectEx(&rect, style, FALSE, exStyle);
			OffsetRect(&rect, 0, GetSystemMetrics(SM_CYCAPTION));
		}

		rect.bottom = _glfw_min(rect.bottom, mi.rcWork.bottom);
	}

	SetWindowPos(window->win32.handle, HWND_TOP,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_GLFWwindow* window = GetPropW(hWnd, L"GLFW");
	if (!window)
	{
		if (uMsg == WM_NCCREATE)
		{
			if (_glfwIsWindows10Version1607OrGreaterWin32())
			{
				const CREATESTRUCTW* cs = (const CREATESTRUCTW*)lParam;
				const _GLFWwndconfig* wndconfig = cs->lpCreateParams;

				if (wndconfig && wndconfig->scaleToMonitor)
					EnableNonClientDpiScaling(hWnd);
			}
		}

		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_MOUSEACTIVATE:
	{
		if (HIWORD(lParam) == WM_LBUTTONDOWN)
		{
			if (LOWORD(lParam) != HTCLIENT)
				window->win32.frameAction = GLFW_TRUE;
		}

		break;
	}

	case WM_CAPTURECHANGED:
	{
		if (lParam == 0 && window->win32.frameAction)
		{
			if (window->cursorMode == GLFW_CURSOR_DISABLED)
				disableCursor(window);
			else if (window->cursorMode == GLFW_CURSOR_CAPTURED)
				captureCursor(window);

			window->win32.frameAction = GLFW_FALSE;
		}

		break;
	}

	case WM_SETFOCUS:
	{
		_glfwInputWindowFocus(window, GLFW_TRUE);



		if (window->win32.frameAction)
			break;

		if (window->cursorMode == GLFW_CURSOR_DISABLED)
			disableCursor(window);
		else if (window->cursorMode == GLFW_CURSOR_CAPTURED)
			captureCursor(window);

		return 0;
	}

	case WM_KILLFOCUS:
	{
		if (window->cursorMode == GLFW_CURSOR_DISABLED)
			enableCursor(window);
		else if (window->cursorMode == GLFW_CURSOR_CAPTURED)
			releaseCursor();

		if (window->monitor && window->autoIconify)
			_glfwIconifyWindowWin32(window);

		_glfwInputWindowFocus(window, GLFW_FALSE);
		return 0;
	}

	case WM_SYSCOMMAND:
	{
		switch (wParam & 0xfff0)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
		{
			if (window->monitor)
			{
				return 0;
			}
			else
				break;
		}


		case SC_KEYMENU:
		{
			if (!window->win32.keymenu)
				return 0;

			break;
		}
		}
		break;
	}

	case WM_CLOSE:
	{
		_glfwInputWindowCloseRequest(window);
		return 0;
	}

	case WM_INPUTLANGCHANGE:
	{
		_glfwUpdateKeyNamesWin32();
		break;
	}

	case WM_CHAR:
	case WM_SYSCHAR:
	{
		if (wParam >= 0xd800 && wParam <= 0xdbff)
			window->win32.highSurrogate = (WCHAR)wParam;
		else
		{
			uint32_t codepoint = 0;

			if (wParam >= 0xdc00 && wParam <= 0xdfff)
			{
				if (window->win32.highSurrogate)
				{
					codepoint += (window->win32.highSurrogate - 0xd800) << 10;
					codepoint += (WCHAR)wParam - 0xdc00;
					codepoint += 0x10000;
				}
			}
			else
				codepoint = (WCHAR)wParam;

			window->win32.highSurrogate = 0;
			_glfwInputChar(window, codepoint, getKeyMods(), uMsg != WM_SYSCHAR);
		}

		if (uMsg == WM_SYSCHAR && window->win32.keymenu)
			break;

		return 0;
	}

	case WM_UNICHAR:
	{
		if (wParam == UNICODE_NOCHAR)
		{
			return TRUE;
		}

		_glfwInputChar(window, (uint32_t)wParam, getKeyMods(), GLFW_TRUE);
		return 0;
	}

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		int key, scancode;
		const int action = (HIWORD(lParam) & KF_UP) ? GLFW_RELEASE : GLFW_PRESS;
		const int mods = getKeyMods();

		scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
		if (!scancode)
		{
			scancode = MapVirtualKeyW((UINT)wParam, MAPVK_VK_TO_VSC);
		}

		if (scancode == 0x54)
			scancode = 0x137;

		if (scancode == 0x146)
			scancode = 0x45;

		if (scancode == 0x136)
			scancode = 0x36;

		key = _glfw.win32.keycodes[scancode];


		if (wParam == VK_CONTROL)
		{
			if (HIWORD(lParam) & KF_EXTENDED)
			{

				key = GLFW_KEY_RIGHT_CONTROL;
			}
			else
			{
				MSG next;
				const DWORD time = GetMessageTime();

				if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
				{
					if (next.message == WM_KEYDOWN ||
						next.message == WM_SYSKEYDOWN ||
						next.message == WM_KEYUP ||
						next.message == WM_SYSKEYUP)
					{
						if (next.wParam == VK_MENU &&
							(HIWORD(next.lParam) & KF_EXTENDED) &&
							next.time == time)
						{

							break;
						}
					}
				}


				key = GLFW_KEY_LEFT_CONTROL;
			}
		}
		else if (wParam == VK_PROCESSKEY)
		{
			break;
		}

		if (action == GLFW_RELEASE && wParam == VK_SHIFT)
		{
			_glfwInputKey(window, GLFW_KEY_LEFT_SHIFT, scancode, action, mods);
			_glfwInputKey(window, GLFW_KEY_RIGHT_SHIFT, scancode, action, mods);
		}
		else if (wParam == VK_SNAPSHOT)
		{

			_glfwInputKey(window, key, scancode, GLFW_PRESS, mods);
			_glfwInputKey(window, key, scancode, GLFW_RELEASE, mods);
		}
		else
			_glfwInputKey(window, key, scancode, action, mods);

		break;
	}

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		int i, button, action;

		if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
			button = GLFW_MOUSE_BUTTON_LEFT;
		else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
			button = GLFW_MOUSE_BUTTON_RIGHT;
		else if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP)
			button = GLFW_MOUSE_BUTTON_MIDDLE;
		else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
			button = GLFW_MOUSE_BUTTON_4;
		else
			button = GLFW_MOUSE_BUTTON_5;

		if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
			uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN)
		{
			action = GLFW_PRESS;
		}
		else
			action = GLFW_RELEASE;

		for (i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++)
		{
			if (window->mouseButtons[i] == GLFW_PRESS)
				break;
		}

		if (i > GLFW_MOUSE_BUTTON_LAST)
			SetCapture(hWnd);

		_glfwInputMouseClick(window, button, action, getKeyMods());

		for (i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++)
		{
			if (window->mouseButtons[i] == GLFW_PRESS)
				break;
		}

		if (i > GLFW_MOUSE_BUTTON_LAST)
			ReleaseCapture();

		if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP)
			return TRUE;

		return 0;
	}

	case WM_MOUSEMOVE:
	{
		const int x = GET_X_LPARAM(lParam);
		const int y = GET_Y_LPARAM(lParam);

		if (!window->win32.cursorTracked)
		{
			TRACKMOUSEEVENT tme;
			ZeroMemory(&tme, sizeof(tme));
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = window->win32.handle;
			TrackMouseEvent(&tme);

			window->win32.cursorTracked = GLFW_TRUE;
			_glfwInputCursorEnter(window, GLFW_TRUE);
		}

		if (window->cursorMode == GLFW_CURSOR_DISABLED)
		{
			const int dx = x - window->win32.lastCursorPosX;
			const int dy = y - window->win32.lastCursorPosY;

			if (_glfw.win32.disabledCursorWindow != window)
				break;
			if (window->rawMouseMotion)
				break;

			_glfwInputCursorPos(window,
				window->virtualCursorPosX + dx,
				window->virtualCursorPosY + dy);
		}
		else
			_glfwInputCursorPos(window, x, y);

		window->win32.lastCursorPosX = x;
		window->win32.lastCursorPosY = y;

		return 0;
	}

	case WM_INPUT:
	{
		UINT size = 0;
		HRAWINPUT ri = (HRAWINPUT)lParam;
		RAWINPUT* data = NULL;
		int dx, dy;

		if (_glfw.win32.disabledCursorWindow != window)
			break;
		if (!window->rawMouseMotion)
			break;

		GetRawInputData(ri, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
		if (size > (UINT)_glfw.win32.rawInputSize)
		{
			_glfw_free(_glfw.win32.rawInput);
			_glfw.win32.rawInput = _glfw_calloc(size, 1);
			_glfw.win32.rawInputSize = size;
		}

		size = _glfw.win32.rawInputSize;
		if (GetRawInputData(ri, RID_INPUT,
			_glfw.win32.rawInput, &size,
			sizeof(RAWINPUTHEADER)) == (UINT)-1)
		{
			_glfwInputError(GLFW_PLATFORM_ERROR,
				"Win32: Failed to retrieve raw input data");
			break;
		}

		data = _glfw.win32.rawInput;
		if (data->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			POINT pos = { 0 };
			int width, height;

			if (data->data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
			{
				pos.x += GetSystemMetrics(SM_XVIRTUALSCREEN);
				pos.y += GetSystemMetrics(SM_YVIRTUALSCREEN);
				width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
				height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
			}
			else
			{
				width = GetSystemMetrics(SM_CXSCREEN);
				height = GetSystemMetrics(SM_CYSCREEN);
			}

			pos.x += (int)((data->data.mouse.lLastX / 65535.f) * width);
			pos.y += (int)((data->data.mouse.lLastY / 65535.f) * height);
			ScreenToClient(window->win32.handle, &pos);

			dx = pos.x - window->win32.lastCursorPosX;
			dy = pos.y - window->win32.lastCursorPosY;
		}
		else
		{
			dx = data->data.mouse.lLastX;
			dy = data->data.mouse.lLastY;
		}

		_glfwInputCursorPos(window,
			window->virtualCursorPosX + dx,
			window->virtualCursorPosY + dy);

		window->win32.lastCursorPosX += dx;
		window->win32.lastCursorPosY += dy;
		break;
	}

	case WM_MOUSELEAVE:
	{
		window->win32.cursorTracked = GLFW_FALSE;
		_glfwInputCursorEnter(window, GLFW_FALSE);
		return 0;
	}

	case WM_MOUSEWHEEL:
	{
		_glfwInputScroll(window, 0.0, (SHORT)HIWORD(wParam) / (double)WHEEL_DELTA);
		return 0;
	}

	case WM_MOUSEHWHEEL:
	{

		_glfwInputScroll(window, -((SHORT)HIWORD(wParam) / (double)WHEEL_DELTA), 0.0);
		return 0;
	}

	case WM_ENTERSIZEMOVE:
	case WM_ENTERMENULOOP:
	{
		if (window->win32.frameAction)
			break;

		if (window->cursorMode == GLFW_CURSOR_DISABLED)
			enableCursor(window);
		else if (window->cursorMode == GLFW_CURSOR_CAPTURED)
			releaseCursor();

		break;
	}

	case WM_EXITSIZEMOVE:
	case WM_EXITMENULOOP:
	{
		if (window->win32.frameAction)
			break;

		if (window->cursorMode == GLFW_CURSOR_DISABLED)
			disableCursor(window);
		else if (window->cursorMode == GLFW_CURSOR_CAPTURED)
			captureCursor(window);

		break;
	}

	case WM_SIZE:
	{
		const int width = LOWORD(lParam);
		const int height = HIWORD(lParam);
		const GLFWbool iconified = wParam == SIZE_MINIMIZED;
		const GLFWbool maximized = wParam == SIZE_MAXIMIZED ||
			(window->win32.maximized &&
				wParam != SIZE_RESTORED);

		if (_glfw.win32.capturedCursorWindow == window)
			captureCursor(window);

		if (window->win32.iconified != iconified)
			_glfwInputWindowIconify(window, iconified);

		if (window->win32.maximized != maximized)
			_glfwInputWindowMaximize(window, maximized);

		if (width != window->win32.width || height != window->win32.height)
		{
			window->win32.width = width;
			window->win32.height = height;

			_glfwInputFramebufferSize(window, width, height);
			_glfwInputWindowSize(window, width, height);
		}

		if (window->monitor && window->win32.iconified != iconified)
		{
			if (iconified)
				releaseMonitor(window);
			else
			{
				acquireMonitor(window);
				fitToMonitor(window);
			}
		}

		window->win32.iconified = iconified;
		window->win32.maximized = maximized;
		return 0;
	}

	case WM_MOVE:
	{
		if (_glfw.win32.capturedCursorWindow == window)
			captureCursor(window);

		_glfwInputWindowPos(window,
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam));
		return 0;
	}

	case WM_SIZING:
	{
		if (window->numer == GLFW_DONT_CARE ||
			window->denom == GLFW_DONT_CARE)
		{
			break;
		}

		applyAspectRatio(window, (int)wParam, (RECT*)lParam);
		return TRUE;
	}

	case WM_GETMINMAXINFO:
	{
		RECT frame = { 0 };
		MINMAXINFO* mmi = (MINMAXINFO*)lParam;
		const DWORD style = getWindowStyle(window);
		const DWORD exStyle = getWindowExStyle(window);

		if (window->monitor)
			break;

		if (_glfwIsWindows10Version1607OrGreaterWin32())
		{
			AdjustWindowRectExForDpi(&frame, style, FALSE, exStyle,
				GetDpiForWindow(window->win32.handle));
		}
		else
			AdjustWindowRectEx(&frame, style, FALSE, exStyle);

		if (window->minwidth != GLFW_DONT_CARE &&
			window->minheight != GLFW_DONT_CARE)
		{
			mmi->ptMinTrackSize.x = window->minwidth + frame.right - frame.left;
			mmi->ptMinTrackSize.y = window->minheight + frame.bottom - frame.top;
		}

		if (window->maxwidth != GLFW_DONT_CARE &&
			window->maxheight != GLFW_DONT_CARE)
		{
			mmi->ptMaxTrackSize.x = window->maxwidth + frame.right - frame.left;
			mmi->ptMaxTrackSize.y = window->maxheight + frame.bottom - frame.top;
		}

		if (!window->decorated)
		{
			MONITORINFO mi;
			const HMONITOR mh = MonitorFromWindow(window->win32.handle,
				MONITOR_DEFAULTTONEAREST);

			ZeroMemory(&mi, sizeof(mi));
			mi.cbSize = sizeof(mi);
			GetMonitorInfoW(mh, &mi);

			mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
			mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
			mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
			mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
		}

		return 0;
	}

	case WM_PAINT:
	{
		_glfwInputWindowDamage(window);
		break;
	}

	case WM_ERASEBKGND:
	{
		return TRUE;
	}

	case WM_NCACTIVATE:
	case WM_NCPAINT:
	{
		if (!window->decorated)
			return TRUE;

		break;
	}

	case WM_DWMCOMPOSITIONCHANGED:
	case WM_DWMCOLORIZATIONCOLORCHANGED:
	{
		if (window->win32.transparent)
			updateFramebufferTransparency(window);
		return 0;
	}

	case WM_GETDPISCALEDSIZE:
	{
		if (window->win32.scaleToMonitor)
			break;

		if (_glfwIsWindows10Version1703OrGreaterWin32())
		{
			RECT source = { 0 }, target = { 0 };
			SIZE* size = (SIZE*)lParam;

			AdjustWindowRectExForDpi(&source, getWindowStyle(window),
				FALSE, getWindowExStyle(window),
				GetDpiForWindow(window->win32.handle));
			AdjustWindowRectExForDpi(&target, getWindowStyle(window),
				FALSE, getWindowExStyle(window),
				LOWORD(wParam));

			size->cx += (target.right - target.left) -
				(source.right - source.left);
			size->cy += (target.bottom - target.top) -
				(source.bottom - source.top);
			return TRUE;
		}

		break;
	}

	case WM_DPICHANGED:
	{
		const float xscale = HIWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;
		const float yscale = LOWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;

		if (!window->monitor &&
			(window->win32.scaleToMonitor ||
				_glfwIsWindows10Version1703OrGreaterWin32()))
		{
			RECT* suggested = (RECT*)lParam;
			SetWindowPos(window->win32.handle, HWND_TOP,
				suggested->left,
				suggested->top,
				suggested->right - suggested->left,
				suggested->bottom - suggested->top,
				SWP_NOACTIVATE | SWP_NOZORDER);
		}

		_glfwInputWindowContentScale(window, xscale, yscale);
		break;
	}

	case WM_SETCURSOR:
	{
		if (LOWORD(lParam) == HTCLIENT)
		{
			updateCursorImage(window);
			return TRUE;
		}

		break;
	}

	case WM_DROPFILES:
	{
		HDROP drop = (HDROP)wParam;
		POINT pt;
		int i;

		const int count = DragQueryFileW(drop, 0xffffffff, NULL, 0);
		char** paths = _glfw_calloc(count, sizeof(char*));


		DragQueryPoint(drop, &pt);
		_glfwInputCursorPos(window, pt.x, pt.y);

		for (i = 0; i < count; i++)
		{
			const UINT length = DragQueryFileW(drop, i, NULL, 0);
			WCHAR* buffer = _glfw_calloc((size_t)length + 1, sizeof(WCHAR));

			DragQueryFileW(drop, i, buffer, length + 1);
			paths[i] = _glfwCreateUTF8FromWideStringWin32(buffer);

			_glfw_free(buffer);
		}

		_glfwInputDrop(window, count, (const char**)paths);

		for (i = 0; i < count; i++)
			_glfw_free(paths[i]);
		_glfw_free(paths);

		DragFinish(drop);
		return 0;
	}
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static int createNativeWindow(_GLFWwindow* window,
	const _GLFWwndconfig* wndconfig,
	const _GLFWfbconfig* fbconfig)
{
	int frameX, frameY, frameWidth, frameHeight;
	WCHAR* wideTitle;
	DWORD style = getWindowStyle(window);
	DWORD exStyle = getWindowExStyle(window);

	if (!_glfw.win32.mainWindowClass)
	{
		WNDCLASSEXW wc = { sizeof(wc) };
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = windowProc;
		wc.hInstance = _glfw.win32.instance;
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.lpszClassName = L"GLFW30";

		wc.hIcon = LoadImageW(GetModuleHandleW(NULL),
			L"GLFW_ICON", IMAGE_ICON,
			0, 0, LR_DEFAULTSIZE | LR_SHARED);
		if (!wc.hIcon)
		{

			wc.hIcon = LoadImageW(NULL,
				IDI_APPLICATION, IMAGE_ICON,
				0, 0, LR_DEFAULTSIZE | LR_SHARED);
		}

		_glfw.win32.mainWindowClass = RegisterClassExW(&wc);
		if (!_glfw.win32.mainWindowClass)
		{
			_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
				"Win32: Failed to register window class");
			return GLFW_FALSE;
		}
	}

	if (window->monitor)
	{
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfoW(window->monitor->win32.handle, &mi);

		frameX = mi.rcMonitor.left;
		frameY = mi.rcMonitor.top;
		frameWidth = mi.rcMonitor.right - mi.rcMonitor.left;
		frameHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}
	else
	{
		RECT rect = { 0, 0, wndconfig->width, wndconfig->height };

		window->win32.maximized = wndconfig->maximized;
		if (wndconfig->maximized)
			style |= WS_MAXIMIZE;

		AdjustWindowRectEx(&rect, style, FALSE, exStyle);

		if (wndconfig->xpos == GLFW_ANY_POSITION && wndconfig->ypos == GLFW_ANY_POSITION)
		{
			frameX = CW_USEDEFAULT;
			frameY = CW_USEDEFAULT;
		}
		else
		{
			frameX = wndconfig->xpos + rect.left;
			frameY = wndconfig->ypos + rect.top;
		}

		frameWidth = rect.right - rect.left;
		frameHeight = rect.bottom - rect.top;
	}

	wideTitle = _glfwCreateWideStringFromUTF8Win32(window->title);
	if (!wideTitle)
		return GLFW_FALSE;

	window->win32.handle = CreateWindowExW(exStyle,
		MAKEINTATOM(_glfw.win32.mainWindowClass),
		wideTitle,
		style,
		frameX, frameY,
		frameWidth, frameHeight,
		NULL,
		NULL,
		_glfw.win32.instance,
		(LPVOID)wndconfig);

	_glfw_free(wideTitle);

	if (!window->win32.handle)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to create window");
		return GLFW_FALSE;
	}

	SetPropW(window->win32.handle, L"GLFW", window);

	ChangeWindowMessageFilterEx(window->win32.handle, WM_DROPFILES, MSGFLT_ALLOW, NULL);
	ChangeWindowMessageFilterEx(window->win32.handle, WM_COPYDATA, MSGFLT_ALLOW, NULL);
	ChangeWindowMessageFilterEx(window->win32.handle, WM_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);

	window->win32.scaleToMonitor = wndconfig->scaleToMonitor;
	window->win32.keymenu = wndconfig->win32.keymenu;
	window->win32.showDefault = wndconfig->win32.showDefault;

	if (!window->monitor)
	{
		RECT rect = { 0, 0, wndconfig->width, wndconfig->height };
		WINDOWPLACEMENT wp = { sizeof(wp) };
		const HMONITOR mh = MonitorFromWindow(window->win32.handle, MONITOR_DEFAULTTONEAREST);

		if (wndconfig->scaleToMonitor)
		{
			float xscale, yscale;
			_glfwGetHMONITORContentScaleWin32(mh, &xscale, &yscale);

			if (xscale > 0.f && yscale > 0.f)
			{
				rect.right = (int)(rect.right * xscale);
				rect.bottom = (int)(rect.bottom * yscale);
			}
		}

		if (_glfwIsWindows10Version1607OrGreaterWin32())
		{
			AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle,
				GetDpiForWindow(window->win32.handle));
		}
		else
			AdjustWindowRectEx(&rect, style, FALSE, exStyle);

		GetWindowPlacement(window->win32.handle, &wp);
		OffsetRect(&rect,
			wp.rcNormalPosition.left - rect.left,
			wp.rcNormalPosition.top - rect.top);

		wp.rcNormalPosition = rect;
		wp.showCmd = SW_HIDE;
		SetWindowPlacement(window->win32.handle, &wp);

		if (wndconfig->maximized && !wndconfig->decorated)
		{
			MONITORINFO mi = { sizeof(mi) };
			GetMonitorInfoW(mh, &mi);

			SetWindowPos(window->win32.handle, HWND_TOP,
				mi.rcWork.left,
				mi.rcWork.top,
				mi.rcWork.right - mi.rcWork.left,
				mi.rcWork.bottom - mi.rcWork.top,
				SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}

	DragAcceptFiles(window->win32.handle, TRUE);

	if (fbconfig->transparent)
	{
		updateFramebufferTransparency(window);
		window->win32.transparent = GLFW_TRUE;
	}

	_glfwGetWindowSizeWin32(window, &window->win32.width, &window->win32.height);

	return GLFW_TRUE;
}

GLFWbool _glfwCreateWindowWin32(_GLFWwindow* window,
	const _GLFWwndconfig* wndconfig,
	const _GLFWctxconfig* ctxconfig,
	const _GLFWfbconfig* fbconfig)
{
	if (!createNativeWindow(window, wndconfig, fbconfig))
		return GLFW_FALSE;

	{
		{
			if (!_glfwInitWGL())
				return GLFW_FALSE;
			if (!_glfwCreateContextWGL(window, ctxconfig, fbconfig))
				return GLFW_FALSE;
		}

		if (!_glfwRefreshContextAttribs(window, ctxconfig))
			return GLFW_FALSE;
	}

	if (wndconfig->mousePassthrough)
		_glfwSetWindowMousePassthroughWin32(window, GLFW_TRUE);

	if (window->monitor)
	{
		_glfwShowWindowWin32(window);
		_glfwFocusWindowWin32(window);
		acquireMonitor(window);
		fitToMonitor(window);

		if (wndconfig->centerCursor)
			_glfwCenterCursorInContentArea(window);
	}
	else
	{
		if (wndconfig->visible)
		{
			_glfwShowWindowWin32(window);
			if (wndconfig->focused)
				_glfwFocusWindowWin32(window);
		}
	}

	return GLFW_TRUE;
}

void _glfwDestroyWindowWin32(_GLFWwindow* window)
{
	if (window->monitor)
		releaseMonitor(window);

	if (window->context.destroy)
		window->context.destroy(window);

	if (_glfw.win32.disabledCursorWindow == window)
		enableCursor(window);

	if (_glfw.win32.capturedCursorWindow == window)
		releaseCursor();

	if (window->win32.handle)
	{
		RemovePropW(window->win32.handle, L"GLFW");
		DestroyWindow(window->win32.handle);
		window->win32.handle = NULL;
	}

	if (window->win32.bigIcon)
		DestroyIcon(window->win32.bigIcon);

	if (window->win32.smallIcon)
		DestroyIcon(window->win32.smallIcon);
}

void _glfwSetWindowTitleWin32(_GLFWwindow* window, const char* title)
{
	WCHAR* wideTitle = _glfwCreateWideStringFromUTF8Win32(title);
	if (!wideTitle)
		return;

	SetWindowTextW(window->win32.handle, wideTitle);
	_glfw_free(wideTitle);
}

void _glfwSetWindowIconWin32(_GLFWwindow* window, int count, const GLFWimage* images)
{
	HICON bigIcon = NULL, smallIcon = NULL;

	if (count)
	{
		const GLFWimage* bigImage = chooseImage(count, images,
			GetSystemMetrics(SM_CXICON),
			GetSystemMetrics(SM_CYICON));
		const GLFWimage* smallImage = chooseImage(count, images,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON));

		bigIcon = createIcon(bigImage, 0, 0, GLFW_TRUE);
		smallIcon = createIcon(smallImage, 0, 0, GLFW_TRUE);
	}
	else
	{
		bigIcon = (HICON)GetClassLongPtrW(window->win32.handle, GCLP_HICON);
		smallIcon = (HICON)GetClassLongPtrW(window->win32.handle, GCLP_HICONSM);
	}

	SendMessageW(window->win32.handle, WM_SETICON, ICON_BIG, (LPARAM)bigIcon);
	SendMessageW(window->win32.handle, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);

	if (window->win32.bigIcon)
		DestroyIcon(window->win32.bigIcon);

	if (window->win32.smallIcon)
		DestroyIcon(window->win32.smallIcon);

	if (count)
	{
		window->win32.bigIcon = bigIcon;
		window->win32.smallIcon = smallIcon;
	}
}

void _glfwGetWindowPosWin32(_GLFWwindow* window, int* xpos, int* ypos)
{
	POINT pos = { 0, 0 };
	ClientToScreen(window->win32.handle, &pos);

	if (xpos)
		*xpos = pos.x;
	if (ypos)
		*ypos = pos.y;
}

void _glfwSetWindowPosWin32(_GLFWwindow* window, int xpos, int ypos)
{
	RECT rect = { xpos, ypos, xpos, ypos };

	if (_glfwIsWindows10Version1607OrGreaterWin32())
	{
		AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
			FALSE, getWindowExStyle(window),
			GetDpiForWindow(window->win32.handle));
	}
	else
	{
		AdjustWindowRectEx(&rect, getWindowStyle(window),
			FALSE, getWindowExStyle(window));
	}

	SetWindowPos(window->win32.handle, NULL, rect.left, rect.top, 0, 0,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void _glfwGetWindowSizeWin32(_GLFWwindow* window, int* width, int* height)
{
	RECT area;
	GetClientRect(window->win32.handle, &area);

	if (width)
		*width = area.right;
	if (height)
		*height = area.bottom;
}

void _glfwSetWindowSizeWin32(_GLFWwindow* window, int width, int height)
{
	if (window->monitor)
	{
		if (window->monitor->window == window)
		{
			acquireMonitor(window);
			fitToMonitor(window);
		}
	}
	else
	{
		RECT rect = { 0, 0, width, height };

		if (_glfwIsWindows10Version1607OrGreaterWin32())
		{
			AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
				FALSE, getWindowExStyle(window),
				GetDpiForWindow(window->win32.handle));
		}
		else
		{
			AdjustWindowRectEx(&rect, getWindowStyle(window),
				FALSE, getWindowExStyle(window));
		}

		SetWindowPos(window->win32.handle, HWND_TOP,
			0, 0, rect.right - rect.left, rect.bottom - rect.top,
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
	}
}

void _glfwSetWindowSizeLimitsWin32(_GLFWwindow* window,
	int minwidth, int minheight,
	int maxwidth, int maxheight)
{
	RECT area;

	if ((minwidth == GLFW_DONT_CARE || minheight == GLFW_DONT_CARE) &&
		(maxwidth == GLFW_DONT_CARE || maxheight == GLFW_DONT_CARE))
	{
		return;
	}

	GetWindowRect(window->win32.handle, &area);
	MoveWindow(window->win32.handle,
		area.left, area.top,
		area.right - area.left,
		area.bottom - area.top, TRUE);
}

void _glfwSetWindowAspectRatioWin32(_GLFWwindow* window, int numer, int denom)
{
	RECT area;

	if (numer == GLFW_DONT_CARE || denom == GLFW_DONT_CARE)
		return;

	GetWindowRect(window->win32.handle, &area);
	applyAspectRatio(window, WMSZ_BOTTOMRIGHT, &area);
	MoveWindow(window->win32.handle,
		area.left, area.top,
		area.right - area.left,
		area.bottom - area.top, TRUE);
}

void _glfwGetWindowFrameSizeWin32(_GLFWwindow* window,
	int* left, int* top,
	int* right, int* bottom)
{
	RECT rect;
	int width, height;

	_glfwGetWindowSizeWin32(window, &width, &height);
	SetRect(&rect, 0, 0, width, height);

	if (_glfwIsWindows10Version1607OrGreaterWin32())
	{
		AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
			FALSE, getWindowExStyle(window),
			GetDpiForWindow(window->win32.handle));
	}
	else
	{
		AdjustWindowRectEx(&rect, getWindowStyle(window),
			FALSE, getWindowExStyle(window));
	}

	if (left)
		*left = -rect.left;
	if (top)
		*top = -rect.top;
	if (right)
		*right = rect.right - width;
	if (bottom)
		*bottom = rect.bottom - height;
}

void _glfwGetWindowContentScaleWin32(_GLFWwindow* window, float* xscale, float* yscale)
{
	const HANDLE handle = MonitorFromWindow(window->win32.handle,
		MONITOR_DEFAULTTONEAREST);
	_glfwGetHMONITORContentScaleWin32(handle, xscale, yscale);
}

void _glfwIconifyWindowWin32(_GLFWwindow* window)
{
	ShowWindow(window->win32.handle, SW_MINIMIZE);
}

void _glfwRestoreWindowWin32(_GLFWwindow* window)
{
	ShowWindow(window->win32.handle, SW_RESTORE);
}

void _glfwMaximizeWindowWin32(_GLFWwindow* window)
{
	if (IsWindowVisible(window->win32.handle))
		ShowWindow(window->win32.handle, SW_MAXIMIZE);
	else
		maximizeWindowManually(window);
}

void _glfwShowWindowWin32(_GLFWwindow* window)
{
	int showCommand = SW_SHOWNA;

	if (window->win32.showDefault)
	{
		STARTUPINFOW si = { sizeof(si) };
		GetStartupInfoW(&si);
		if (si.dwFlags & STARTF_USESHOWWINDOW)
			showCommand = si.wShowWindow;

		window->win32.showDefault = GLFW_FALSE;
	}

	ShowWindow(window->win32.handle, showCommand);
}

void _glfwHideWindowWin32(_GLFWwindow* window)
{
	ShowWindow(window->win32.handle, SW_HIDE);
}

void _glfwRequestWindowAttentionWin32(_GLFWwindow* window)
{
	FlashWindow(window->win32.handle, TRUE);
}

void _glfwFocusWindowWin32(_GLFWwindow* window)
{
	BringWindowToTop(window->win32.handle);
	SetForegroundWindow(window->win32.handle);
	SetFocus(window->win32.handle);
}

void _glfwSetWindowMonitorWin32(_GLFWwindow* window,
	_GLFWmonitor* monitor,
	int xpos, int ypos,
	int width, int height,
	int refreshRate)
{
	if (window->monitor == monitor)
	{
		if (monitor)
		{
			if (monitor->window == window)
			{
				acquireMonitor(window);
				fitToMonitor(window);
			}
		}
		else
		{
			RECT rect = { xpos, ypos, xpos + width, ypos + height };

			if (_glfwIsWindows10Version1607OrGreaterWin32())
			{
				AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
					FALSE, getWindowExStyle(window),
					GetDpiForWindow(window->win32.handle));
			}
			else
			{
				AdjustWindowRectEx(&rect, getWindowStyle(window),
					FALSE, getWindowExStyle(window));
			}

			SetWindowPos(window->win32.handle, HWND_TOP,
				rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOZORDER);
		}

		return;
	}

	if (window->monitor)
		releaseMonitor(window);

	_glfwInputWindowMonitor(window, monitor);

	if (window->monitor)
	{
		MONITORINFO mi = { sizeof(mi) };
		UINT flags = SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS;

		if (window->decorated)
		{
			DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
			style &= ~WS_OVERLAPPEDWINDOW;
			style |= getWindowStyle(window);
			SetWindowLongW(window->win32.handle, GWL_STYLE, style);
			flags |= SWP_FRAMECHANGED;
		}

		acquireMonitor(window);

		GetMonitorInfoW(window->monitor->win32.handle, &mi);
		SetWindowPos(window->win32.handle, HWND_TOPMOST,
			mi.rcMonitor.left,
			mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			flags);
	}
	else
	{
		HWND after;
		RECT rect = { xpos, ypos, xpos + width, ypos + height };
		DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
		UINT flags = SWP_NOACTIVATE | SWP_NOCOPYBITS;

		if (window->decorated)
		{
			style &= ~WS_POPUP;
			style |= getWindowStyle(window);
			SetWindowLongW(window->win32.handle, GWL_STYLE, style);

			flags |= SWP_FRAMECHANGED;
		}

		if (window->floating)
			after = HWND_TOPMOST;
		else
			after = HWND_NOTOPMOST;

		if (_glfwIsWindows10Version1607OrGreaterWin32())
		{
			AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
				FALSE, getWindowExStyle(window),
				GetDpiForWindow(window->win32.handle));
		}
		else
		{
			AdjustWindowRectEx(&rect, getWindowStyle(window),
				FALSE, getWindowExStyle(window));
		}

		SetWindowPos(window->win32.handle, after,
			rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			flags);
	}
}

GLFWbool _glfwWindowFocusedWin32(_GLFWwindow* window)
{
	return window->win32.handle == GetActiveWindow();
}

GLFWbool _glfwWindowIconifiedWin32(_GLFWwindow* window)
{
	return IsIconic(window->win32.handle);
}

GLFWbool _glfwWindowVisibleWin32(_GLFWwindow* window)
{
	return IsWindowVisible(window->win32.handle);
}

GLFWbool _glfwWindowMaximizedWin32(_GLFWwindow* window)
{
	return IsZoomed(window->win32.handle);
}

GLFWbool _glfwWindowHoveredWin32(_GLFWwindow* window)
{
	return cursorInContentArea(window);
}

GLFWbool _glfwFramebufferTransparentWin32(_GLFWwindow* window)
{
	BOOL composition, opaque;
	DWORD color;

	if (!window->win32.transparent)
		return GLFW_FALSE;

	if (FAILED(DwmIsCompositionEnabled(&composition)) || !composition)
		return GLFW_FALSE;

	if (!IsWindows8OrGreater())
	{
		if (FAILED(DwmGetColorizationColor(&color, &opaque)) || opaque)
			return GLFW_FALSE;
	}

	return GLFW_TRUE;
}

void _glfwSetWindowResizableWin32(_GLFWwindow* window, GLFWbool enabled)
{
	updateWindowStyles(window);
}

void _glfwSetWindowDecoratedWin32(_GLFWwindow* window, GLFWbool enabled)
{
	updateWindowStyles(window);
}

void _glfwSetWindowFloatingWin32(_GLFWwindow* window, GLFWbool enabled)
{
	const HWND after = enabled ? HWND_TOPMOST : HWND_NOTOPMOST;
	SetWindowPos(window->win32.handle, after, 0, 0, 0, 0,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

void _glfwSetWindowMousePassthroughWin32(_GLFWwindow* window, GLFWbool enabled)
{
	COLORREF key = 0;
	BYTE alpha = 0;
	DWORD flags = 0;
	DWORD exStyle = GetWindowLongW(window->win32.handle, GWL_EXSTYLE);

	if (exStyle & WS_EX_LAYERED)
		GetLayeredWindowAttributes(window->win32.handle, &key, &alpha, &flags);

	if (enabled)
		exStyle |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
	else
	{
		exStyle &= ~WS_EX_TRANSPARENT;


		if (exStyle & WS_EX_LAYERED)
		{
			if (!(flags & LWA_ALPHA))
				exStyle &= ~WS_EX_LAYERED;
		}
	}

	SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);

	if (enabled)
		SetLayeredWindowAttributes(window->win32.handle, key, alpha, flags);
}

float _glfwGetWindowOpacityWin32(_GLFWwindow* window)
{
	BYTE alpha;
	DWORD flags;

	if ((GetWindowLongW(window->win32.handle, GWL_EXSTYLE) & WS_EX_LAYERED) &&
		GetLayeredWindowAttributes(window->win32.handle, NULL, &alpha, &flags))
	{
		if (flags & LWA_ALPHA)
			return alpha / 255.f;
	}

	return 1.f;
}

void _glfwSetWindowOpacityWin32(_GLFWwindow* window, float opacity)
{
	LONG exStyle = GetWindowLongW(window->win32.handle, GWL_EXSTYLE);
	if (opacity < 1.f || (exStyle & WS_EX_TRANSPARENT))
	{
		const BYTE alpha = (BYTE)(255 * opacity);
		exStyle |= WS_EX_LAYERED;
		SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);
		SetLayeredWindowAttributes(window->win32.handle, 0, alpha, LWA_ALPHA);
	}
	else if (exStyle & WS_EX_TRANSPARENT)
	{
		SetLayeredWindowAttributes(window->win32.handle, 0, 0, 0);
	}
	else
	{
		exStyle &= ~WS_EX_LAYERED;
		SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);
	}
}

void _glfwSetRawMouseMotionWin32(_GLFWwindow* window, GLFWbool enabled)
{
	if (_glfw.win32.disabledCursorWindow != window)
		return;

	if (enabled)
		enableRawMouseMotion(window);
	else
		disableRawMouseMotion(window);
}

void _glfwGetCursorPosWin32(_GLFWwindow* window, double* xpos, double* ypos)
{
	POINT pos;

	if (GetCursorPos(&pos))
	{
		ScreenToClient(window->win32.handle, &pos);

		if (xpos)
			*xpos = pos.x;
		if (ypos)
			*ypos = pos.y;
	}
}

void _glfwSetCursorPosWin32(_GLFWwindow* window, double xpos, double ypos)
{
	POINT pos = { (int)xpos, (int)ypos };


	window->win32.lastCursorPosX = pos.x;
	window->win32.lastCursorPosY = pos.y;

	ClientToScreen(window->win32.handle, &pos);
	SetCursorPos(pos.x, pos.y);
}

void _glfwSetCursorModeWin32(_GLFWwindow* window, int mode)
{
	if (_glfwWindowFocusedWin32(window))
	{
		if (mode == GLFW_CURSOR_DISABLED)
		{
			_glfwGetCursorPosWin32(window,
				&_glfw.win32.restoreCursorPosX,
				&_glfw.win32.restoreCursorPosY);
			_glfwCenterCursorInContentArea(window);
			if (window->rawMouseMotion)
				enableRawMouseMotion(window);
		}
		else if (_glfw.win32.disabledCursorWindow == window)
		{
			if (window->rawMouseMotion)
				disableRawMouseMotion(window);
		}

		if (mode == GLFW_CURSOR_DISABLED || mode == GLFW_CURSOR_CAPTURED)
			captureCursor(window);
		else
			releaseCursor();

		if (mode == GLFW_CURSOR_DISABLED)
			_glfw.win32.disabledCursorWindow = window;
		else if (_glfw.win32.disabledCursorWindow == window)
		{
			_glfw.win32.disabledCursorWindow = NULL;
			_glfwSetCursorPosWin32(window,
				_glfw.win32.restoreCursorPosX,
				_glfw.win32.restoreCursorPosY);
		}
	}

	if (cursorInContentArea(window))
		updateCursorImage(window);
}

const char* _glfwGetScancodeNameWin32(int scancode)
{
	if (scancode < 0 || scancode >(KF_EXTENDED | 0xff))
	{
		_glfwInputError(GLFW_INVALID_VALUE, "Invalid scancode %i", scancode);
		return NULL;
	}

	const int key = _glfw.win32.keycodes[scancode];
	if (key == GLFW_KEY_UNKNOWN)
		return NULL;

	return _glfw.win32.keynames[key];
}

int _glfwGetKeyScancodeWin32(int key)
{
	return _glfw.win32.scancodes[key];
}

GLFWbool _glfwCreateCursorWin32(_GLFWcursor* cursor,
	const GLFWimage* image,
	int xhot, int yhot)
{
	cursor->win32.handle = (HCURSOR)createIcon(image, xhot, yhot, GLFW_FALSE);
	if (!cursor->win32.handle)
		return GLFW_FALSE;

	return GLFW_TRUE;
}

GLFWbool _glfwCreateStandardCursorWin32(_GLFWcursor* cursor, int shape)
{
	int id = 0;

	switch (shape)
	{
	case GLFW_ARROW_CURSOR:
		id = OCR_NORMAL;
		break;
	case GLFW_IBEAM_CURSOR:
		id = OCR_IBEAM;
		break;
	case GLFW_CROSSHAIR_CURSOR:
		id = OCR_CROSS;
		break;
	case GLFW_POINTING_HAND_CURSOR:
		id = OCR_HAND;
		break;
	case GLFW_RESIZE_EW_CURSOR:
		id = OCR_SIZEWE;
		break;
	case GLFW_RESIZE_NS_CURSOR:
		id = OCR_SIZENS;
		break;
	case GLFW_RESIZE_NWSE_CURSOR:
		id = OCR_SIZENWSE;
		break;
	case GLFW_RESIZE_NESW_CURSOR:
		id = OCR_SIZENESW;
		break;
	case GLFW_RESIZE_ALL_CURSOR:
		id = OCR_SIZEALL;
		break;
	case GLFW_NOT_ALLOWED_CURSOR:
		id = OCR_NO;
		break;
	default:
		_glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Unknown standard cursor");
		return GLFW_FALSE;
	}

	cursor->win32.handle = LoadImageW(NULL,
		MAKEINTRESOURCEW(id), IMAGE_CURSOR, 0, 0,
		LR_DEFAULTSIZE | LR_SHARED);
	if (!cursor->win32.handle)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to create standard cursor");
		return GLFW_FALSE;
	}

	return GLFW_TRUE;
}

void _glfwDestroyCursorWin32(_GLFWcursor* cursor)
{
	if (cursor->win32.handle)
		DestroyIcon((HICON)cursor->win32.handle);
}

void _glfwSetCursorWin32(_GLFWwindow* window, _GLFWcursor* cursor)
{
	if (cursorInContentArea(window))
		updateCursorImage(window);
}

void _glfwSetClipboardStringWin32(const char* string)
{
	int characterCount, tries = 0;
	HANDLE object;
	WCHAR* buffer;

	characterCount = MultiByteToWideChar(CP_UTF8, 0, string, -1, NULL, 0);
	if (!characterCount)
		return;

	object = GlobalAlloc(GMEM_MOVEABLE, characterCount * sizeof(WCHAR));
	if (!object)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to allocate global handle for clipboard");
		return;
	}

	buffer = GlobalLock(object);
	if (!buffer)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to lock global handle");
		GlobalFree(object);
		return;
	}

	MultiByteToWideChar(CP_UTF8, 0, string, -1, buffer, characterCount);
	GlobalUnlock(object);

	while (!OpenClipboard(_glfw.win32.helperWindowHandle))
	{
		Sleep(1);
		tries++;

		if (tries == 3)
		{
			_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
				"Win32: Failed to open clipboard");
			GlobalFree(object);
			return;
		}
	}

	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, object);
	CloseClipboard();
}

const char* _glfwGetClipboardStringWin32(void)
{
	HANDLE object;
	WCHAR* buffer;
	int tries = 0;


	while (!OpenClipboard(_glfw.win32.helperWindowHandle))
	{
		Sleep(1);
		tries++;

		if (tries == 3)
		{
			_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
				"Win32: Failed to open clipboard");
			return NULL;
		}
	}

	object = GetClipboardData(CF_UNICODETEXT);
	if (!object)
	{
		_glfwInputErrorWin32(GLFW_FORMAT_UNAVAILABLE,
			"Win32: Failed to convert clipboard to string");
		CloseClipboard();
		return NULL;
	}

	buffer = GlobalLock(object);
	if (!buffer)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"Win32: Failed to lock global handle");
		CloseClipboard();
		return NULL;
	}

	_glfw_free(_glfw.win32.clipboardString);
	_glfw.win32.clipboardString = _glfwCreateUTF8FromWideStringWin32(buffer);

	GlobalUnlock(object);
	CloseClipboard();

	return _glfw.win32.clipboardString;
}

void* glfwGetWin32Window(GLFWwindow* handle)
{
	_GLFW_REQUIRE_INIT_OR_RETURN(NULL);
	_GLFWwindow* window = (_GLFWwindow*)handle;
	assert(window != NULL);
	return window->win32.handle;
}

static int findPixelFormatAttribValueWGL(const int* attribs,
	int attribCount,
	const int* values,
	int attrib)
{
	int i;

	for (i = 0; i < attribCount; i++)
	{
		if (attribs[i] == attrib)
			return values[i];
	}

	_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
		"WGL: Unknown pixel format attribute requested");
	return 0;
}

#define ADD_ATTRIB(a) \
{ \
assert((size_t) attribCount < sizeof(attribs) / sizeof(attribs[0])); \
attribs[attribCount++] = a; \
}
#define FIND_ATTRIB_VALUE(a) \
findPixelFormatAttribValueWGL(attribs, attribCount, values, a)

static int choosePixelFormatWGL(_GLFWwindow* window,
	const _GLFWctxconfig* ctxconfig,
	const _GLFWfbconfig* fbconfig)
{
	_GLFWfbconfig* usableConfigs;
	const _GLFWfbconfig* closest;
	int i, pixelFormat, nativeCount, usableCount = 0, attribCount = 0;
	int attribs[40];
	int values[sizeof(attribs) / sizeof(attribs[0])];

	nativeCount = DescribePixelFormat(window->context.wgl.dc,
		1,
		sizeof(PIXELFORMATDESCRIPTOR),
		NULL);

	if (_glfw.wgl.ARB_pixel_format)
	{
		ADD_ATTRIB(WGL_SUPPORT_OPENGL_ARB);
		ADD_ATTRIB(WGL_DRAW_TO_WINDOW_ARB);
		ADD_ATTRIB(WGL_PIXEL_TYPE_ARB);
		ADD_ATTRIB(WGL_ACCELERATION_ARB);
		ADD_ATTRIB(WGL_RED_BITS_ARB);
		ADD_ATTRIB(WGL_RED_SHIFT_ARB);
		ADD_ATTRIB(WGL_GREEN_BITS_ARB);
		ADD_ATTRIB(WGL_GREEN_SHIFT_ARB);
		ADD_ATTRIB(WGL_BLUE_BITS_ARB);
		ADD_ATTRIB(WGL_BLUE_SHIFT_ARB);
		ADD_ATTRIB(WGL_ALPHA_BITS_ARB);
		ADD_ATTRIB(WGL_ALPHA_SHIFT_ARB);
		ADD_ATTRIB(WGL_DEPTH_BITS_ARB);
		ADD_ATTRIB(WGL_STENCIL_BITS_ARB);
		ADD_ATTRIB(WGL_ACCUM_BITS_ARB);
		ADD_ATTRIB(WGL_ACCUM_RED_BITS_ARB);
		ADD_ATTRIB(WGL_ACCUM_GREEN_BITS_ARB);
		ADD_ATTRIB(WGL_ACCUM_BLUE_BITS_ARB);
		ADD_ATTRIB(WGL_ACCUM_ALPHA_BITS_ARB);
		ADD_ATTRIB(WGL_AUX_BUFFERS_ARB);
		ADD_ATTRIB(WGL_STEREO_ARB);
		ADD_ATTRIB(WGL_DOUBLE_BUFFER_ARB);

		if (_glfw.wgl.ARB_multisample)
			ADD_ATTRIB(WGL_SAMPLES_ARB);

		if (_glfw.wgl.ARB_framebuffer_sRGB || _glfw.wgl.EXT_framebuffer_sRGB)
			ADD_ATTRIB(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
	
		const int attrib = WGL_NUMBER_PIXEL_FORMATS_ARB;
		int extensionCount;

		if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,
			1, 0, 1, &attrib, &extensionCount))
		{
			_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
				"WGL: Failed to retrieve pixel format attribute");
			return 0;
		}

		nativeCount = _glfw_min(nativeCount, extensionCount);
	}

	usableConfigs = _glfw_calloc(nativeCount, sizeof(_GLFWfbconfig));

	for (i = 0; i < nativeCount; i++)
	{
		_GLFWfbconfig* u = usableConfigs + usableCount;
		pixelFormat = i + 1;

		if (_glfw.wgl.ARB_pixel_format)
		{


			if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,
				pixelFormat, 0,
				attribCount,
				attribs, values))
			{
				_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
					"WGL: Failed to retrieve pixel format attributes");

				_glfw_free(usableConfigs);
				return 0;
			}

			if (!FIND_ATTRIB_VALUE(WGL_SUPPORT_OPENGL_ARB) ||
				!FIND_ATTRIB_VALUE(WGL_DRAW_TO_WINDOW_ARB))
			{
				continue;
			}

			if (FIND_ATTRIB_VALUE(WGL_PIXEL_TYPE_ARB) != WGL_TYPE_RGBA_ARB)
				continue;

			if (FIND_ATTRIB_VALUE(WGL_ACCELERATION_ARB) == WGL_NO_ACCELERATION_ARB)
				continue;

			if (FIND_ATTRIB_VALUE(WGL_DOUBLE_BUFFER_ARB) != fbconfig->doublebuffer)
				continue;

			u->redBits = FIND_ATTRIB_VALUE(WGL_RED_BITS_ARB);
			u->greenBits = FIND_ATTRIB_VALUE(WGL_GREEN_BITS_ARB);
			u->blueBits = FIND_ATTRIB_VALUE(WGL_BLUE_BITS_ARB);
			u->alphaBits = FIND_ATTRIB_VALUE(WGL_ALPHA_BITS_ARB);

			u->depthBits = FIND_ATTRIB_VALUE(WGL_DEPTH_BITS_ARB);
			u->stencilBits = FIND_ATTRIB_VALUE(WGL_STENCIL_BITS_ARB);

			u->accumRedBits = FIND_ATTRIB_VALUE(WGL_ACCUM_RED_BITS_ARB);
			u->accumGreenBits = FIND_ATTRIB_VALUE(WGL_ACCUM_GREEN_BITS_ARB);
			u->accumBlueBits = FIND_ATTRIB_VALUE(WGL_ACCUM_BLUE_BITS_ARB);
			u->accumAlphaBits = FIND_ATTRIB_VALUE(WGL_ACCUM_ALPHA_BITS_ARB);

			u->auxBuffers = FIND_ATTRIB_VALUE(WGL_AUX_BUFFERS_ARB);

			if (FIND_ATTRIB_VALUE(WGL_STEREO_ARB))
				u->stereo = GLFW_TRUE;

			if (_glfw.wgl.ARB_multisample)
				u->samples = FIND_ATTRIB_VALUE(WGL_SAMPLES_ARB);

			if (_glfw.wgl.ARB_framebuffer_sRGB ||
				_glfw.wgl.EXT_framebuffer_sRGB)
			{
				if (FIND_ATTRIB_VALUE(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB))
					u->sRGB = GLFW_TRUE;
			}
		}
		else
		{
			PIXELFORMATDESCRIPTOR pfd;

			if (!DescribePixelFormat(window->context.wgl.dc,
				pixelFormat,
				sizeof(PIXELFORMATDESCRIPTOR),
				&pfd))
			{
				_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
					"WGL: Failed to describe pixel format");

				_glfw_free(usableConfigs);
				return 0;
			}

			if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW) ||
				!(pfd.dwFlags & PFD_SUPPORT_OPENGL))
			{
				continue;
			}

			if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) &&
				(pfd.dwFlags & PFD_GENERIC_FORMAT))
			{
				continue;
			}

			if (pfd.iPixelType != PFD_TYPE_RGBA)
				continue;

			if (!!(pfd.dwFlags & PFD_DOUBLEBUFFER) != fbconfig->doublebuffer)
				continue;

			u->redBits = pfd.cRedBits;
			u->greenBits = pfd.cGreenBits;
			u->blueBits = pfd.cBlueBits;
			u->alphaBits = pfd.cAlphaBits;

			u->depthBits = pfd.cDepthBits;
			u->stencilBits = pfd.cStencilBits;

			u->accumRedBits = pfd.cAccumRedBits;
			u->accumGreenBits = pfd.cAccumGreenBits;
			u->accumBlueBits = pfd.cAccumBlueBits;
			u->accumAlphaBits = pfd.cAccumAlphaBits;

			u->auxBuffers = pfd.cAuxBuffers;

			if (pfd.dwFlags & PFD_STEREO)
				u->stereo = GLFW_TRUE;
		}

		u->handle = pixelFormat;
		usableCount++;
	}

	if (!usableCount)
	{
		_glfwInputError(GLFW_API_UNAVAILABLE,
			"WGL: The driver does not appear to support OpenGL");

		_glfw_free(usableConfigs);
		return 0;
	}

	closest = _glfwChooseFBConfig(fbconfig, usableConfigs, usableCount);
	if (!closest)
	{
		_glfwInputError(GLFW_FORMAT_UNAVAILABLE,
			"WGL: Failed to find a suitable pixel format");

		_glfw_free(usableConfigs);
		return 0;
	}

	pixelFormat = (int)closest->handle;
	_glfw_free(usableConfigs);

	return pixelFormat;
}

#undef ADD_ATTRIB
#undef FIND_ATTRIB_VALUE

static void makeContextCurrentWGL(_GLFWwindow* window)
{
	if (window)
	{
		if (wglMakeCurrent(window->context.wgl.dc, window->context.wgl.handle))
			_glfwPlatformSetTls(&_glfw.contextSlot, window);
		else
		{
			_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
				"WGL: Failed to make context current");
			_glfwPlatformSetTls(&_glfw.contextSlot, NULL);
		}
	}
	else
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
			_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
				"WGL: Failed to clear current context");
		}

		_glfwPlatformSetTls(&_glfw.contextSlot, NULL);
	}
}

static void swapBuffersWGL(_GLFWwindow* window)
{
	if (!window->monitor)
	{

		if (!IsWindows8OrGreater())
		{
			BOOL enabled = FALSE;

			if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled)
			{
				int count = abs(window->context.wgl.interval);
				while (count--)
					DwmFlush();
			}
		}
	}

	SwapBuffers(window->context.wgl.dc);
}

static void swapIntervalWGL(int interval)
{
	_GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
	assert(window != NULL);

	window->context.wgl.interval = interval;

	if (!window->monitor)
	{
		if (!IsWindows8OrGreater())
		{
			BOOL enabled = FALSE;

			if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled)
				interval = 0;
		}
	}

	if (_glfw.wgl.EXT_swap_control)
		wglSwapIntervalEXT(interval);
}

static int extensionSupportedWGL(const char* extension)
{
	const char* extensions = NULL;

	if (_glfw.wgl.GetExtensionsStringARB)
		extensions = wglGetExtensionsStringARB(wglGetCurrentDC());
	else if (_glfw.wgl.GetExtensionsStringEXT)
		extensions = wglGetExtensionsStringEXT();

	if (!extensions)
		return GLFW_FALSE;

	return _glfwStringInExtensionString(extension, extensions);
}

static GLFWglproc getProcAddressWGL(const char* procname)
{
	const GLFWglproc proc = (GLFWglproc)wglGetProcAddress(procname);
	if (proc)
		return proc;

	return (GLFWglproc)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, procname);
}

static void destroyContextWGL(_GLFWwindow* window)
{
	if (window->context.wgl.handle)
	{
		wglDeleteContext(window->context.wgl.handle);
		window->context.wgl.handle = NULL;
	}
}

GLFWbool _glfwInitWGL(void)
{
	PIXELFORMATDESCRIPTOR pfd;
	HGLRC prc, rc;
	HDC pdc, dc;

	if (_glfw.wgl.instance)
		return GLFW_TRUE;

	_glfw.wgl.instance = _glfwPlatformLoadModule("opengl32.dll");
	if (!_glfw.wgl.instance)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR, "WGL: Failed to load opengl32.dll");
		return GLFW_FALSE;
	}

	_glfw.wgl.CreateContext = (PFN_wglCreateContext)
		_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglCreateContext");
	_glfw.wgl.DeleteContext = (PFN_wglDeleteContext)
		_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglDeleteContext");
	_glfw.wgl.GetProcAddress = (PFN_wglGetProcAddress)
		_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetProcAddress");
	_glfw.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)
		_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetCurrentDC");
	_glfw.wgl.GetCurrentContext = (PFN_wglGetCurrentContext)
		_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetCurrentContext");
	_glfw.wgl.MakeCurrent = (PFN_wglMakeCurrent)
		_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglMakeCurrent");
	_glfw.wgl.ShareLists = (PFN_wglShareLists)
		_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglShareLists");

	dc = GetDC(_glfw.win32.helperWindowHandle);

	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;

	if (!SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"WGL: Failed to set pixel format for dummy context");
		return GLFW_FALSE;
	}

	rc = wglCreateContext(dc);
	if (!rc)
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"WGL: Failed to create dummy context");
		return GLFW_FALSE;
	}

	pdc = wglGetCurrentDC();
	prc = wglGetCurrentContext();

	if (!wglMakeCurrent(dc, rc))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"WGL: Failed to make dummy context current");
		wglMakeCurrent(pdc, prc);
		wglDeleteContext(rc);
		return GLFW_FALSE;
	}

	_glfw.wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)
		wglGetProcAddress("wglGetExtensionsStringEXT");
	_glfw.wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
		wglGetProcAddress("wglGetExtensionsStringARB");
	_glfw.wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
		wglGetProcAddress("wglCreateContextAttribsARB");
	_glfw.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
		wglGetProcAddress("wglSwapIntervalEXT");
	_glfw.wgl.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
		wglGetProcAddress("wglGetPixelFormatAttribivARB");

	_glfw.wgl.ARB_multisample =
		extensionSupportedWGL("WGL_ARB_multisample");
	_glfw.wgl.ARB_framebuffer_sRGB =
		extensionSupportedWGL("WGL_ARB_framebuffer_sRGB");
	_glfw.wgl.EXT_framebuffer_sRGB =
		extensionSupportedWGL("WGL_EXT_framebuffer_sRGB");
	_glfw.wgl.ARB_create_context =
		extensionSupportedWGL("WGL_ARB_create_context");
	_glfw.wgl.ARB_create_context_profile =
		extensionSupportedWGL("WGL_ARB_create_context_profile");
	_glfw.wgl.EXT_create_context_es2_profile =
		extensionSupportedWGL("WGL_EXT_create_context_es2_profile");
	_glfw.wgl.ARB_create_context_robustness =
		extensionSupportedWGL("WGL_ARB_create_context_robustness");
	_glfw.wgl.ARB_create_context_no_error =
		extensionSupportedWGL("WGL_ARB_create_context_no_error");
	_glfw.wgl.EXT_swap_control =
		extensionSupportedWGL("WGL_EXT_swap_control");
	_glfw.wgl.EXT_colorspace =
		extensionSupportedWGL("WGL_EXT_colorspace");
	_glfw.wgl.ARB_pixel_format =
		extensionSupportedWGL("WGL_ARB_pixel_format");
	_glfw.wgl.ARB_context_flush_control =
		extensionSupportedWGL("WGL_ARB_context_flush_control");

	wglMakeCurrent(pdc, prc);
	wglDeleteContext(rc);
	return GLFW_TRUE;
}

void _glfwTerminateWGL(void)
{
	if (_glfw.wgl.instance)
		_glfwPlatformFreeModule(_glfw.wgl.instance);
}

#define SET_ATTRIB(a, v) \
{ \
assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
attribs[index++] = a; \
attribs[index++] = v; \
}

GLFWbool _glfwCreateContextWGL(_GLFWwindow* window,
	const _GLFWctxconfig* ctxconfig,
	const _GLFWfbconfig* fbconfig)
{
	int attribs[40];
	int pixelFormat;
	PIXELFORMATDESCRIPTOR pfd;
	HGLRC share = NULL;

	if (ctxconfig->share)
		share = ctxconfig->share->context.wgl.handle;

	window->context.wgl.dc = GetDC(window->win32.handle);
	if (!window->context.wgl.dc)
	{
		_glfwInputError(GLFW_PLATFORM_ERROR,
			"WGL: Failed to retrieve DC for window");
		return GLFW_FALSE;
	}

	pixelFormat = choosePixelFormatWGL(window, ctxconfig, fbconfig);
	if (!pixelFormat)
		return GLFW_FALSE;

	if (!DescribePixelFormat(window->context.wgl.dc,
		pixelFormat, sizeof(pfd), &pfd))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"WGL: Failed to retrieve PFD for selected pixel format");
		return GLFW_FALSE;
	}

	if (!SetPixelFormat(window->context.wgl.dc, pixelFormat, &pfd))
	{
		_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
			"WGL: Failed to set selected pixel format");
		return GLFW_FALSE;
	}

	if (ctxconfig->forward)
	{
		if (!_glfw.wgl.ARB_create_context)
		{
			_glfwInputError(GLFW_VERSION_UNAVAILABLE,
				"WGL: A forward compatible OpenGL context requested but WGL_ARB_create_context is unavailable");
			return GLFW_FALSE;
		}
	}

	if (ctxconfig->profile)
	{
		if (!_glfw.wgl.ARB_create_context_profile)
		{
			_glfwInputError(GLFW_VERSION_UNAVAILABLE,
				"WGL: OpenGL profile requested but WGL_ARB_create_context_profile is unavailable");
			return GLFW_FALSE;
		}
	}

	if (_glfw.wgl.ARB_create_context)
	{
		int index = 0, mask = 0, flags = 0;

		if (ctxconfig->forward)
			flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

		if (ctxconfig->profile == GLFW_OPENGL_CORE_PROFILE)
			mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		else if (ctxconfig->profile == GLFW_OPENGL_COMPAT_PROFILE)
			mask |= WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;


		if (ctxconfig->debug)
			flags |= WGL_CONTEXT_DEBUG_BIT_ARB;

		if (ctxconfig->robustness)
		{
			if (_glfw.wgl.ARB_create_context_robustness)
			{
				if (ctxconfig->robustness == GLFW_NO_RESET_NOTIFICATION)
				{
					SET_ATTRIB(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
						WGL_NO_RESET_NOTIFICATION_ARB);
				}
				else if (ctxconfig->robustness == GLFW_LOSE_CONTEXT_ON_RESET)
				{
					SET_ATTRIB(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
						WGL_LOSE_CONTEXT_ON_RESET_ARB);
				}

				flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
			}
		}

		if (ctxconfig->release)
		{
			if (_glfw.wgl.ARB_context_flush_control)
			{
				if (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_NONE)
				{
					SET_ATTRIB(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,
						WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
				}
				else if (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_FLUSH)
				{
					SET_ATTRIB(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,
						WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
				}
			}
		}

		if (ctxconfig->noerror)
		{
			if (_glfw.wgl.ARB_create_context_no_error)
				SET_ATTRIB(WGL_CONTEXT_OPENGL_NO_ERROR_ARB, GLFW_TRUE);
		}

		if (ctxconfig->major != 1 || ctxconfig->minor != 0)
		{
			SET_ATTRIB(WGL_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major);
			SET_ATTRIB(WGL_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor);
		}

		if (flags)
			SET_ATTRIB(WGL_CONTEXT_FLAGS_ARB, flags);

		if (mask)
			SET_ATTRIB(WGL_CONTEXT_PROFILE_MASK_ARB, mask);

		SET_ATTRIB(0, 0);

		window->context.wgl.handle =
			wglCreateContextAttribsARB(window->context.wgl.dc, share, attribs);
		if (!window->context.wgl.handle)
		{
			const DWORD error = GetLastError();

			if (error == (0xc0070000 | ERROR_INVALID_VERSION_ARB))
			{
				_glfwInputError(GLFW_VERSION_UNAVAILABLE,
					"WGL: Driver does not support OpenGL version %i.%i",
					ctxconfig->major,
					ctxconfig->minor);
			}
			else if (error == (0xc0070000 | ERROR_INVALID_PROFILE_ARB))
			{
				_glfwInputError(GLFW_VERSION_UNAVAILABLE,
					"WGL: Driver does not support the requested OpenGL profile");
			}
			else if (error == (0xc0070000 | ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB))
			{
				_glfwInputError(GLFW_INVALID_VALUE,
					"WGL: The share context is not compatible with the requested context");
			}
			else
			{
				_glfwInputError(GLFW_VERSION_UNAVAILABLE, "WGL: Failed to create OpenGL context");
			}

			return GLFW_FALSE;
		}
	}
	else
	{
		window->context.wgl.handle = wglCreateContext(window->context.wgl.dc);
		if (!window->context.wgl.handle)
		{
			_glfwInputErrorWin32(GLFW_VERSION_UNAVAILABLE,
				"WGL: Failed to create OpenGL context");
			return GLFW_FALSE;
		}

		if (share)
		{
			if (!wglShareLists(share, window->context.wgl.handle))
			{
				_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
					"WGL: Failed to enable sharing with specified OpenGL context");
				return GLFW_FALSE;
			}
		}
	}

	window->context.makeCurrent = makeContextCurrentWGL;
	window->context.swapBuffers = swapBuffersWGL;
	window->context.swapInterval = swapIntervalWGL;
	window->context.extensionSupported = extensionSupportedWGL;
	window->context.getProcAddress = getProcAddressWGL;
	window->context.destroy = destroyContextWGL;

	return GLFW_TRUE;
}