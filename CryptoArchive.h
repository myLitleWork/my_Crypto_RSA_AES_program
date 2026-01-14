#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include "myCrypto.h"
//#include "myBoostSerPair.h"

namespace fs = std::filesystem;

class CryptoArchiveCreator {

public:

	static inline std::string baseOutputDir = "Отправка"; // Базовая директория для сохранения архива
	static inline std::string sender = "";        // Отправитель

	// Конструктор принимает отправителя
	CryptoArchiveCreator() = default;
	CryptoArchiveCreator(const std::string& recipient, std::atomic<bool>* stopFlag, std::atomic<int>* progress);

	const std::string createArchive(const std::vector<std::string>& filePaths, const std::string& textHeader);

private:

	std::string recipient;
	std::string rsaPublicKey;
	std::string aesKey;
    std::atomic<bool>* stopRequested;  // Флаг остановки процесса
	std::atomic<int>* progress;        // Прогресс выполнения

	struct Header {

		std::string textHeader;
		std::string from;
		std::vector<std::pair<std::string, size_t>> fileInfo;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & textHeader;
			ar & from;
			ar & fileInfo;
		}

	};

	std::map<std::string, std::string> fileMap;

	std::string loadPublicKey(const std::string& recipient);

    // Функция для получения имени криптоархива
	std::string getArchiveFilename();

	uint64_t getTotalFileSize(const std::vector<std::string>& filePaths);
	void updateProgress(uint64_t processed, uint64_t total);
	void generateAESKey();
	void encryptAESKey(std::vector<uint8_t>& encryptedKey);
	void serializeHeader(const Header& header, std::vector<uint8_t>& serializedHeader);
	void writeEncryptedArchive(const std::vector<std::string>& filePaths, const std::string& outputFilename, const std::vector<uint8_t>& encryptedAESKey, const std::vector<uint8_t>& encryptedHeader);
};

