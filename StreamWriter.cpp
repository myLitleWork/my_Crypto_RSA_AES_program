//---------------------------------------------------------------------------

#pragma hdrstop
#include <vcl.h>
#include <map>
#include <string>
#include <sstream>
#include "StreamWriter.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
// Функция для записи заголовка и файлов в один файл-криптограмму
void writeFilesWithHeader(const std::map<std::string, std::string>& files, const std::string& outputFilename) {
    // Шаг 1: Создаем заголовок
    Header header;
    for (const auto& [filename, filepath] : files) {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Ошибка открытия файла: " << filepath << std::endl;
            continue;
        }

        // Записываем в заголовок имя файла и его размер
        header.fileInfo[filename] = static_cast<size_t>(file.tellg());
        file.close();
    }

    // Открываем файл для записи заголовка и данных
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Ошибка открытия выходного файла" << std::endl;
        return;
    }

    // Шаг 2: Сериализуем заголовок в байты с помощью Boost.Serialization
    std::stringstream headerStream;
    {
        boost::archive::binary_oarchive oa(headerStream);
        oa << header;
    }
    std::string serializedHeader = headerStream.str();

    // Шаг 3: Записываем размер заголовка в файл
    size_t headerSize = serializedHeader.size();
    outputFile.write(reinterpret_cast<const char*>(&headerSize), sizeof(headerSize));

    // Шаг 4: Записываем сам сериализованный заголовок
    outputFile.write(serializedHeader.c_str(), headerSize);

    // Шаг 5: Записываем файлы в поток
    for (const auto& [filename, filepath] : files) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Ошибка открытия файла для записи: " << filepath << std::endl;
            continue;
        }

        // Копируем содержимое файла в выходной файл
        outputFile << file.rdbuf();
        file.close();
	}

	outputFile.close();
}

// Функция для чтения заголовка и восстановления файлов
void readFilesWithHeader(const std::string& inputFilename, const std::string& outputDirectory) {
	std::ifstream inputFile(inputFilename, std::ios::binary);
	if (!inputFile.is_open()) {
		std::cerr << "Ошибка открытия входного файла" << std::endl;
		return;
	}

	// Шаг 1: Читаем размер заголовка
	size_t headerSize;
	inputFile.read(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));

    // Шаг 2: Читаем сериализованный заголовок
    std::vector<char> headerBuffer(headerSize);
    inputFile.read(headerBuffer.data(), headerSize);

    // Шаг 3: Десериализуем заголовок с помощью Boost.Serialization
    Header header;
	std::stringstream headerStream(std::string(headerBuffer.begin(), headerBuffer.end()));
    {
		boost::archive::binary_iarchive ia(headerStream);
        ia >> header;
	}

    // Шаг 4: Читаем и восстанавливаем файлы на основе заголовка
	for (const auto& [filename, filesize] : header.fileInfo) {
        std::ofstream outputFile(outputDirectory + "/" + filename, std::ios::binary);
        if (!outputFile.is_open()) {
			std::cerr << "Ошибка создания выходного файла: " << filename << std::endl;
            continue;
		}

		// Читаем файл из потока
        std::vector<char> fileBuffer(filesize);
        inputFile.read(fileBuffer.data(), filesize);
		outputFile.write(fileBuffer.data(), filesize);

		outputFile.close();
    }

	inputFile.close();
}


std::map<std::string, std::string> CreateFileMap(const std::vector<String>& files) {
    std::map<std::string, std::string> fileMap;
	for (const auto& filePath : files) {
        // Извлекаем имя файла с помощью ExtractFileName
        String fileName = ExtractFileName(filePath);
        // Добавляем запись в std::map: имя файла -> полный путь
        fileMap[std::string(AnsiString(fileName).c_str())] = std::string(AnsiString(filePath).c_str());
    }
    return fileMap;
}