//---------------------------------------------------------------------------

#pragma hdrstop

#include "CryptoArchive.h"
#include <filesystem>
#include <sstream>
//---------------------------------------------------------------------------
#pragma package(smart_init)

std::string getExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

CryptoArchiveCreator::CryptoArchiveCreator(const std::string& _recipient, std::atomic<bool>* stopFlag, std::atomic<int>* progress)
	: recipient(_recipient), stopRequested(stopFlag), progress(progress)
{
	rsaPublicKey = loadPublicKey(_recipient);
}

uint64_t CryptoArchiveCreator::getTotalFileSize(const std::vector<std::string>& filePaths) {
    uint64_t totalSize = 0;
    for (const auto& filePath : filePaths) {
		try {
			totalSize += std::filesystem::file_size(filePath);
		} catch (const std::filesystem::filesystem_error& e) {
			// Обработка ошибок, если файл недоступен или его размер не удается получить
			std::cerr << "Ошибка доступа к файлу " << filePath << ": " << e.what() << std::endl;
		}
    }
    return totalSize;
}

void CryptoArchiveCreator::updateProgress(uint64_t processed, uint64_t total) {
    if (progress != nullptr && total > 0) { // Проверка деления на ноль
        *progress = static_cast<int>((static_cast<double>(processed) / total) * 100);
    }
}

void CryptoArchiveCreator::generateAESKey() {
	aesKey = myCrypto::GenerateAESKey();
}

void CryptoArchiveCreator::encryptAESKey(std::vector<uint8_t>& encryptedKey) {
	std::vector<uint8_t> aesKeyVec(aesKey.begin(), aesKey.end());
	encryptedKey = myCrypto::RsaPubEncrypt(aesKeyVec, rsaPublicKey);
}

void CryptoArchiveCreator::serializeHeader(const Header& header, std::vector<uint8_t>& serializedHeader) {
    std::stringstream headerStream;
    {
        boost::archive::binary_oarchive oa(headerStream);
		oa << header;
    }
    std::string headerStr = headerStream.str();
    serializedHeader.assign(headerStr.begin(), headerStr.end());
}

std::string CryptoArchiveCreator::loadPublicKey(const std::string& recipient) {

	std::string pub_key_filename = getExecutablePath() + "/rsa_keys/" + recipient + "_pubkey.pem";

	if (!std::filesystem::exists(pub_key_filename)) {
		throw "Открытый RSA-ключ абонента '" + recipient + "' не найден!"; // Возвращаем пустую строку, если ключ не найден
	}

	std::ifstream pub_key_file(pub_key_filename);
	std::stringstream pub_key_stream;
	pub_key_stream << pub_key_file.rdbuf();
	return pub_key_stream.str(); // Возвращаем содержимое публичного ключа
}

// Основная функция для создания криптоархива
const std::string CryptoArchiveCreator::createArchive(const std::vector<std::string>& filePaths, const std::string& _textHeader) {

    if(sender.empty())
		throw "Не задан отправитель!";

	// Генерация AES ключа
	generateAESKey();

    // Шифрование AES ключа с использованием RSA
    std::vector<uint8_t> encryptedAESKey;
    encryptAESKey(encryptedAESKey);

	// Создание заголовка с информацией о файлах
	Header header;
	header.textHeader = _textHeader;  // Устанавливаем текст заголовка
	header.from = sender;            // Устанавливаем отправителя
	for (const auto& filePath : filePaths) {

		size_t fileSize;
		try {
			fileSize = std::filesystem::file_size(filePath);
		} catch (const std::filesystem::filesystem_error& e) {
			// Обработка ошибок, если файл недоступен или его размер не удается получить
			std::cerr << "Ошибка доступа к файлу " << filePath << ": " << e.what() << std::endl;
		}
		std::string fileName = fs::path(filePath).filename().string();
		header.fileInfo.push_back( {fileName, fileSize} );

	}

	// Сериализация заголовка
    std::vector<uint8_t> serializedHeader;
	serializeHeader(header, serializedHeader);

    // Шифрование заголовка с помощью AES
    std::vector<uint8_t> encryptedHeader = myCrypto::AESEncrypt(serializedHeader, aesKey);

    // Получение имени криптоархива на основе имени получателя
	std::string outputFilename = getArchiveFilename();

    // Запись зашифрованного архива
	writeEncryptedArchive(filePaths, outputFilename, encryptedAESKey, encryptedHeader);

    return outputFilename;
}

// Здесь вы реализуете вашу функцию для генерации имени криптоархива
std::string CryptoArchiveCreator::getArchiveFilename() {
	// Пример генерации имени на основе имени получателя и базовой директории

	 if (!fs::exists(baseOutputDir)) {
		fs::create_directory(baseOutputDir);
	}

	std::string base_filename = recipient + ".cry2";
	std::string filename = baseOutputDir + "\\" + base_filename;
	int index = 1;

	// Если файл с таким именем уже существует, добавляем индекс
    while (std::filesystem::exists(filename)) {
		filename = baseOutputDir + "\\" + recipient + "_" + std::to_string(index++) + ".cry2";
	}

	return filename;
}


void CryptoArchiveCreator::writeEncryptedArchive(const std::vector<std::string>& filePaths, const std::string& outputFilename, const std::vector<uint8_t>& encryptedAESKey, const std::vector<uint8_t>& encryptedHeader) {
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Ошибка открытия выходного файла для записи архива" << std::endl;
        return;
    }

	uint64_t totalSize = getTotalFileSize(filePaths);
    uint64_t processedSize = 0;

    size_t encryptedAESKeySize = encryptedAESKey.size();
    outputFile.write(reinterpret_cast<const char*>(&encryptedAESKeySize), sizeof(encryptedAESKeySize));
    outputFile.write(reinterpret_cast<const char*>(encryptedAESKey.data()), encryptedAESKeySize);

    size_t encryptedHeaderSize = encryptedHeader.size();
    outputFile.write(reinterpret_cast<const char*>(&encryptedHeaderSize), sizeof(encryptedHeaderSize));
    outputFile.write(reinterpret_cast<const char*>(encryptedHeader.data()), encryptedHeaderSize);

    const size_t bufferSize = 1024 * 1024 * 10;
	std::vector<uint8_t> buffer(bufferSize);

    for (const auto& filePath : filePaths) {
        std::ifstream inputFile(filePath, std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "Ошибка открытия файла: " << filePath << std::endl;
            continue;
        }

        while (!inputFile.eof()) {
            if (stopRequested && stopRequested->load()) {  // Проверяем флаг остановки
                std::cerr << "Процесс шифрования был остановлен." << std::endl;
                inputFile.close();
                outputFile.close();
                std::remove(outputFilename.c_str());  // Удаляем созданный файл архива
                return;  // Корректный выход без выброса исключения
            }


			inputFile.read(reinterpret_cast<char*>(buffer.data()), bufferSize);
            std::streamsize bytesRead = inputFile.gcount();

			if (bytesRead > 0) {

				std::vector<uint8_t> data(buffer.begin(), buffer.begin() + bytesRead);
                std::vector<uint8_t> encryptedData = myCrypto::AESEncrypt(data, aesKey);

                size_t encryptedDataSize = encryptedData.size();
                outputFile.write(reinterpret_cast<const char*>(&encryptedDataSize), sizeof(encryptedDataSize));
                outputFile.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedDataSize);

                processedSize += bytesRead;
                updateProgress(processedSize, totalSize);  // Обновляем прогресс
            }
        }
        inputFile.close();
    }

    outputFile.close();
}


