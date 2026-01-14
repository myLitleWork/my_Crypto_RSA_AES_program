#include "Message.h"

#include "MsgCrySer.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <fstream>
#include <sstream>
#include <vector>

// Сериализация Message в байты
std::vector<BYTE> MessageCryptaSerializer::serializeMessage(const Message& message) {
	std::ostringstream oss(std::ios::binary);  // Создаём поток для записи
	boost::archive::binary_oarchive oa(oss);   // Используем стандартный бинарный архив

	oa << message;  // Сериализуем объект Message в архив

	std::string str = oss.str();
	std::vector<BYTE> buffer;

	// Перемещаем символы из строки в вектор с использованием индекса
	buffer.reserve(str.size());  // Резервируем память заранее для повышения эффективности
	for (size_t i = 0; i < str.size(); ++i) {
		buffer.push_back(static_cast<BYTE>(str[i]));  // Добавляем символ в вектор
	}
	return buffer;
}

// Десериализация Message из байтов

Message MessageCryptaSerializer::deserializeMessage(const std::vector<BYTE>& data) {
    // Используем array_source для чтения данных непосредственно из вектора
    boost::iostreams::array_source source(reinterpret_cast<const char*>(data.data()), data.size());
    boost::iostreams::stream<boost::iostreams::array_source> iss(source);
    boost::archive::binary_iarchive ia(iss);  // Используем стандартный бинарный архив для десериализации
    Message message;
    ia >> message;  // Десериализуем объект Message
    return message;
}
// Сохранение Message в файл
void MessageCryptaSerializer::saveMessageToFile(const Message& message, const std::string& filepath) {
    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    boost::archive::binary_oarchive oa(ofs);  // Стандартный бинарный архив
    oa << message;  // Сериализация в файл
}

// Загрузка Message из файла
Message MessageCryptaSerializer::loadMessageFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    boost::archive::binary_iarchive ia(ifs);  // Стандартный бинарный архив
    Message message;
    ia >> message;  // Десериализация из файла
    return message;
}

// Сериализация Crypta в байты
std::vector<BYTE> MessageCryptaSerializer::serializeCrypta(const Crypta& crypta) {
    std::ostringstream oss(std::ios::binary);  // Создаём поток для записи
    boost::archive::binary_oarchive oa(oss);   // Используем стандартный бинарный архив

    oa << crypta;  // Сериализуем объект Crypta

	std::string str = oss.str();
	std::vector<BYTE> buffer;

	// Перемещаем символы из строки в вектор с использованием индекса
	buffer.reserve(str.size());  // Резервируем память заранее для повышения эффективности
	for (size_t i = 0; i < str.size(); ++i) {
		buffer.push_back(static_cast<BYTE>(str[i]));  // Добавляем символ в вектор
	}
	return buffer;
}

// Десериализация Crypta из байтов
Crypta MessageCryptaSerializer::deserializeCrypta(const std::vector<BYTE>& data) {
    // Используем array_source для чтения данных непосредственно из вектора
    boost::iostreams::array_source source(reinterpret_cast<const char*>(data.data()), data.size());
    boost::iostreams::stream<boost::iostreams::array_source> iss(source);
    boost::archive::binary_iarchive ia(iss);  // Используем стандартный бинарный архив для десериализации
    Crypta crypta;
    ia >> crypta;  // Десериализация объекта Crypta
    return crypta;
}

// Сохранение Crypta в файл
void MessageCryptaSerializer::saveCryptaToFile(const Crypta& crypta, const std::string& filepath) {
    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    boost::archive::binary_oarchive oa(ofs);  // Стандартный бинарный архив
    oa << crypta;  // Сериализация объекта Crypta в файл
}

// Загрузка Crypta из файла
Crypta MessageCryptaSerializer::loadCryptaFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    boost::archive::binary_iarchive ia(ifs);  // Стандартный бинарный архив

    Crypta crypta;
    ia >> crypta;  // Десериализация объекта Crypta из файла

    return crypta;
}

