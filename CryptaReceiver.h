#ifndef CRYPTARECEIVER_H
#define CRYPTARECEIVER_H

#include <string>
#include "Message.h"

class CryptaReceiver {
public:
    // Метод для приёма и расшифровки зашифрованного сообщения
	std::string receiveEncryptedMessage(const std::string& to, const std::string& password, const std::string& cryptaFilePath);
	void setDefaultSaveDir(const std::string& default_dir);

	static std::string loadPrivateKey(const std::string& recipient, const std::string& password);
    static std::string getUniqueDirectoryName(const std::string& from, const std::string& default_dir);

private:

	std::string default_dir = "Прием";
	// Метод для загрузки закрытого ключа получателя


    // Метод для десериализации Crypta из файла
    Crypta deserializeCryptaFromFile(const std::string& filePath);

    // Метод для расшифровки и сохранения сообщения
	std::string saveDecryptedMessageToDirectory(const Crypta& crypta, const std::string& privateKey);
};

#endif // CRYPTARECEIVER_H

