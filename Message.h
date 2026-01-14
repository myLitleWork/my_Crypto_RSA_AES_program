#ifndef MESSAGE_H
#define MESSAGE_H

#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#define BYTE unsigned char

class Crypta;

#include <map>
#include <string>
#include <vector>
#include <filesystem>

struct Message {
	std::string text;
	std::map<std::string, std::vector<BYTE>> attachments;

	// Удаляем конструктор копирования и оператор присваивания
   // Message(const Message&) = delete;
   // Message& operator=(const Message&) = delete;

	// Добавляем перемещающий конструктор и оператор перемещения
	Message(Message&&) = default;
	Message& operator=(Message&&) = default;

	// Другие функции
	void addAttachment(const std::filesystem::path& _file_dir);
	void saveToDirectory(const std::filesystem::path& dirPath) const;
	size_t getAttachCount() const { return attachments.size(); }

	Message(const Crypta& _crypta, const std::string& pri_key);
	Message() = default;

	static Message loadFromDirectory(const std::filesystem::path& dirPath);

	// Метод для бинарной сериализации
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & text;
		ar & attachments;
	}
};


#include <vector>
#include <string>

struct Crypta {
    std::vector<BYTE> aes_rsa_key_;
    std::vector<BYTE> data_;
    std::string from;

    // Удаляем конструктор копирования и оператор присваивания
  //  Crypta(const Crypta&) = delete;
  //  Crypta& operator=(const Crypta&) = delete;

    // Добавляем перемещающий конструктор и оператор перемещения (без noexcept, как и в Message)
    Crypta(Crypta&&) = default;
	Crypta& operator=(Crypta&&) = default;

    // Конструкторы
    Crypta() = default;
	Crypta(const Message& _message, const std::string& pub_key, const std::string& from);

    std::vector<BYTE> getRsaAeskey() const { return aes_rsa_key_; }
    std::vector<BYTE> getData() const { return data_; }

    // Метод для бинарной сериализации
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & aes_rsa_key_;
        ar & data_;
        ar & from;
    }
};


#endif // MESSAGE_H

