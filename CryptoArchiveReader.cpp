#include "CryptoArchiveReader.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include "myCrypto.h"
#include "myKeyGenerator.h"

namespace fs = std::filesystem;

CryptoArchiveReader::CryptoArchiveReader(const std::string& _recipient, const std::string& _password, std::atomic<bool>* stopFlag, std::atomic<int>* progress)
    : stopRequested(stopFlag), progress(progress) {
    if (baseDecryptionDir.empty())
        throw "Не иницилизирована базовая директория сохранения!";

	recipient = _recipient;
    rsaPrivateKey = loadPrivateKey(_password);
}

void CryptoArchiveReader::decryptAESKey(std::ifstream& inputFile, std::string& aesKey) {
    size_t encryptedAESKeySize;
    inputFile.read(reinterpret_cast<char*>(&encryptedAESKeySize), sizeof(encryptedAESKeySize));

    std::vector<uint8_t> encryptedAESKey(encryptedAESKeySize);
    inputFile.read(reinterpret_cast<char*>(encryptedAESKey.data()), encryptedAESKeySize);

    std::vector<uint8_t> aesKeyVec = myCrypto::RsaPriDecrypt(encryptedAESKey, rsaPrivateKey);
    aesKey = std::string(aesKeyVec.begin(), aesKeyVec.end());
}

void CryptoArchiveReader::deserializeHeader(std::ifstream& inputFile, const std::string& aesKey, Header& header) {
    size_t encryptedHeaderSize;
    inputFile.read(reinterpret_cast<char*>(&encryptedHeaderSize), sizeof(encryptedHeaderSize));

    std::vector<uint8_t> encryptedHeader(encryptedHeaderSize);
    inputFile.read(reinterpret_cast<char*>(encryptedHeader.data()), encryptedHeaderSize);

    std::vector<uint8_t> serializedHeader = myCrypto::AESDecrypt(encryptedHeader, aesKey);

    std::stringstream headerStream(std::string(serializedHeader.begin(), serializedHeader.end()));
    {
        boost::archive::binary_iarchive ia(headerStream);
        ia >> header;
    }
}

std::string CryptoArchiveReader::createDecryptionDirectory(const std::string& sender, const std::string& baseDecryptionDir) {

	std::string base_directory = baseDecryptionDir + "\\" + sender;
	std::string directory = base_directory;
	int index = 1;

	// Если директория с таким именем уже существует, добавляем индекс
	while (fs::exists(directory)) {
		directory = base_directory + "_" + std::to_string(index++);
	}

	fs::create_directory(directory);
	return directory;
}

void CryptoArchiveReader::updateProgress(size_t processed, size_t total) {
	if (total > 0) {
		progress->store(static_cast<int>((static_cast<double>(processed) / total) * 100));
	}
}
std::string CryptoArchiveReader::loadPrivateKey(const std::string& password) {

	std::string pri_key_filename = getExecutablePath() + "/rsa_keys/" + recipient + "_prikey_encrypted.pem";

	if (!std::filesystem::exists(pri_key_filename)) {
		throw "Закрытый RSA-ключ оператора '" + recipient + "'  - не найден!";
	}

	// Загрузка и расшифровка закрытого ключа с использованием AES и пароля

	std::ifstream pri_key_file(pri_key_filename, std::ios::binary);
	std::stringstream pri_key_stream;
	pri_key_stream << pri_key_file.rdbuf();

	std::string encrypted_pri_key = pri_key_stream.str();

	std::string decrypted_pri_key = KeyGenerator::DecryptPrivateKey(recipient, password);

	return decrypted_pri_key;
}




std::string CryptoArchiveReader::decryptArchive(const std::string& encryptedFilename) {

    std::ifstream inputFile(encryptedFilename, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Ошибка открытия зашифрованного файла" << std::endl;
        return "Ошибка открытия зашифрованного файла";
    }

    // Расшифровываем AES-ключ
    std::string aesKey;
    decryptAESKey(inputFile, aesKey);

    // Десериализуем заголовок
    Header header;
    deserializeHeader(inputFile, aesKey, header);

    // Создаем директорию для расшифрованных данных на основе отправителя
    std::string decryptionDir = createDecryptionDirectory(header.from, baseDecryptionDir);

    // Рассчитываем общий размер для прогресса
    size_t totalSize = 0;
    for (const auto& [fileName, fileSize] : header.fileInfo) {
        totalSize += fileSize;
	}

	size_t processedSize = 0;

    // Проходим по каждому файлу в зашифрованном архиве
	for (const auto& [fileName, fileSize] : header.fileInfo) {



        // Открываем выходной файл
        std::ofstream outputFile(decryptionDir + "\\" + fileName, std::ios::binary);
        if (!outputFile.is_open()) {
            std::cerr << "Ошибка создания выходного файла: " << fileName << std::endl;
            continue;
        }

        size_t bytesToRead = fileSize;

		while (bytesToRead > 0) {

			size_t encryptedDataSize;
			inputFile.read(reinterpret_cast<char*>(&encryptedDataSize), sizeof(encryptedDataSize));

			std::vector<uint8_t> encryptedData(encryptedDataSize);
            inputFile.read(reinterpret_cast<char*>(encryptedData.data()), encryptedDataSize);

            std::vector<uint8_t> decryptedData = myCrypto::AESDecrypt(encryptedData, aesKey);
			outputFile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());

            // Обновление прогресса
			processedSize += decryptedData.size();
			updateProgress(processedSize, totalSize);

            // Проверка, чтобы `bytesToRead` не стало отрицательным
			if (decryptedData.size() > bytesToRead) {
				break;
			}

            bytesToRead -= decryptedData.size();

            // Проверка на остановку во время чтения
			if (stopRequested->load()) {
				std::cerr << "Процесс расшифрования был остановлен." << std::endl;
				outputFile.close();
                // Удаление неполной директории при остановке
				std::filesystem::remove_all(decryptionDir);
				return "Процесс расшифрования был остановлен.";
            }
        }

        outputFile.close();
    }

    inputFile.close();

	// Сохранение текста заголовка в файл "header_text.txt"
	if(header.textHeader.empty())
		return " - " + decryptionDir + " - успешное расшифрование";

	std::ofstream textFile(decryptionDir + "\\header_text.txt");
    if (!textFile.is_open()) {
        std::cerr << "Ошибка создания файла header_text.txt для сохранения заголовка." << std::endl;
    } else {
		textFile << header.textHeader;
		textFile.close();
    }

	return " - " + decryptionDir + " - успешное расшифрование, заголовок сообщения: \"" + header.textHeader + "\"";
}


