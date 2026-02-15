#ifndef KEYGENERATOR_H
#define KEYGENERATOR_H

#include <string>

std::string getExecutablePath();

class KeyGenerator {

public:
    // Метод для генерации RSA ключей и их сохранения
	void GenerateKeys(const std::string& username, const std::string& password);
    static std::string DecryptPrivateKey(const std::string& username, const std::string& password);

private:
    // Метод для сохранения ключа в файл
	void SaveKeyToFile(const std::string& filename, const std::string& key);
    static std::string AdjustPasswordToKeyLength(const std::string& password, size_t required_length);
};

#endif // KEYGENERATOR_H

