//---------------------------------------------------------------------------

#ifndef StreamWriterH
#define StreamWriterH
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <vcl.h>
// Структура заголовка, содержащая имена файлов и их размеры
struct Header {
    std::map<std::string, size_t> fileInfo;
    // Функция для сериализации структуры
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & fileInfo;
    }
};

void writeFilesWithHeader(const std::map<std::string, std::string>& files, const std::string& outputFilename);
void readFilesWithHeader(const std::string& inputFilename, const std::string& outputDirectory);
std::map<std::string, std::string> CreateFileMap(const std::vector<String>& files);
//---------------------------------------------------------------------------
#endif
