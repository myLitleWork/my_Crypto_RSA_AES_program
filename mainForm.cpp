#include <vcl.h>
#pragma hdrstop
#include <iostream>
#include <iomanip>
#include "mainForm.h"
#include "myCrypto.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <chrono>
#include "Message.h"
//#include "flashNum.h"
#include "myKeyGenerator.h"
#include <functional>
#include <windows.h>
#include "FileCtrl.hpp"
#include "StreamWriter.h"
#include "CryptoArchiveReader.h"
#include <cctype>
#include <regex>

#include <atomic>
#include <thread>
 #define strStd(s) String{s.c_str()}
//----------------Глобальные атомарные переменные-------------------------
std::atomic<int> progress(0);      // Прогресс выполнения
std::atomic<bool> stopFlag(false); // Флаг остановки процесса
//------------------------------------------------------------------------
#define stdStr(s) std::string{AnsiString{s}.c_str()}
#define strStd(s) String{s.c_str()}
//------------------------------------------------------------------------

std::string extractNameBeforeUnderscore(const std::string& fileName) {
    size_t pos = fileName.find('_');
    if (pos != std::string::npos) {
        return fileName.substr(0, pos);  // Извлекаем часть строки до символа "_"
    }
    return fileName;  // Если "_" не найден, возвращаем полное имя файла
}

//-------------------------------------------------------------------------

std::string getExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

//-------------------------------------------------------------------------

std::pair<std::vector<std::string>, std::vector<std::string>> scanRSAKeysFolder(const std::string& folderPath) {
    std::vector<std::string> publicKeys;
    std::vector<std::string> privateKeys;
    // Сканируем папку
	for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
		if (std::filesystem::is_regular_file(entry)) {
            std::string filename = entry.path().filename().string();
            // Проверяем, является ли файл открытым или закрытым ключом
            if (filename.find("pubkey.pem") != std::string::npos) {
                publicKeys.push_back(filename);
            } else if (filename.find("prikey_encrypted.pem") != std::string::npos) {
                privateKeys.push_back(filename);
            }
        }
    }
    return {publicKeys, privateKeys}; // Возвращаем пару векторов
}

//-------------------------------------------------------------------------

std::string vectorByteToString(const std::vector<BYTE>& data) {
    return std::string(data.begin(), data.end());
}

//-------------------------------------------------------------------------

// Функция преобразования std::string в std::vector<BYTE>
std::vector<BYTE> stringToVectorByte(const std::string& str) {
    return std::vector<BYTE>(str.begin(), str.end());
}

//-------------------------------------------------------------------------

void writeVectorToFile(const std::vector<BYTE>& data, const std::string& filename) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
        std::cout << "Data has been written to file: " << filename << std::endl;
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

//-------------------------------------------------------------------------

std::vector<BYTE> readVectorFromFile(const std::string& filename) {

	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<BYTE> data(size);
        if (file.read(reinterpret_cast<char*>(data.data()), size)) {
            std::cout << "Data has been read from file: " << filename << std::endl;
            return data;
        } else {
            std::cerr << "Error reading data from file: " << filename << std::endl;
        }
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
	}

	return std::vector<BYTE>(); // Return empty vector if failed to read

}

//-------------------------------------------------------------------------

char* loadKeyFromFile(const std::string& filename, size_t& key_len)
{
		std::ifstream file(filename, std::ios::binary);

		if (!file.is_open()) {
		   //	std::cerr << "Failed to open file: " << filename << std::endl;
			return nullptr;
		}

		file.seekg(0, std::ios::end);
		key_len = file.tellg();
		file.seekg(0, std::ios::beg);
		char* key = new char[key_len];
		file.read(key, key_len);
		file.close();

		return key;
}

//-------------------------------------------------------------------------
// Сохранение данных в файл
void saveToFile(const char* data, size_t data_len, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    file.write(data, data_len);
    file.close();
}


std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename); // Открываем файл для чтения
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }

    std::string content; // Создаем строку для хранения содержимого файла
    std::string line;
    while (std::getline(file, line)) { // Читаем файл построчно
        content += line; // Добавляем строку к содержимому
    }

    file.close(); // Закрываем файл

    return content;
}


//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;

//---------------------------------------------------------------------------

__fastcall TformMain::TformMain(TComponent* Owner)
	: TForm(Owner)
{
}

//---------------------------------------------------------------------------

