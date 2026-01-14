#include "CryptaReceiver.h"
#include "MsgCrySer.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iostream>
#include "myCrypto.h"
#include "myKeyGenerator.h"

std::string getExecutablePath() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

// Метод для приёма зашифрованного сообщения
std::string CryptaReceiver::receiveEncryptedMessage(const std::string& recipient, const std::string& password, const std::string& cryptaFilePath) {
    // Загрузка закрытого ключа получателя
	std::string privateKey = loadPrivateKey(recipient, password);

	if (privateKey.empty()) {
		return " - '" + cryptaFilePath + "' - ключ абонента " + recipient + " не найден или испорчен!";
	}

    // Десериализация объекта Crypta из файла
    Crypta crypta = deserializeCryptaFromFile(cryptaFilePath);

	// Расшифровка и сохранение сообщения
	return saveDecryptedMessageToDirectory(crypta, privateKey);
}

void CryptaReceiver::setDefaultSaveDir(const std::string& _default_dir)
{
	default_dir = _default_dir;
}

// Метод для загрузки закрытого ключа получателя
std::string CryptaReceiver::loadPrivateKey(const std::string& recipient, const std::string& password) {

	std::string pri_key_filename = getExecutablePath() + "/rsa_keys/" + recipient + "_prikey_encrypted.pem";

	if (!std::filesystem::exists(pri_key_filename)) {
		return {}; // Возвращаем пустую строку, если ключ не найден
	}

	// Загрузка и расшифровка закрытого ключа с использованием AES и пароля

    std::ifstream pri_key_file(pri_key_filename, std::ios::binary);
    std::stringstream pri_key_stream;
	pri_key_stream << pri_key_file.rdbuf();

	std::string encrypted_pri_key = pri_key_stream.str();

    KeyGenerator generator;
	std::string decrypted_pri_key = generator.DecryptPrivateKey(recipient, password);

	return decrypted_pri_key;
}

// Метод для десериализации объекта Crypta из файла
Crypta CryptaReceiver::deserializeCryptaFromFile(const std::string& filePath) {
    return MessageCryptaSerializer::loadCryptaFromFile(filePath);
}

// Метод для расшифровки и сохранения сообщения
std::string CryptaReceiver::saveDecryptedMessageToDirectory(const Crypta& crypta, const std::string& privateKey) {
    // Используем конструктор Message для расшифровки объекта Crypta с помощью закрытого ключа
    Message message(crypta, privateKey);

    // Получаем уникальное имя директории
	std::string directory = getUniqueDirectoryName(crypta.from, default_dir);

    // Создание директории
    std::filesystem::create_directory(directory);

    // Сохранение текста и вложений
    message.saveToDirectory(directory);

	return "Криптограмма от абонента '" + crypta.from + "' расшифрована, содержимое в " + directory;
}


std::string CryptaReceiver::getUniqueDirectoryName(const std::string& from, const std::string& default_dir) {
    if (!fs::exists(default_dir)) {
        fs::create_directory(default_dir);
    }

	std::string base_directory = default_dir + "\\" + from;
    std::string directory = base_directory;
    int index = 1;

    // Если директория с таким именем уже существует, добавляем индекс
    while (std::filesystem::exists(directory)) {
        directory = base_directory + "_" + std::to_string(index++);
    }

    return directory;
}


