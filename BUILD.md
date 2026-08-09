# Сборка и выпуск ShifRator3

## Требования

- Windows и C++Builder / RAD Studio 22.0;
- Win32 C++ toolchain и `rsvars.bat` в стандартном каталоге Studio 22.0;
- Boost Serialization из установленного toolchain;
- хранимая в репозитории project-local `myCrypto-bcb.lib`;
- подтверждённая Win32 runtime-зависимость `runtime\myCrypto.dll`;
- общий workspace helper `D:\MyWorkSpaceMain\TOOLS\scripts\release`.

Скрипты ничего не скачивают и не устанавливают.

## Debug

```bat
tools\build.bat Debug
```

Команда собирает `Debug|Win32` без version bump, package и cloud publish.
Для компиляции без линковки допускается `tools\build.bat Debug NoLink`.
`Release NoLink` намеренно запрещён.

## Канонический Release

```bat
tools\release.bat
```

Это синхронный entrypoint: preflight и mutex, fingerprint/version sync,
максимум один patch bump, `Release|Win32`, проверка PE, portable package,
manifest/checksums, verifier и настроенная cloud-публикация. Код возврата `0`
означает, что локальный package уже проверен, а при настроенном CloudRoot уже
созданы ZIP и `.zip.sha256`. Обычный entrypoint требует чистого Git-состояния
до изменения версии; исходные `HEAD`, dirty-state и digest списка изменений
сохраняются в package manifest.

Для восстановления уже выбранной версии после неуспешной сборки:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts\Build-ShifRator3Package.ps1 -AllowDirty -SkipVersionBump
```

Этот режим повторяет полный build/package/publish workflow, но не повышает
patch. Отдельный package-only helper не изменяет версию.

## Версия и fingerprint

Канон — `app-version.json`, формат `major.minor.patch`. Release sync создаёт
`ShifRator3Version.rc`; PE получает `FileVersion=A.B.C.0` и
`ProductVersion=A.B.C`, а Caption читает ProductVersion из VERSIONINFO.

Fingerprint ограничен корневыми исходниками/ресурсами, project-local
`scripts`, `tools`, всем `tutorials`, deployment template,
`runtime\myCrypto.dll` и `myCrypto-bcb.lib`. Он не обходит `.git`, `Win32`,
`Win64`, `out`, IDE recovery/cache, каталоги ключей, токены и пользовательские
криптоархивы.
Обычный Release повышает patch только при изменении fingerprint;
намеренный новый номер без изменения fingerprint выполняется тем же workflow:

```bat
tools\release.bat ForceBump
```

`myProgram.cbproj` не входит в byte-fingerprint. Значимое изменение его
Release-настроек требует `ForceBump`.

## Package

```text
out\package\<version>\ShifRator3-<version>\
  app\myProgram.exe
  app\myCrypto.dll
  tutorials\...
  Verify-Package.cmd
  Verify-Package.ps1
  package-manifest.json
  package-checksums.sha256
```

В package не входят `docs`, README/BUILD, исходники, `.lib`, debug outputs,
ключи, токены, пароли, пользовательские архивы, логи,
настройки и каталоги `Отправка`/`Прием`. Туториалы и template проверяются
fail-closed; обхода `-AllowSensitive` нет. Manifest содержит release
fingerprint и исходный Git provenance; verifier сверяет пути, роли, размеры и
SHA-256 каждого payload-файла.

## CloudRoot

Публикация включается через параметр `-CloudRoot`, process-env или User-env
`WORKSPACE_RELEASE_CLOUD_ROOT`. В CloudRoot создаются только версионные ZIP и
`.zip.sha256` по правилам workspace. Для локальной проверки без публикации:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts\Build-ShifRator3Package.ps1 -AllowDirty -SkipCloudPublish
```

`-SkipCloudPublish` не отключает локальную упаковку и verifier.

## Аудит абсолютных путей

Старые Win32-пути к `F:\ShifRator` и пользовательскому каталогу Embarcadero
заменены project-local ссылками. Заголовки и import-библиотеки Boost берутся
из `$(BDS)\include\boost` и `$(BDS)\lib\boost`. Неактивные Win64-настройки,
включая прежний `F:\ShifRator3\Win64`, намеренно не изменены: поддерживаемая
Release-платформа этой автоматизации — только Win32.
