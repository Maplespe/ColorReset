// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "MinHook.h"
#include "..\ColorReset\easyhook.h"
#include <Uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

HMODULE g_hModule;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    g_hModule = hModule;
    return TRUE;
}

extern DWORD WINAPI MGetSysColor(int nIndex);
typedef DWORD(WINAPI* O_GetSysColor)(int);
O_GetSysColor F_GetSysColor = 0;

extern HRESULT WINAPI MGetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor);
typedef HRESULT(WINAPI* O_GetThemeColor)(HTHEME, int, int, int, COLORREF*);
O_GetThemeColor F_GetThemeColor = 0;

//全局主题句柄
HTHEME g_hTheme = 0;

//EasyHook EntryPoint
extern "C" __declspec(dllexport) BOOL CALLBACK NativeInjectionEntryPoint(REMOTE_ENTRY_INFO * pInfo)
{
    if (MH_Initialize() == MH_OK)
    {
        MH_CreateHook(&GetSysColor, &MGetSysColor, (LPVOID*)&F_GetSysColor);
        MH_CreateHook(&GetThemeColor, &MGetThemeColor, (LPVOID*)&F_GetThemeColor);

        if (MH_EnableHook(&GetSysColor) == MH_OK
            && MH_EnableHook(&GetThemeColor) == MH_OK)
        {
            
            MessageBeep(MB_OK);
            //MessageBoxW(0, L"内鬼成功安插a.a(你的颜色会变成棕色)", L"2333", MB_ICONINFORMATION);
        }
        else
            MessageBoxW(0, L"函数加载失败..", L"Fuck", MB_ICONERROR);
    }
    else
        MessageBoxW(0, L"初始化反汇编引擎失败!", L"Fuck", MB_ICONERROR);

    return true;
}

DWORD WINAPI MGetSysColor(int nIndex)
{
    DWORD Color = 0;
    //将所有索引值的颜色 都设置为指定颜色(以下颜色值是系统默认主题颜色)
    if (nIndex == COLOR_SCROLLBAR)
        Color = RGB(200, 200, 200);
    else if (nIndex == COLOR_DESKTOP || nIndex == COLOR_BACKGROUND)
        Color = RGB(0, 0, 0);
    else if (nIndex == COLOR_ACTIVECAPTION)
        Color = RGB(153, 180, 209);
    else if (nIndex == COLOR_INACTIVECAPTION)
        Color = RGB(191, 205, 219);
    else if (nIndex == COLOR_MENU)
        Color = RGB(240, 240, 240);
    else if (nIndex == COLOR_WINDOW)
        Color = RGB(255, 255, 255);
    else if (nIndex == COLOR_WINDOWFRAME)
        Color = RGB(100, 100, 100);
    else if (nIndex == COLOR_MENUTEXT)
        Color = RGB(0, 0, 0);
    else if (nIndex == COLOR_WINDOWTEXT)
        Color = RGB(0, 0, 0);
    else if (nIndex == COLOR_CAPTIONTEXT)
        Color = RGB(0, 0, 0);
    else if (nIndex == COLOR_ACTIVEBORDER)
        Color = RGB(180, 180, 180);
    else if (nIndex == COLOR_INACTIVEBORDER)
        Color = RGB(244, 247, 252);
    else if (nIndex == COLOR_APPWORKSPACE)
        Color = RGB(171, 171, 171);
    else if (nIndex == COLOR_HIGHLIGHT)
        Color = RGB(0, 120, 215);
    else if (nIndex == COLOR_HIGHLIGHTTEXT)
        Color = RGB(255, 255, 255);
    else if (nIndex == COLOR_3DFACE)
        Color = RGB(240, 240, 240);
    else if (nIndex == COLOR_3DSHADOW || nIndex == COLOR_BTNSHADOW)
        Color = RGB(160, 160, 160);
    else if (nIndex == COLOR_GRAYTEXT)
        Color = RGB(109, 109, 109);
    else if (nIndex == COLOR_BTNTEXT)
        Color = RGB(0, 0, 0);
    else if (nIndex == COLOR_INACTIVECAPTIONTEXT)
        Color = RGB(0, 0, 0);
    else if (nIndex == COLOR_BTNHIGHLIGHT || nIndex == COLOR_BTNHILIGHT
        || nIndex == COLOR_3DHILIGHT || nIndex == COLOR_3DHIGHLIGHT)
        Color = RGB(255, 255, 255);
    else if (nIndex == COLOR_3DDKSHADOW)
        Color = RGB(105, 105, 105);
    else if (nIndex == COLOR_3DLIGHT)
        Color = RGB(227, 227, 227);
    else if (nIndex == COLOR_INFOTEXT)
        Color = RGB(0, 0, 0);
    else if (nIndex == COLOR_INFOBK)
        Color = RGB(255, 255, 225);
    else if (nIndex == COLOR_HOTLIGHT)
        Color = RGB(0, 102, 204);
    else if (nIndex == COLOR_GRADIENTACTIVECAPTION)
        Color = RGB(185, 209, 234);
    else if (nIndex == COLOR_GRADIENTINACTIVECAPTION)
        Color = RGB(215, 228, 242);
    else if (nIndex == COLOR_MENUHILIGHT)
        Color = RGB(0, 120, 215);
    else if (nIndex == COLOR_MENUBAR)
        Color = RGB(240, 240, 240);
    else
        Color = 0;

    return Color;
}

HRESULT __stdcall MGetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor)
{
    /*
    if(pColor)
        *pColor = RGB(87, 80, 70);*/

    //主题颜色太多了
    //暂时还没需求... 需要的自己判断添加

    return F_GetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
}