void __fastcall TformMain::btnAddCryptsClick(TObject *Sender)
{
	TFileOpenDialog *dlg = new TFileOpenDialog(this);
	dlg->DefaultFolder = GetCurrentDir();
	lstCrypts->Items->Clear();
	// Разрешаем множественный выбор файлов
	dlg->Options << fdoAllowMultiSelect;
	// Устанавливаем фильтр файлов с расширением .cry
	dlg->FileTypes->Add()->DisplayName = "Crypta Files (*.cry2)";
	dlg->FileTypes->Items[0]->FileMask = "*.cry2";
	// Показываем диалоговое окно
	if (dlg->Execute()) {
		// Очищаем ListBox перед добавлением новых файлов
		// Проходим по каждому выбранному файлу и добавляем его в ListBox
		for (int i = 0; i < dlg->Files->Count; i++) {
			lstCrypts->Items->Add(dlg->Files->Strings[i]);
		}
	}
	// Удаляем диалог
	delete dlg;
}

//---------------------------------------------------------------------------

void __fastcall TformMain::btnAddAtachmentsClick(TObject *Sender)
{
	TFileOpenDialog *dlg = new TFileOpenDialog(this);

	lstAttach->Items->Clear();
	// Разрешаем множественный выбор файлов
	dlg->Options << fdoAllowMultiSelect;
	// Показываем диалоговое окно
	if (dlg->Execute()) {
		// Очищаем ListBox перед добавлением новых файлов

		// Проходим по каждому выбранному файлу и добавляем его в ListBox
		for (int i = 0; i < dlg->Files->Count; i++) {
			lstAttach->Items->Add(dlg->Files->Strings[i]);
		}
	}
	// Удаляем диалог
	delete dlg;

	UpdateLabelWithFileInfo();

}

//---------------------------------------------------------------------------

void __fastcall TformMain::btnEncryptClick(TObject *Sender)
{
	LockFormExceptStopButton(this);
	if (!isProcessing) {
		// Режим запуска процесса
		if (edtDirCrypts->Text.IsEmpty()) {
			String Dir = edtDirCrypts->Text;
			SelectDirectory("Выберите папку для сохранения криптограмм", "пк", Dir, TSelectDirExtOpts(), 0);
			edtDirCrypts->Text = Dir;
		}

		if (edtDirCrypts->Text.IsEmpty())
			return;

		CryptoArchiveCreator::baseOutputDir = stdStr(edtDirCrypts->Text);
		CryptoArchiveCreator::sender = stdStr(EditUserName->Text);

		std::vector<std::string> filePaths;
		for (int i = 0; i < lstAttach->Count; i++)
			filePaths.push_back(stdStr(lstAttach->Items->Strings[i]));

		memStatus->Lines->Clear();

		// Сбрасываем прогресс и флаг остановки
		progress.store(0);
		stopFlag.store(false);

		try {
			// Меняем текст кнопки на "Остановить"
			btnEncrypt->Caption = "Остановить";
			isProcessing = true;  // Устанавливаем флаг, что процесс запущен

			// Запускаем процесс в отдельном потоке
			std::thread encryptThread([=]() {
				try {
					// Проходим по элементам CheckListBox и шифруем сообщение для каждого выбранного получателя
					for (int i = 0; i < chkListAbonents->Count; ++i) {
						if (!chkListAbonents->Checked[i])
							continue;

						std::string recipient = stdStr(chkListAbonents->Items->Strings[i]);
						std::string textHeader = stdStr(memText->Text);

						TThread::Synchronize(nullptr, [=]() {
							Gauge1->Progress = 0;
							memStatus->Lines->Add(" - Зашифрование сообщения для абонента '" + chkListAbonents->Items->Strings[i] + "' ...");
						});



						// Создаем архив
						auto result_str = mapCryptoArchiveCreators.at(recipient).createArchive(filePaths, textHeader);

                        // Проверка на остановку
						if (stopFlag.load()) {
							TThread::Synchronize(nullptr, [=]() {
								memStatus->Lines->Add("Процесс шифрования был остановлен.");
								UnlockForm(this);
							});
							return;  // Прерываем выполнение
						}

						// Обновляем статус
						TThread::Synchronize(nullptr, [=]() {
							memStatus->Lines->Add(" - Успешно - " + strStd(result_str));
							progress.store(0);
							Gauge1->Progress = 100;

						});
					}

					// По завершению шифрования меняем текст кнопки обратно на "Зашифровать"
					TThread::Synchronize(nullptr, [=]() {
						btnEncrypt->Caption = "Зашифровать";
						isProcessing = false;  // Сбрасываем флаг
						Timer1->Enabled = false;  // Останавливаем таймер

					});

				} catch (const std::exception& e) {
					TThread::Synchronize(nullptr, [=]() {
						MessageDlg(strStd(std::string{e.what()}), mtError, TMsgDlgButtons() << mbOK, 0);
						btnEncrypt->Caption = "Зашифровать";  // Возвращаем текст кнопки
						isProcessing = false;  // Сбрасываем флаг
						Timer1->Enabled = false;  // Останавливаем таймер
						UnlockForm(this);
					});
				}
				UnlockForm(this);
			});

			encryptThread.detach();  // Отсоединяем поток, чтобы он работал параллельно
			Timer1->Enabled = true;  // Включаем таймер для обновления прогресса

		} catch (const std::exception &e) {
			MessageDlg(strStd(std::string{e.what()}), mtError, TMsgDlgButtons() << mbOK, 0);
			UnlockForm(this);
		}
	} else {
		// Режим остановки процесса
		try {
			stopFlag.store(true);  // Устанавливаем флаг остановки
			TThread::Synchronize(nullptr, [=]() {
				memStatus->Lines->Add("Остановка процесса шифрования...");
				btnEncrypt->Caption = "Зашифровать";
				isProcessing = false;
				Timer1->Enabled = false;
				UnlockForm(this);
			});
		} catch (const std::exception& e) {
			MessageDlg("Ошибка при остановке процесса: " + strStd(std::string{e.what()}), mtError, TMsgDlgButtons() << mbOK, 0);
            UnlockForm(this);
		}
	}
}


