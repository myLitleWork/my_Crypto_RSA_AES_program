# ShifRator3

ShifRator3 — настольное приложение C++Builder VCL для создания RSA-ключей,
шифрования пользовательских файлов в криптоархивы и их расшифрования. Проект
также поддерживает вход по паролю и локальную привязку системного токена к
компьютеру.

## Быстрый старт разработчика

Требуется C++Builder / RAD Studio 22.0 с Win32 toolchain. Проект использует
Boost Serialization из установленного toolchain, хранимую в репозитории
import-библиотеку `myCrypto-bcb.lib` и проверенную runtime-библиотеку
`runtime\myCrypto.dll`.
Зависимости не загружаются автоматически.

```bat
tools\build.bat Debug
tools\release.bat
```

`tools\build.bat Debug` собирает только `Debug|Win32`: он не меняет версию,
не создаёт portable package и не публикует ZIP. `tools\release.bat` —
единственная каноническая команда Release; полный контракт описан в
`BUILD.md`.

Пользовательские инструкции находятся в `tutorials`. Внутренние материалы из
`docs` в release не входят.
