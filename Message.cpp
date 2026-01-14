#include "Message.h"
#include "MsgCrySer.h"
#include "myCrypto.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>



void Message::addAttachment(const std::filesystem::path& _file_dir)
{
    std::ifstream file(_file_dir, std::ios::binary);
    if (!file)
        throw std::runtime_error("Cannot open file: " + _file_dir.string());

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), {});
    attachments[_file_dir.filename().string()] = data;
}

void Message::saveToDirectory(const std::filesystem::path& dirPath) const
{

    std::filesystem::create_directory(dirPath);

	for (const auto& [filename, data] : attachments) {
		std::ofstream file(dirPath / filename, std::ios::binary);
		if (!file)
			throw std::runtime_error("Failed to open attachment file for writing: " + filename);

		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		file.close();
	}

	if(text.empty())
		return;

	std::ofstream textFile(dirPath / "text.txt");
	if (!textFile)
		throw std::runtime_error("Failed to open text file for writing.");

	textFile << text;
	textFile.close();


}



Message Message::loadFromDirectory(const std::filesystem::path& dirPath)
{
    Message msg;
    std::filesystem::path textFilePath = dirPath / "text.txt";

    // Попытка открыть файл текста
    std::ifstream textFile(textFilePath);
    if (textFile.is_open()) {
        std::string line, content;
        while (getline(textFile, line)) {
            content += line + "\n";
        }
        msg.text = content;
        textFile.close();
	}

    // Чтение вложений
    for (auto& entry : std::filesystem::directory_iterator(dirPath)) {
		if (std::filesystem::is_regular_file(entry.status()) && entry.path() != textFilePath) {
            std::ifstream file(entry.path(), std::ios::binary);
            if (!file) {
				throw std::runtime_error("Failed to open attachment file: " + entry.path().string());
            }
			std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), {});
            msg.attachments[entry.path().filename().string()] = data;
        }
    }

    return msg;
}



Message::Message(const Crypta& _crypta, const std::string& pri_key)
{
	auto aes_key_bytes = myCrypto::RsaPriDecrypt(_crypta.getRsaAeskey(), pri_key);
    std::string aes_key_str(aes_key_bytes.begin(), aes_key_bytes.end());
    // Десериализация из данных Crypta
	*this = MessageCryptaSerializer::deserializeMessage(myCrypto::AESDecrypt(_crypta.getData(), aes_key_str));
}
Crypta::Crypta(const Message& _message, const std::string& pub_key, const std::string& _from)
{
	from = _from;
	auto aes_key = myCrypto::GenerateAESKey();
	auto nn = aes_key.size();
	std::vector<BYTE> aes_key_bytes(aes_key.begin(), aes_key.end());
	aes_rsa_key_ = myCrypto::RsaPubEncrypt(aes_key_bytes, pub_key);
	int dd = aes_rsa_key_.size();
	// Сериализация данных Message в байты

	data_ = myCrypto::AESEncrypt(MessageCryptaSerializer::serializeMessage(_message), aes_key);
}
