//---------------------------------------------------------------------------
#ifndef SystemTokenAuthH
#define SystemTokenAuthH
//---------------------------------------------------------------------------
#include <string>
#include <utility>

namespace SystemTokenAuth
{
	// Возвращает отпечаток текущей системы (привязка к ПК)
	std::string GetSystemFingerprint();

	// Путь к файлу токена в AppData (narrow string для работы с fstream)
	std::string GetTokenFilePath();

	// Проверяет наличие валидного токена для этой системы.
	// Возвращает (true, username) если токен есть и привязан к системе, иначе (false, "").
	std::pair<bool, std::string> TryAuthorizeBySystemToken();

	// Сохраняет системный токен и перешифровывает RSA-ключ хешем токена.
	// decryptedPrivateKey — расшифрованный PEM закрытого ключа (получен паролем).
	// Ключ сохраняется зашифрованным хешем отпечатка системы — далее расшифровка только по токену.
	bool SaveSystemToken(const std::string& username, const std::string& decryptedPrivateKey);

	// Загружает и расшифровывает закрытый ключ по токену (привязан к этому ПК).
	// Возвращает расшифрованный PEM или пустую строку при ошибке.
	std::string LoadPrivateKeyWithToken(const std::string& username);

	// Удаляет сохранённый системный токен и связанный с ним перешифрованный ключ
	void ClearSystemToken();
}
//---------------------------------------------------------------------------
#endif
