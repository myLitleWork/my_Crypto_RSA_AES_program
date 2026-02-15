#ifndef CryptoArchiveReaderH
#define CryptoArchiveReaderH
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <atomic>  // Добавлено для использования атомарных переменных
#include "myCrypto.h"

namespace fs = std::filesystem;

class CryptoArchiveReader {

public:
    static inline std::string baseDecryptionDir = "";  // Базовая директория для расшифрования

    // Обновленный конструктор принимает закрытый ключ для расшифрования и атомарные переменные
	CryptoArchiveReader(const std::string& _recipient, const std::string& _password, std::atomic<bool>* stopFlag, std::atomic<int>* progress);

    // Метод для расшифрования архива
    std::string decryptArchive(const std::string& encryptedFilename);

private:

    std::string recipient;
    std::string rsaPrivateKey;
    std::atomic<bool>* stopRequested;  // Флаг остановки процесса
	std::atomic<int>* progress;       // Прогресс выполнения

    struct Header {
        std::string textHeader;  // Текст заголовка
        std::string from;        // Отправитель
		std::vector<std::pair<std::string, size_t>> fileInfo;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & textHeader;
            ar & from;
            ar & fileInfo;
        }
    };

    // Метод для расшифрования AES-ключа
    void decryptAESKey(std::ifstream& inputFile, std::string& aesKey);

    // Метод загрузки и расшифрования ключа
    std::string loadPrivateKey(const std::string& password);

    // Метод для десериализации заголовка
    void deserializeHeader(std::ifstream& inputFile, const std::string& aesKey, Header& header);

    // Метод для создания уникальной директории для расшифрованных данных
    std::string createDecryptionDirectory(const std::string& sender, const std::string& baseDecryptionDir);

    // Метод для обновления прогресса
    void updateProgress(size_t processed, size_t total);
};

#endif

