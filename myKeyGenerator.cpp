#include "myKeyGenerator.h"
#include "myCrypto.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>

std::string getExecutablePath() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

namespace fs = std::filesystem;

void KeyGenerator::GenerateKeys(const std::string& username, const std::string& password) {

	std::string pri_key_str, pub_key_str;

	myCrypto::GenerateRSAKeys(pri_key_str, pub_key_str);

	// Шифрование закрытого ключа с помощью AES
	std::string adjusted_password = AdjustPasswordToKeyLength(password, 32);

	std::string encrypted_pri_key = myCrypto::AESEncrypt(pri_key_str, adjusted_password);


    // Создание директории, если её не существует
	if (!fs::exists(getExecutablePath() + "/rsa_keys")) {
		fs::create_directory(getExecutablePath() + "/rsa_keys");
	}

	// Сохранение открытого ключа на диск
	SaveKeyToFile(username + "_pubkey.pem", pub_key_str);

    // Сохранение зашифрованного закрытого ключа на диск
    SaveKeyToFile(username + "_prikey_encrypted.pem", encrypted_pri_key);

}


void KeyGenerator::SaveKeyToFile(const std::string& filename, const std::string& key) {
	std::ofstream file(getExecutablePath() + "/rsa_keys/" + filename, std::ios::binary);
    if (file.is_open()) {
        file.write(key.c_str(), key.size());
        file.close();
    } else {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
}

std::string KeyGenerator::DecryptPrivateKey(const std::string& username, const std::string& password) {


    // Имя файла с зашифрованным закрытым ключом
    std::string encrypted_pri_key_file = getExecutablePath()+ "/rsa_keys/" + username + "_prikey_encrypted.pem";

	// Чтение зашифрованного закрытого ключа из файла
	auto encrypted_pri_key = myCrypto::loadKeyFromFile(encrypted_pri_key_file);

	if (encrypted_pri_key.empty()) {
		throw std::runtime_error("Failed to load encrypted private key.");
	}

	// Расшифровка закрытого ключа с использованием AES и пароля
	std::string decrypted_pri_key;
	std::string adjusted_password = AdjustPasswordToKeyLength(password, 32);

	decrypted_pri_key = myCrypto::AESDecrypt(encrypted_pri_key, adjusted_password);

	if (decrypted_pri_key.empty()) {
		throw std::runtime_error("Failed to decrypt private key. Check the password.");
    }

    return decrypted_pri_key;
}

std::string KeyGenerator::AdjustPasswordToKeyLength(const std::string& password, size_t required_length) {
    std::string adjusted_password = password;

    // Если пароль слишком короткий, дополняем его символами до нужной длины
    if (adjusted_password.length() < required_length) {
        adjusted_password.append(required_length - adjusted_password.length(), '0'); // Дополняем нулями
    }
    // Если пароль слишком длинный, обрезаем его
    else if (adjusted_password.length() > required_length) {
        adjusted_password = adjusted_password.substr(0, required_length);
    }

    return adjusted_password;
}

