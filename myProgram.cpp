//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <fstream>
//---------------------------------------------------------------------------
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>
USEFORM("mainForm.cpp", formMain);
//---------------------------------------------------------------------------

namespace
{

std::wstring GetAppDataFolder()
{
	wchar_t buf[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, buf))) {
		std::wstring folder = std::wstring(buf) + L"\\ShifRator3\\";
		CreateDirectoryW(folder.c_str(), nullptr);
		return folder;
	}
	wchar_t temp[MAX_PATH];
	GetTempPathW(MAX_PATH, temp);
	return std::wstring(temp);
}

std::wstring GetLockFilePath()
{
	return GetAppDataFolder() + L"ShifRator3.lock";
}

std::wstring GetRequestFilePath()
{
	return GetAppDataFolder() + L"ShifRator3_request.txt";
}

bool IsCry2FileArg(const UnicodeString& value)
{
	if (value.IsEmpty())
		return false;

	UnicodeString ext = LowerCase(ExtractFileExt(value));
	return ext == ".cry2" && FileExists(value);
}

HANDLE TryAcquireLock()
{
	std::wstring lockPath = GetLockFilePath();
	HANDLE hFile = CreateFileW(
		lockPath.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (hFile == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	OVERLAPPED ov = {};
	BOOL locked = LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &ov);
	if (!locked) {
		CloseHandle(hFile);
		return INVALID_HANDLE_VALUE;
	}

	return hFile;
}

void WriteRequestFile(const UnicodeString& cryptPath)
{
	std::wstring reqPath = GetRequestFilePath();
	HANDLE hFile = CreateFileW(
		reqPath.c_str(),
		GENERIC_WRITE,
		0,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	std::string data = AnsiString(cryptPath).c_str();
	DWORD written = 0;
	WriteFile(hFile, data.c_str(), static_cast<DWORD>(data.size()), &written, nullptr);
	CloseHandle(hFile);
}

}

int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	HANDLE lockFile = TryAcquireLock();
	const bool anotherInstanceRunning = (lockFile == INVALID_HANDLE_VALUE);

	const bool hasCryptArg = (ParamCount() >= 1 && IsCry2FileArg(ParamStr(1)));

	if (anotherInstanceRunning) {
		if (hasCryptArg)
			WriteRequestFile(ParamStr(1));
		return 0;
	}

	if (hasCryptArg)
		WriteRequestFile(ParamStr(1));

	try
	{
		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->CreateForm(__classid(TformMain), &formMain);
		TStyleManager::TrySetStyle("Aqua Light Slate");
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}

	if (lockFile != INVALID_HANDLE_VALUE) {
		CloseHandle(lockFile);
		DeleteFileW(GetLockFilePath().c_str());
	}

	return 0;
}
//---------------------------------------------------------------------------