//---------------------------------------------------------------------------

void __fastcall TformMain::btnDecryptClick(TObject *Sender)
{
	if(!lstCrypts->Count)
        return;

	LockFormExceptStopButton(this);
	if (!isProcessing) {
        // Режим запуска процесса
        if (edtSaveMessageDir->Text.IsEmpty()) {
            String Dir = "";
            Dir = edtSaveMessageDir->Text;
            SelectDirectory("Выберите папку для сохранения сообщений", "пк", Dir, TSelectDirExtOpts(), 0);
            edtSaveMessageDir->Text = Dir;
        }

        if (edtSaveMessageDir->Text.IsEmpty())
			return;



        CryptoArchiveReader::baseDecryptionDir = stdStr(edtSaveMessageDir->Text);

        std::string username = stdStr(EditUserName->Text);
        std::string password = stdStr(EditPassword->Text);

        memStatus->Lines->Clear();

        // Сбрасываем прогресс и флаг остановки
        progress.store(0);
        stopFlag.store(false);

        // Меняем текст кнопки на "Остановить"
        btnDecrypt->Caption = "Остановить";
        isProcessing = true;  // Устанавливаем флаг, что процесс запущен

        // Запускаем процесс в отдельном потоке
		std::thread decryptThread([=]() {
            try {
                CryptoArchiveReader cryptoArchiveReader{username, password, &stopFlag, &progress};

                // Проходим по элементам списка криптограмм и расшифровываем каждую
				for (int i = 0; i < lstCrypts->Count; i++) {
                    if (stopFlag.load()) {
                        TThread::Synchronize(nullptr, [=]() {
							memStatus->Lines->Add("Процесс расшифрования был остановлен.");
							UnlockForm(this);
                        });
						return;
                    }

                    std::string filePath;
                    try {
                        // Получаем путь к файлу из элемента ListBox
                        filePath = stdStr(lstCrypts->Items->Strings[i]);

                        TThread::Synchronize(nullptr, [=]() {
                            Gauge1->Progress = 0;  // Обнуляем прогресс
                            memStatus->Lines->Add("Расшифрование файла: " + lstCrypts->Items->Strings[i]);
                        });

                        std::string result = cryptoArchiveReader.decryptArchive(filePath);

                        TThread::Synchronize(nullptr, [=]() {
                            memStatus->Lines->Add(strStd(result));
                            progress.store(0);  // Сбрасываем прогресс
							Gauge1->Progress = 100;
                        });
                    } catch (const std::exception& e) {
                        TThread::Synchronize(nullptr, [=]() {
							memStatus->Lines->Add((filePath + " - ошибка расшифрования!").c_str());
						});
					}

				}

                // По завершению расшифрования меняем текст кнопки обратно на "Расшифровать"
                TThread::Synchronize(nullptr, [=]() {
                    btnDecrypt->Caption = "Расшифровать";
                    isProcessing = false;  // Сбрасываем флаг
					Timer1->Enabled = false;  // Останавливаем таймер
					UnlockForm(this);
				});
            } catch (const std::exception& e) {
                TThread::Synchronize(nullptr, [=]() {
                    MessageDlg(strStd(std::string{e.what()}), mtError, TMsgDlgButtons() << mbOK, 0);
                    btnDecrypt->Caption = "Расшифровать";  // Возвращаем текст кнопки
                    isProcessing = false;  // Сбрасываем флаг
					Timer1->Enabled = false;  // Останавливаем таймер
					UnlockForm(this);
                });
            }
        });

        decryptThread.detach();  // Отсоединяем поток, чтобы он работал параллельно
        Timer1->Enabled = true;  // Включаем таймер для обновления прогресса

    } else {
        // Режим остановки процесса
        stopFlag.store(true);  // Устанавливаем флаг остановки
        TThread::Synchronize(nullptr, [=]() {
            memStatus->Lines->Add("Остановка процесса расшифрования...");
            btnDecrypt->Caption = "Расшифровать";
            isProcessing = false;
			Timer1->Enabled = false;
			UnlockForm(this);
		});
	}

	btnDecrypt->Enabled = true;
	Application->ProcessMessages();
}


