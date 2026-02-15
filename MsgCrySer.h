#ifndef MESSAGE_CRYPTA_SERIALIZER_H
#define MESSAGE_CRYPTA_SERIALIZER_H


#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <filesystem>
#include <fstream>
 #include <sstream>
#include "Message.h"

// Класс для сериализации/десериализации Message и Crypta
class MessageCryptaSerializer {
public:
    // Сериализация Message в байты
	static std::vector<BYTE> serializeMessage(const Message& message);
	static std::vector<BYTE> serializeMessage2(const Message& message);
    // Десериализация Message из байтов
    static Message deserializeMessage(const std::vector<BYTE>& data);

    // Сохранение Message в файл с расширением .msg
    static void saveMessageToFile(const Message& message, const std::string& filepath);

    // Загрузка Message из файла с расширением .msg
    static Message loadMessageFromFile(const std::string& filepath);

    // Сериализация Crypta в байты
    static std::vector<BYTE> serializeCrypta(const Crypta& crypta);

    // Десериализация Crypta из байтов
    static Crypta deserializeCrypta(const std::vector<BYTE>& data);

    // Сохранение Crypta в файл с расширением .cry
    static void saveCryptaToFile(const Crypta& crypta, const std::string& filepath);

    // Загрузка Crypta из файла с расширением .cry
    static Crypta loadCryptaFromFile(const std::string& filepath);
};

#endif // MESSAGE_CRYPTA_SERIALIZER_H

