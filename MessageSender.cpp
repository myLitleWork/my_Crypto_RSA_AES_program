#include "MessageSender.h"
#include "MsgCrySer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

std::string getExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

// Метод для отправки зашифрованного сообщения
std::string MessageSender::sendEncryptedMessage(const std::string& from, const std::string& to, const Message& message) {
    // Поиск публичного ключа получателя
	std::string pub_key = loadPublicKey(to);

    // Если публичный ключ не найден, выводим сообщение об ошибке
	if (pub_key.empty()) {
		std::cerr << "Public key for recipient '" << to << "' not found." << std::endl;
		return " - Ключ шифрования не найден для абонента '" + to + "'";
	}

	// Шифрование сообщения с использованием публичного ключа
	Crypta crypta(message, pub_key, from);

	// Сохранение зашифрованного сообщения в файл
	auto save_crypta_dir = getFileName(to, default_dir);
	MessageCryptaSerializer::saveCryptaToFile(crypta, save_crypta_dir);

	return " - Сообщение успешно зашифровано для абонента '" + to + "' файл - " + save_crypta_dir;
}

// Метод для загрузки публичного ключа получателя
std::string MessageSender::loadPublicKey(const std::string& recipient) {
    std::string pub_key_filename = getExecutablePath() + "/rsa_keys/" + recipient + "_pubkey.pem";

    if (!std::filesystem::exists(pub_key_filename)) {
		return ""; // Возвращаем пустую строку, если ключ не найден
    }

	std::ifstream pub_key_file(pub_key_filename);
	std::stringstream pub_key_stream;
	pub_key_stream << pub_key_file.rdbuf();
	return pub_key_stream.str(); // Возвращаем содержимое публичного ключа
}

std::string MessageSender::getFileName(const std::string& recipient, const std::string& default_dir) {

	 if (!fs::exists(default_dir)) {
		fs::create_directory(default_dir);
	}
	std::string base_filename = recipient + ".cry";
	std::string filename = default_dir + "\\" + base_filename;
    int index = 1;
    // Если файл с таким именем уже существует, добавляем индекс
    while (std::filesystem::exists(filename)) {
		filename = default_dir + "\\" + recipient + "_" + std::to_string(index++) + ".cry";
	}
	// Сохраняем объект Crypta в файл
    return filename;
}

void MessageSender::setDefaultSaveDir(const std::string& _default_dir)
{
	default_dir = _default_dir;
}

