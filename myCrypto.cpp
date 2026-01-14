#include "myCrypto.h"

std::string myCrypto::loadKeyFromFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
	int key_len;
    if (!file.is_open()) {
		return {};  // Ошибка открытия файла}
    }

    file.seekg(0, std::ios::end);
	key_len = file.tellg();
    file.seekg(0, std::ios::beg);

	char* key = new char[key_len];
    file.read(key, key_len);
	file.close();

	auto key_str = std::string(key,key_len);

	return key_str;
}

std::string myCrypto::CryptoUniversal(const std::string& input_text, const unsigned char* char_key, int key_len, CryptoFunction cryptoFunc)
{
    const unsigned char* input_char = (unsigned char*)input_text.data();
    size_t input_len = input_text.length();

    int output_len = 0;
    char* output_char = cryptoFunc(input_char, input_len, char_key, key_len, &output_len);

    if (!output_char)
    {
        return {}; // Возвращаем пустую строку в случае ошибки
    }

    std::string output_text(output_char, output_len);
    dll::myDelete(output_char);  // Удаление зашифрованных данных

    return output_text;
}

std::vector<uint8_t> myCrypto::CryptoUniversal(const std::vector<uint8_t>& input_data, const std::string& key, CryptoFunction cryptoFunc)
{
    size_t input_len = input_data.size();
    int output_len = 0;

    char* output_char = cryptoFunc(input_data.data(), input_len, reinterpret_cast<const unsigned char*>(key.data()), key.length(), &output_len);

    if (!output_char)
    {
		return {}; // Возвращаем пустой вектор в случае ошибки
	}

	std::vector<uint8_t> output_data(output_char, output_char + output_len);
	dll::myDelete(output_char);  // Освобождение памяти

	return output_data;
}

std::string myCrypto::AESEncrypt(const std::string& input_text, const std::string key)
{
    const unsigned char* char_key = (unsigned char*)key.data();
    return CryptoUniversal(input_text, char_key, aes_key_length, dll::AESEncrypt);
}

std::string myCrypto::AESDecrypt(const std::string& input_text, const std::string key)
{
    const unsigned char* char_key = (unsigned char*)key.data();
    return CryptoUniversal(input_text, char_key, aes_key_length, dll::AESDecrypt);
}

std::string myCrypto::GenerateAESKey()
{
    unsigned char* char_key = dll::GenerateAESKey();
    std::string key(reinterpret_cast<const char*>(char_key), aes_key_length);
    dll::myDelete((char*)char_key);
    return key;
}

std::vector<uint8_t> myCrypto::RsaPubEncrypt(const std::vector<uint8_t>& input_data, const std::string& pub_key)
{
    return CryptoUniversal(input_data, pub_key, dll::RsaPubEncrypt);
}

std::vector<uint8_t> myCrypto::RsaPriDecrypt(const std::vector<uint8_t>& input_data, const std::string& pri_key)
{
    return CryptoUniversal(input_data, pri_key, dll::RsaPriDecrypt);
}

std::vector<uint8_t> myCrypto::RsaPriEncrypt(const std::vector<uint8_t>& input_data, const std::string& pub_key)
{
	return CryptoUniversal(input_data, pub_key, dll::RsaPriEncrypt);
}

std::vector<uint8_t> myCrypto::RsaPubDecrypt(const std::vector<uint8_t>& input_data, const std::string& pri_key)
{
    return CryptoUniversal(input_data, pri_key, dll::RsaPubDecrypt);
}

std::vector<uint8_t> myCrypto::AESEncrypt(const std::vector<uint8_t>& input_data, const std::string& key)
{
    return CryptoUniversal(input_data, key, dll::AESEncrypt);
}

std::vector<uint8_t> myCrypto::AESDecrypt(const std::vector<uint8_t>& input_data, const std::string& key)
{
    return CryptoUniversal(input_data, key, dll::AESDecrypt);
}

void  myCrypto::GenerateRSAKeys(std::string&  pri_key_out, std::string& pub_key_out)
{
	int pub_key_len;
	char* pub_key_char;

	int pri_key_len;
	char* pri_key_char;


	dll::GenerateRSAKeyChar(&pri_key_char, &pri_key_len, &pub_key_char, &pub_key_len);

	pri_key_out = std::string(pri_key_char, pri_key_len);
	pub_key_out = std::string(pub_key_char, pub_key_len);

	dll::myDelete(pri_key_char);
	dll::myDelete(pub_key_char);

}
