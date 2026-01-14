#ifndef MESSAGESENDER_H
#define MESSAGESENDER_H

#include <string>
#include "Message.h"

class MessageSender {

public:

	void setDefaultSaveDir(const std::string& default_dir);
    // Метод для отправки зашифрованного сообщения
	std::string sendEncryptedMessage(const std::string& from, const std::string& to, const Message& message);
	// Метод для загрузки публичного ключа получателя
	static std::string loadPublicKey(const std::string& recipient);
	static std::string getFileName(const std::string& recipient, const std::string& default_dir);

private:

	std::string default_dir = "Отправка";

};

#endif // MESSAGESENDER_H