//---------------------------------------------------------------------------

void __fastcall TformMain::Button5Click(TObject *Sender)
{
	AnsiString serialNumber;


}

//---------------------------------------------------------------------------

bool isPasswordStrong(const std::string& password, std::string& message) {
    if (password.size() < 8) {
        message = "Пароль должен содержать минимум 8 символов.";
        return false;
    }
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (char ch : password) {
        if (std::islower(ch)) hasLower = true;
        else if (std::isupper(ch)) hasUpper = true;
        else if (std::isdigit(ch)) hasDigit = true;
        else if (std::ispunct(ch)) hasSpecial = true;
    }
    if (!hasLower) message = "Пароль должен содержать минимум одну строчную букву.";
    else if (!hasUpper) message = "Пароль должен содержать минимум одну заглавную букву.";
    else if (!hasDigit) message = "Пароль должен содержать минимум одну цифру.";
    else if (!hasSpecial) message = "Пароль должен содержать минимум один специальный символ.";
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

void __fastcall TformMain::btnGenerateRsaKeysClick(TObject *Sender)
{
	std::string username = stdStr(EditUserName->Text);
	std::string password = stdStr(EditPassword->Text);

	if (username.empty() || password.empty()) {
		ShowMessage("Имя пользователя или пароль не могут быть пустыми!");
		return;
	}

    std::string message;
    if (!isPasswordStrong(password, message)) {
        ShowMessage("Пароль не соответствует требованиям безопасности:\n" + strStd(message) +
                    "\nТребования:\n- Минимум 8 символов\n- Строчные и заглавные буквы\n- Минимум одна цифра\n- Минимум один специальный символ");
        return;
	}

	// Проверка, существуют ли уже ключи для данного пользователя
	std::string rsaKeysFolder = getExecutablePath() + "\\rsa_keys";
	std::string privateKeyPath = rsaKeysFolder + "\\" + username + "_prikey_encrypted.pem";
	std::string publicKeyPath = rsaKeysFolder + "\\" + username + "_pubkey.pem";

	// Проверка наличия старых ключей
	bool keysExist = std::filesystem::exists(privateKeyPath) || std::filesystem::exists(publicKeyPath);

	if (keysExist) {
		// Выводим предупреждение о том, что старые ключи будут удалены
		int result = MessageDlg("RSA ключи для этого пользователя уже существуют. Продолжение удалит старые ключи. Вы уверены, что хотите продолжить?", mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0);

		// Если пользователь нажал "No", выходим из функции
		if (result != mrYes) {
			return;
		}

		result = MessageDlg("Расшифровать ранее полученные криптограммы будет невозможно. Вы уверены, что хотите продолжить?", mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0);

		// Если пользователь нажал "No", выходим из функции
		if (result != mrYes) {
			return;
		}

	}

	try {

		KeyGenerator generator;
		generator.GenerateKeys(username, password);
		ShowMessage("RSA ключи успешно сгенерированы и сохранены!");
		EditPassword->Clear();

		if(keysExist)
		{
			mapCryptoArchiveCreators[username] = CryptoArchiveCreator{username, &stopFlag, &progress};
			return;
		}

		mapCryptoArchiveCreators.insert({username, CryptoArchiveCreator{username, &stopFlag, &progress}});
        lstOperators->Items->Add(username.c_str());
		chkListAbonents->Items->Add(username.c_str());

	} catch (const std::exception& e) {
		ShowMessage(e.what());
	}

	lbAuto->Caption = "Нет авторизации!";
}

//---------------------------------------------------------------------------

void __fastcall TformMain::btnSaveOpenRsaKeyClick(TObject *Sender)
{

	std::string pub_key_filename = std::string(getExecutablePath()) + "/rsa_keys/" +  stdStr(EditUserName->Text) + "_pubkey.pem";

	if (!std::filesystem::exists(pub_key_filename)) {
        ShowMessage("Ключ не найден!");
        return; // Возвращаем пустую строку, если ключ не найден
	}
    String Dir;
    if (SelectDirectory("Выберите папку для сохранения файлов", "пк", Dir, TSelectDirExtOpts(), 0)) {
		std::string destination_path = stdStr(Dir) + "/" + std::filesystem::path(pub_key_filename).filename().string();
		try {
			std::filesystem::copy(pub_key_filename, destination_path, std::filesystem::copy_options::overwrite_existing);
			ShowMessage("Файл '"+ EditUserName->Text +"_pubkey.pem' успешно скопирован в: " + Dir);
		} catch (const std::filesystem::filesystem_error& e) {
			ShowMessage("Ошибка при копировании файла: " + AnsiString(e.what()));
		}
	}

}

//---------------------------------------------------------------------------

void __fastcall TformMain::FormShow(TObject *Sender)
{
	EditPassword->PasswordChar = '*';
    EditUserName->SetFocus();
    std::string exePath = getExecutablePath();

	// Добавляем поддиректорию для папки ключей (если они находятся в подпапке)
	std::string rsaKeysFolder = exePath + "\\rsa_keys";  // Папка с ключами
	// Сканируем папку и получаем пары файлов открытых и закрытых ключей
    std::pair<std::vector<std::string>, std::vector<std::string>> keys = scanRSAKeysFolder(rsaKeysFolder);
	for (const auto& pubKey : keys.first) {
		std::string displayName = extractNameBeforeUnderscore(pubKey);
		chkListAbonents->Items->Add(displayName.c_str());
		mapCryptoArchiveCreators.insert({displayName, CryptoArchiveCreator{displayName, &stopFlag, &progress}});
	}
	// Добавляем найденные приватные ключи в список, обрезая имена до символа "_"
    for (const auto& priKey : keys.second) {
        std::string displayName = extractNameBeforeUnderscore(priKey);
        lstOperators->Items->Add(displayName.c_str());
	}
}

//---------------------------------------------------------------------------

void __fastcall TformMain::lstOperatorsClick(TObject *Sender)
{
	if(lstOperators->ItemIndex != -1)
	EditUserName->Text = lstOperators->Items->Strings[lstOperators->ItemIndex];
}

//---------------------------------------------------------------------------

void __fastcall TformMain::N1Click(TObject *Sender)
{
	lstCrypts->Items->Clear();
	lstAttach->Items->Clear();

	lbAttachInfo->Caption = "Файлов приложений - 0";
}
//---------------------------------------------------------------------------

void __fastcall TformMain::Timer2Timer(TObject *Sender)
{

	static auto setAutorizationStatus = [this] (const String& text, bool status)
	{
		lbAuto->Caption = text;
		pcMain->Visible = status;
		lbAuto->Visible = status;

		if(!status)
		 memStatus->Lines->Clear();
	};

	std::string username = stdStr(EditUserName->Text);
	std::string password = stdStr(EditPassword->Text);

	if (username.empty() || password.empty()) {

		autoriz = false;
		pcMain->Visible = false;
        lbAuto->Visible = false;
		return;
	}
	try {

		std::string decrypted_pri_key = KeyGenerator::DecryptPrivateKey(username, password);

		if (decrypted_pri_key.find("PRIVATE") != std::string::npos) {
			// Успех: показываем окно с информационным стилем
			auto text = "Авторизован как оператор: \"" + String{username.c_str()} + "\"";
			setAutorizationStatus(text,true);

		} else {
			 // No
			 setAutorizationStatus("Нет авторизации!",false);
		}
	} catch (const std::exception& e) {

		setAutorizationStatus("Нет авторизации!",false);
	}
}

//---------------------------------------------------------------------------

void TformMain::UpdateLabelWithFileInfo() {

	auto label = lbAttachInfo;
	auto listBox = lstAttach;

	int fileCount = listBox->Items->Count; // Количество файлов
    __int64 totalSize = 0; // Общий размер файлов в байтах

    // Проходим по каждому файлу в ListBox
    for (int i = 0; i < listBox->Items->Count; i++) {
        String fileName = listBox->Items->Strings[i];
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;

        // Получаем информацию о файле
        if (GetFileAttributesExW(fileName.c_str(), GetFileExInfoStandard, &fileInfo)) {
            __int64 fileSize = (__int64(fileInfo.nFileSizeHigh) << 32) + fileInfo.nFileSizeLow;
            totalSize += fileSize;
        }
    }

    // Определяем единицу измерения и форматируем объем
    String sizeStr;
    if (totalSize < 1024) {
        sizeStr = IntToStr(totalSize) + " байт";
    } else if (totalSize < 1024 * 1024) {
        sizeStr = FormatFloat("#0.0", totalSize / 1024.0) + " КБ";
    } else {
        sizeStr = FormatFloat("#0.0", totalSize / (1024.0 * 1024.0)) + " МБ";
    }

    // Обновляем текст лейбла на русском языке
    label->Caption = "Файлов приложений - " + IntToStr(fileCount) + ", общий объем: " + sizeStr;

    // Меняем цвет лейбла в зависимости от общего объема
    if (totalSize > 250 * 1024 * 1024) { // Больше 250 МБ
		label->Font->Color = clRed;
    } else {
		label->Font->Color = clGreen;
    }
}

//---------------------------------------------------------------------------

void __fastcall TformMain::actGenKeysExecute(TObject *Sender)
{
    btnGenerateRsaKeysClick(Sender);
}

//---------------------------------------------------------------------------

void __fastcall TformMain::edtDirCryptsDblClick(TObject *Sender)
{

	String Dir = edtDirCrypts->Text;
	SelectDirectory("Выберите папку для сохранения криптограмм", "пк", Dir, TSelectDirExtOpts(),0);
	edtDirCrypts->Text = Dir;

}

//---------------------------------------------------------------------------

void __fastcall TformMain::edtSaveMessageDirDblClick(TObject *Sender)
{

	String Dir = edtSaveMessageDir->Text;
	SelectDirectory("Выберите папку для сохранения сообщений", "пк", Dir, TSelectDirExtOpts(), 0);
	edtSaveMessageDir->Text = Dir;

}

//---------------------------------------------------------------------------

void __fastcall TformMain::pcMainChange(TObject *Sender)
{
	memStatus->Lines->Clear();
	lbAttachInfo->Caption = "Файлов приложений - 0";
	lstCrypts->Items->Clear();
	lstAttach->Items->Clear();
    Gauge1->Progress = 0;
}

//---------------------------------------------------------------------------

void __fastcall TformMain::btnLoadOpenRsaKeyClick(TObject *Sender)
{
	TFileOpenDialog *dlg = new TFileOpenDialog(this);
    dlg->DefaultFolder = GetCurrentDir();
    lstCrypts->Items->Clear();
    // Разрешаем множественный выбор файлов
    dlg->Options << fdoAllowMultiSelect;
    // Устанавливаем фильтр файлов с расширением .cry
	dlg->FileTypes->Add()->DisplayName = "RSA Files (*.pem)";
	dlg->FileTypes->Items[0]->FileMask = "*.pem";
    // Показываем диалоговое окно
	if (dlg->Execute()) {
        // Очищаем ListBox перед добавлением новых файлов
        // Проходим по каждому выбранному файлу и добавляем его в ListBox
		for (int i = 0; i < dlg->Files->Count; i++) {

			try
			{
				copyOpenRsaKey(dlg->Files->Strings[i]);
			}
			 catch (const std::exception& e)
			{
				ShowMessage(e.what());
			}

        }
    }
    // Удаляем диалог
	delete dlg;
}

//---------------------------------------------------------------------------

void TformMain::copyOpenRsaKey(const String& new_rsa_key_path)
{
	std::string new_rsa_key_path_std = stdStr(new_rsa_key_path);
	std::string rsa_file_name = stdStr(ExtractFileName(new_rsa_key_path));

	bool keyIsNotPubkey = new_rsa_key_path_std.find("pubkey") == std::string::npos;

	if(keyIsNotPubkey)
	{
		memStatus->Lines->Add(" - Выбранный файл '" + strStd(rsa_file_name)  +"' не является открытым ключом RSA!");
        return;
	}

	std::string rsaKeysFolder = getExecutablePath() + "\\rsa_keys";
	std::string oldOpenKeyPath = rsaKeysFolder + "\\" +rsa_file_name;

	bool keysExist = std::filesystem::exists(oldOpenKeyPath);

	auto getAbonentNameStr = [&rsa_file_name]
	{
		auto pos_ = rsa_file_name.find('_');
		auto abonent_name = rsa_file_name.substr(0, pos_);
		return strStd(abonent_name);
	};

	if (keysExist) {
		// Выводим предупреждение о том, что старые ключи будут удалены
		int result = MessageDlg("Ключ абонента '"+ strStd(rsa_file_name) + "' уже существует. Продолжение удалит старый ключ.\nВы уверены, что хотите продолжить?", mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0);

		// Если пользователь нажал "No", выходим из функции
		if (result != mrYes) {
			return;
		}
	}
		String result;
		try {
			std::filesystem::copy(new_rsa_key_path_std, oldOpenKeyPath, std::filesystem::copy_options::overwrite_existing);

			String abonent_name = getAbonentNameStr();

			if(keysExist)
				result = " - Ключ абонента '" + abonent_name + "' - обновлен";
			else
			{
				result = " - Ключ абонента '" + abonent_name + "' - добавлен";
				chkListAbonents->Items->Add(abonent_name);
			}

			Application->ProcessMessages();

		} catch (const std::filesystem::filesystem_error& e) {

			ShowMessage(e.what());
			result = "Ошибка добавления ключа абонента '" + getAbonentNameStr() + "' !";
		}

		memStatus->Lines->Add(result);

}

//-------------------------------------------------------------------------

void __fastcall TformMain::delAbonentClick(TObject *Sender)
{
	auto ind = chkListAbonents->ItemIndex;

	if(ind == -1)
		return;

	try
	{
		delAbonentFunc(ind);
	}
	catch (const std::exception& e)
	{
		ShowMessage(e.what());
	}

}

//---------------------------------------------------------------------------

void TformMain::delAbonentFunc(int ind)
{

	std::string rsaKeysFolder = getExecutablePath() + "\\rsa_keys";
	std::string abonent = stdStr(chkListAbonents->Items->Strings[ind]) ;
	std::string publicKeyPath = rsaKeysFolder + "\\" + abonent + "_pubkey.pem";

	bool keysExist = std::filesystem::exists(publicKeyPath);

	if (keysExist) {
		// Выводим предупреждение о том, что старые ключи будут удалены
		int result = MessageDlg("Продолжение удалит открытый RSA ключ абонента '" + strStd(abonent) + "'.\nВы уверены, что хотите продолжить?", mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0);

		// Если пользователь нажал "No", выходим из функции
		if (result != mrYes) {
			return;
		}
	}
	else
	{
		chkListAbonents->Items->Delete(ind);
		memStatus->Lines->Add(" - Открытый RSA ключ абонента '" + strStd(abonent) +"' не найден!");
		return;
	}

	chkListAbonents->Items->Delete(ind);
	std::filesystem::remove(publicKeyPath);
	memStatus->Lines->Add(" - Открытый RSA ключ абонента '" + strStd(abonent) +"' удален!");
}
void __fastcall TformMain::delOperatorClick(TObject *Sender)
{
	auto ind = lstOperators->ItemIndex;

	if(ind == -1)
		return;

	try
	{
		delOperatorFunc(ind);
	}
	catch (const std::exception& e)
	{
		ShowMessage(e.what());
	}
}

//---------------------------------------------------------------------------

void TformMain::delOperatorFunc(int ind)
{
	String str_operator = lstOperators->Items->Strings[ind];
	std::string rsaKeysFolder = getExecutablePath() + "\\rsa_keys";
	std::string operatoR = stdStr(str_operator) ;
	std::string priKeyPath = rsaKeysFolder + "\\" + operatoR + "_prikey_encrypted.pem";

	bool keysExist = std::filesystem::exists(priKeyPath);

	if (keysExist) {
		// Выводим предупреждение о том, что старые ключи будут удалены
		int result = MessageDlg("Продолжение удалит закрытый RSA ключ оператора '" + strStd(operatoR) + "'.\nВы уверены, что хотите продолжить?", mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0);

		// Если пользователь нажал "No", выходим из функции
		if (result != mrYes) {
			return;
		}

		result = MessageDlg("Все криптограммы направленные ранее оператору '" + strStd(operatoR) + "' расшифровать будет невозможно.\nВы уверены, что хотите продолжить?", mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0);

		// Если пользователь нажал "No", выходим из функции
		if (result != mrYes) {
			return;
		}

	}
	else
	{
		lstOperators->Items->Delete(ind);
		memStatus->Lines->Add(" - Закрытый RSA ключ оператора '" + strStd(operatoR) +"' не найден!");
		return;
	}

	lstOperators->Items->Delete(ind);
	std::filesystem::remove(priKeyPath);
	memStatus->Lines->Add(" - Закрытый RSA ключ оператора '" + strStd(operatoR) +"' удален!");

	if(str_operator == EditUserName->Text)
	{
		EditUserName->Text = "";
		EditPassword->Text = "";
		EditUserName->SetFocus();
	}

}



void __fastcall TformMain::memTextChange(TObject *Sender)
{
	const int maxLength = 150; // Максимальное количество символов
	if (memText->Text.Length() > maxLength) {
		memText->Text = memText->Text.SubString(1, maxLength); // Обрезаем текст до разрешенного количества символов
		memText->SelStart = memText->Text.Length(); // Перемещаем каретку в конец
		ShowMessage("Превышен максимальный размер сопроводительного письма (130 знаков)!");
	}
}
//---------------------------------------------------------------------------


void __fastcall TformMain::Timer1Timer(TObject *Sender)
{
	Gauge1->Progress = progress.load();  // Обновляем прогресс на основе атомарной переменной

	// Если прогресс достиг 100%, останавливаем таймер
	if (Gauge1->Progress >= 100) {
		Timer1->Enabled = false;
		memStatus->Lines->Add("Процесс шифрования завершен.");
	}
}
//---------------------------------------------------------------------------
void __fastcall TformMain::LockFormExceptStopButton(TWinControl* parentControl) {
    // Итерируем по всем дочерним элементам текущего контейнера
    for (int i = 0; i < parentControl->ControlCount; i++) {
        TControl *control = parentControl->Controls[i];

        // Блокируем кнопки, кроме кнопки остановки
		if ((dynamic_cast<TButton*>(control) || dynamic_cast<TEdit*>(control) || dynamic_cast<TMaskEdit*>(control)
		 || dynamic_cast<TListBox*>(control) || dynamic_cast<TCheckListBox*>(control) ) && control != btnDecrypt && control != btnEncrypt) {
            control->Enabled = false;
		}

        // Если это PageControl, то отключаем переключение вкладок
		if (dynamic_cast<TPageControl*>(control)) {
            TPageControl *pageControl = static_cast<TPageControl*>(control);

            // Блокируем переключение вкладок, но оставляем активные элементы внутри текущей вкладки
            for (int j = 0; j < pageControl->PageCount; j++) {
                TTabSheet *tabSheet = pageControl->Pages[j];
                if (tabSheet != pageControl->ActivePage) {
                    tabSheet->TabVisible = false;  // Скрываем вкладки
                }
            }
        }

        // Рекурсивно обрабатываем дочерние элементы, если это контейнер
        if (dynamic_cast<TWinControl*>(control) != nullptr) {
            LockFormExceptStopButton(static_cast<TWinControl*>(control));
        }
    }

    // Принудительное обновление интерфейса
    Application->ProcessMessages();
}

void __fastcall TformMain::UnlockForm(TWinControl* parentControl) {
    // Итерируем по всем дочерним элементам текущего контейнера
    for (int i = 0; i < parentControl->ControlCount; i++) {
        TControl *control = parentControl->Controls[i];

		// Включаем все элементы, кроме кнопки остановки
		if(control != nullptr)
			control->Enabled = true;

         if (dynamic_cast<TPageControl*>(control)) {
            TPageControl *pageControl = static_cast<TPageControl*>(control);
            // Показываем все вкладки снова
            for (int j = 0; j < pageControl->PageCount; j++) {
                pageControl->Pages[j]->TabVisible = true;
            }
        }

        // Если компонент является контейнером (например, TPageControl, TPanel), рекурсивно обрабатываем его дочерние элементы
		if (dynamic_cast<TWinControl*>(control) != nullptr) {
            UnlockForm(static_cast<TWinControl*>(control));
        }
    }
}
