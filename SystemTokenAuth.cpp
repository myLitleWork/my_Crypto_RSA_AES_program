//---------------------------------------------------------------------------
#include "SystemTokenAuth.h"
#include "myCrypto.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace SystemTokenAuth
{
namespace
{
	const wchar_t* TOKEN_FILENAME = L"ShifRator3_system_token.dat";
	const char TOKEN_SEP = '\n';

	std::string WideToNarrow(const std::wstring& ws)
	{
		if (ws.empty()) return {};
		int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
		if (len <= 0) return {};
		std::string s(static_cast<size_t>(len), '\0');
		WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &s[0], len, nullptr, nullptr);
		return s;
	}

	// Детерминированный хеш (FNV-1a) для отпечатка системы
	uint64_t Fnv1aHash(const std::string& s)
	{
		uint64_t h = 14695981039346656037ULL; // FNV offset basis
		for (unsigned char c : s)
			h = (h ^ c) * 1099511628211ULL;
		return h;
	}

	std::string ToHex(uint64_t v)
	{
		std::ostringstream os;
		os << std::hex << std::setfill('0') << std::setw(16) << v;
		return os.str();
	}

	// Ключ 32 байта для AES из отпечатка системы
	std::string DeriveKeyFromFingerprint()
	{
		std::string fp = GetSystemFingerprint();
		while (fp.size() < 32) fp += fp;
		fp.resize(32);
		return fp;
	}

	std::string GetTokenKeyFilePath(const std::string& username)
	{
		std::string path = GetTokenFilePath();
		if (path.empty()) return "";
		size_t last = path.find_last_of("/\\");
		std::string folder = (last != std::string::npos) ? path.substr(0, last + 1) : path;
		return folder + username + "_prikey_token.dat";
	}
}

std::string GetSystemFingerprint()
{
	std::string raw;

	// Серийный номер тома C:
	DWORD volSerial = 0, maxLen = 0;
	wchar_t volName[MAX_PATH + 1] = {};
	if (GetVolumeInformationW(L"C:\\", volName, MAX_PATH, &volSerial, &maxLen, nullptr, nullptr, 0))
	{
		std::ostringstream os;
		os << volSerial;
		raw += os.str();
	}
	raw += "|";

	// Имя компьютера
	wchar_t compName[MAX_COMPUTERNAME_LENGTH + 1] = {};
	DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerNameW(compName, &size))
		raw += WideToNarrow(compName);
	raw += "|";

	// Имя пользователя (дополнительная стабильная привязка)
	wchar_t userBuf[256] = {};
	DWORD userLen = 256;
	if (GetUserNameW(userBuf, &userLen))
		raw += WideToNarrow(userBuf);

	uint64_t h = Fnv1aHash(raw);
	return ToHex(h);
}

std::string GetTokenFilePath()
{
	wchar_t buf[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, buf)))
	{
		char temp[MAX_PATH];
		if (GetTempPathA(MAX_PATH, temp))
			return std::string(temp) + "ShifRator3\\";
		return "";
	}
	std::wstring folder = std::wstring(buf) + L"\\ShifRator3\\";
	CreateDirectoryW(folder.c_str(), nullptr);
	return WideToNarrow(folder) + WideToNarrow(TOKEN_FILENAME);
}

std::pair<bool, std::string> TryAuthorizeBySystemToken()
{
	std::string path = GetTokenFilePath();
	if (path.empty()) return {false, ""};

	std::ifstream f(path, std::ios::in);
	if (!f.is_open()) return {false, ""};

	std::string storedFingerprint, username;
	if (!std::getline(f, storedFingerprint, TOKEN_SEP)) return {false, ""};
	if (!std::getline(f, username)) { /* username может быть пустым */ }

	f.close();

	// Убираем лишние символы
	storedFingerprint.erase(std::remove_if(storedFingerprint.begin(), storedFingerprint.end(),
		[](char c) { return c == '\r' || c == '\n'; }), storedFingerprint.end());
	username.erase(std::remove_if(username.begin(), username.end(),
		[](char c) { return c == '\r' || c == '\n'; }), username.end());

	std::string current = GetSystemFingerprint();
	if (storedFingerprint != current) return {false, ""};

	return {true, username};
}

bool SaveSystemToken(const std::string& username, const std::string& decryptedPrivateKey)
{
	std::string path = GetTokenFilePath();
	if (path.empty() || decryptedPrivateKey.empty()) return false;

	std::ofstream f(path, std::ios::out | std::ios::trunc);
	if (!f.is_open()) return false;

	f << GetSystemFingerprint() << TOKEN_SEP << username << "\n";
	f.close();

	// Перешифровываем RSA-ключ хешем токена
	std::string keyPath = GetTokenKeyFilePath(username);
	if (keyPath.empty()) return true;

	std::string tokenKey = DeriveKeyFromFingerprint();
	std::string encrypted = myCrypto::AESEncrypt(decryptedPrivateKey, tokenKey);
	if (encrypted.empty()) return true;

	std::ofstream kf(keyPath, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!kf.is_open()) return true;
	kf.write(encrypted.data(), encrypted.size());
	kf.close();

	return true;
}

std::string LoadPrivateKeyWithToken(const std::string& username)
{
	std::string keyPath = GetTokenKeyFilePath(username);
	if (keyPath.empty()) return "";

	std::ifstream kf(keyPath, std::ios::binary | std::ios::ate);
	if (!kf.is_open()) return "";

	std::streamsize size = kf.tellg();
	kf.seekg(0, std::ios::beg);
	std::string encrypted(static_cast<size_t>(size), '\0');
	if (!kf.read(&encrypted[0], size)) return "";
	kf.close();

	std::string tokenKey = DeriveKeyFromFingerprint();
	std::string decrypted = myCrypto::AESDecrypt(encrypted, tokenKey);
	return decrypted;
}

void ClearSystemToken()
{
	std::string path = GetTokenFilePath();
	if (path.empty()) return;

	// Читаем username из токена, чтобы удалить перешифрованный ключ
	std::string keyPathToDelete;
	{
		std::ifstream f(path, std::ios::in);
		if (f.is_open()) {
			std::string fp, username;
			if (std::getline(f, fp, TOKEN_SEP) && std::getline(f, username)) {
				username.erase(std::remove_if(username.begin(), username.end(),
					[](char c) { return c == '\r' || c == '\n'; }), username.end());
				keyPathToDelete = GetTokenKeyFilePath(username);
			}
			f.close();
		}
	}

	DeleteFileA(path.c_str());
	if (!keyPathToDelete.empty())
		DeleteFileA(keyPathToDelete.c_str());
}
}
