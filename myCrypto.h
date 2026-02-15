#pragma once
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace dll
{

	#define DLL_EXPORT(S) extern "C" __declspec(dllexport) S __stdcall

	DLL_EXPORT(char*) RsaPriDecrypt(const unsigned char* cipher_text, size_t cipher_text_len, const unsigned char* pri_key, int pri_key_len, int* decrypted_len);
	DLL_EXPORT(char*) RsaPubEncrypt(const unsigned char* clear_text, size_t clear_text_len, const unsigned char* pub_key, int pub_key_len, int* encrypted_len);

	DLL_EXPORT(char*) RsaPriEncrypt(const unsigned char* clear_text, size_t clear_text_len, const unsigned char* pri_key, int pri_key_len, int* encrypted_len);
	DLL_EXPORT(char*) RsaPubDecrypt(const unsigned char* cipher_text, size_t cipher_text_len, const unsigned char* pub_key, int pub_key_len, int* decrypted_len);

	DLL_EXPORT(void) GenerateRSAKey();
	DLL_EXPORT(void) GenerateRSAKeyChar(char** pri_key_out, int* pri_len_out, char** pub_key_out, int* pub_len_out);

	DLL_EXPORT(void) myDelete(char* ptr);

    DLL_EXPORT(unsigned char*) __stdcall GenerateAESKey();
	DLL_EXPORT(char*) __stdcall AESEncrypt(const unsigned char* clear_text, size_t clear_text_len, const unsigned char* key, int key_len, int* encrypted_len);
	DLL_EXPORT(char*) __stdcall AESDecrypt(const unsigned char* cipher_text, size_t cipher_text_len, const unsigned char* key, int key_len, int* decrypted_len);

}

class myCrypto
{
	// Константы для длины ключей

    static inline const size_t aes_key_length = 32;
	static inline const size_t key_length = 4096;

	// Тип функции для криптографии
	using CryptoFunction = std::function<char*(const unsigned char*, size_t, const unsigned char*, int, int*)>;

	// Универсальная криптографическая функция для строк
	static std::string CryptoUniversal(const std::string& input_text, const unsigned char* char_key, int key_len, CryptoFunction cryptoFunc);

	// Универсальная криптографическая функция для вектора байт
	static std::vector<uint8_t> CryptoUniversal(const std::vector<uint8_t>& input_data, const std::string& key, CryptoFunction cryptoFunc);

	myCrypto() = delete;

public:

	// Загрузка ключа из файла
	static std::string loadKeyFromFile(const std::string& filename);

    // Шифрование/расшифровка AES
	static std::string AESEncrypt(const std::string& input_text, const std::string key);
    static std::string AESDecrypt(const std::string& input_text, const std::string key);

    // Генерация ключа AES
	static std::string GenerateAESKey();
	static void GenerateRSAKeys(std::string&  pri_key_out, std::string& pub_key_out);

    // RSA и AES шифрование/расшифровка для вектора данных
	static std::vector<uint8_t> RsaPubEncrypt(const std::vector<uint8_t>& input_data, const std::string& pub_key);
	static std::vector<uint8_t> RsaPriDecrypt(const std::vector<uint8_t>& input_data, const std::string& pri_key);
	static std::vector<uint8_t> RsaPriEncrypt(const std::vector<uint8_t>& input_data, const std::string& pub_key);
	static std::vector<uint8_t> RsaPubDecrypt(const std::vector<uint8_t>& input_data, const std::string& pri_key);

	static std::vector<uint8_t> AESEncrypt(const std::vector<uint8_t>& input_data, const std::string& key);
    static std::vector<uint8_t> AESDecrypt(const std::vector<uint8_t>& input_data, const std::string& key);
};
