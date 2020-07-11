// ColorReset.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "ColorReset.h"
#include "easyhook.h"
#include <shellapi.h>
#include <Shlwapi.h>
#include <string>

#if _M_X64
#pragma comment(lib,"EasyHook64.lib")
#else
#pragma comment(lib,"EasyHook32.lib")
#endif

#pragma comment(lib, "shlwapi.lib")

#pragma region API

//文件是否存在
bool FileIsExist(std::wstring FilePath) {
	WIN32_FIND_DATA  FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(FilePath.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return true;
	}
	return false;
}

//字符串子文本替换
std::wstring replace_all(std::wstring str, std::wstring old_value, std::wstring new_value)
{
	while (true) {
		std::wstring::size_type   pos(0);
		if ((pos = str.find(old_value)) != std::wstring::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return str;
}

//取当前目录
std::wstring GetCurExeDir() {
	wchar_t sPath[MAX_PATH];
	GetModuleFileNameW(0, sPath, MAX_PATH);
	std::wstring path = sPath;
	path = path.substr(0, path.rfind(L"\\"));

	return path;
}

//读取注册表文本型
std::wstring GetRegSZ(HKEY hKey, LPCWSTR SubKey, LPCWSTR KeyName, bool is64 = false) {

	HKEY tkey;

	REGSAM sam = KEY_READ;
	if (is64)
		sam = KEY_READ | KEY_WOW64_64KEY;

	if (ERROR_SUCCESS == RegOpenKeyExW(hKey, SubKey, 0, sam, &tkey))
	{
		wchar_t dwValue[1024];
		DWORD dwSzType = REG_SZ;
		DWORD dwSize = sizeof(dwValue);
		if (RegQueryValueExW(tkey, KeyName, 0, &dwSzType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS)
		{
			return L"Query Error";
		}
		return dwValue;
	}
	RegCloseKey(tkey);
	return L"Open Error";
}

//取扩展程序路径
std::wstring GetExtensionExe(std::wstring filePath)
{
	std::wstring _Path = GetRegSZ(HKEY_CLASSES_ROOT, PathFindExtensionW(filePath.c_str()), L"");
	if (_Path != L"" && _Path != L"Open Error" && _Path != L"Query Error")
	{
		_Path = GetRegSZ(HKEY_CLASSES_ROOT, (_Path + L"\\shell\\open\\command").c_str(), L"");
		if (_Path == L"" || _Path == L"Open Error" || _Path == L"Query Error")
		{
			_Path = GetRegSZ(HKEY_CLASSES_ROOT, (_Path + L"\\shell\\edit\\command").c_str(), L"");
		}
	}
	if (_Path != L"" && _Path != L"Open Error" && _Path != L"Query Error")
	{
		LPWSTR* szArgList;
		int argCount;

		szArgList = CommandLineToArgvW(_Path.c_str(), &argCount);
		if (szArgList != NULL)
		{
			_Path = szArgList[0];
			LocalFree(szArgList);
			return _Path;
		}
	}
	return L"";
}

#pragma endregion

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR   lpCmdLine,
                     _In_ int       nCmdShow)
{

	LPWSTR* szArgList;
	int argCount;

	szArgList = CommandLineToArgvW(lpCmdLine, &argCount);
	if (szArgList == NULL)
	{
		return 10;
	}

	//取自身路径
	wchar_t sPath[MAX_PATH];
	GetModuleFileNameW(0, sPath, MAX_PATH);
	std::wstring curFileName = sPath;

	for (int i = 0; i < argCount; i++)
	{
		std::wstring curcmd = szArgList[i];
		//确保不是自己
		if (curcmd != curFileName)
		{
			//如果不是exe 那就找关联程序
			std::wstring cmd_ = L"";

			std::wstring extstr = PathFindExtensionW(curcmd.c_str());
			if (replace_all(extstr, L".EXE", L".exe") != L".exe") {
				//将原参数保存
				cmd_ = curcmd;
				//取此扩展名所关联的应用程序
				curcmd = GetExtensionExe(curcmd);
			}

			//确定文件存在且是exe
			if (FileIsExist(curcmd) && replace_all(curcmd, L".EXE", L".exe").rfind(L".exe") != -1)
			{
				//初始化结构体
				STARTUPINFO stStartUpInfo;
				memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
				stStartUpInfo.cb = sizeof(stStartUpInfo);

				PROCESS_INFORMATION stProcessInfo;
				memset(&stProcessInfo, 0, sizeof(stProcessInfo));

				wchar_t* _cmd_ = NULL;

				//拷贝源参数
				if (cmd_ != L"") {
					cmd_ = L" \"" + cmd_ + L"\"";
					//拷贝字符串 CreateProcessW 的参数不可为const
					_cmd_ = new wchar_t[cmd_.length() + 1];
					ZeroMemory(_cmd_, cmd_.length());
					//从源字符串复制数据到内存
					memcpy(_cmd_, cmd_.data(), cmd_.length() * sizeof(wchar_t));
					//添加字符串结尾
					_cmd_[cmd_.length()] = '\0';
				}

				//创建进程
				bool ret = CreateProcessW(curcmd.c_str(), _cmd_, NULL, NULL, false, CREATE_NEW_CONSOLE, NULL, NULL, &stStartUpInfo, &stProcessInfo);

				//释放
				if (_cmd_ != NULL)
					delete[] _cmd_;

				if (ret)
				{
					//注入扩展
#if _M_X64
					std::wstring dllPath = GetCurExeDir() + L"\\ColorResetLib.dll";
					NTSTATUS ntStatus = RhInjectLibrary(stProcessInfo.dwProcessId, 0, EASYHOOK_INJECT_DEFAULT, NULL, (WCHAR*)dllPath.c_str(),  NULL, 0);
#else
					std::wstring dllPath = GetCurExeDir() + L"\\ColorResetLib32.dll";
					NTSTATUS ntStatus = RhInjectLibrary(stProcessInfo.dwProcessId, 0, EASYHOOK_INJECT_DEFAULT, (WCHAR*)dllPath.c_str(), NULL, NULL, 0);
#endif
					if (ntStatus != 0)
					{
						std::wstring error = RtlGetLastErrorString();
						MessageBoxW(0, (L"对进程加载ColorResetLib.dll失败!\n错误信息:\n" + error).c_str(), L"失败", MB_ICONERROR);
					}
				}
				else
					MessageBoxW(0, L"CreateProcess Failed!", L"Error", MB_ICONERROR);
			}
		}
	}

	LocalFree(szArgList);
    return (int) 0;
}
