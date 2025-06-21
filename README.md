# ifdcm.sph - DICOM Image Plugin for Susie

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)

- あふｗでレントゲン画像を見たくて開発しました
- DICOM（Digital Imaging and Communications in Medicine）画像ファイルをSusieプラグイン対応の画像ビューアで表示するためのプラグインです

## 📋 目次

- [開発環境](#開発環境)
- [ビルド手順](#ビルド手順)
- [使い方](#使い方)
- [参考資料](#参考資料)

## 🖥️ 開発環境

1. **Visual Studio 2022**
   - C++開発ツールをインストール

2. **vcpkg** (パッケージマネージャー)
   - インストール
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git C:\dev\vcpkg
   C:\dev\vcpkg\bootstrap-vcpkg.bat
   ```
   - PATHを設定
   ```powershell
   setx VCPKG_ROOT "C:\dev\vcpkg"
   ```

3. **DCMTKライブラリ**
   - インストール
   ```powershell
   C:\dev\vcpkg\vcpkg install dcmtk:x64-windows
   ```

## 🔨 ビルド手順

1. **リポジトリのクローン**
   ```powershell
   git clone https://github.com/mtkhs/ifdcm
   cd ifdcm
   ```

2. **ビルドディレクトリの作成**
   ```powershell
   mkdir build
   cd build
   ```

3. **CMakeの実行**
   ```powershell
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

4. **ビルドの実行**
   ```powershell
   cmake --build . --config Release
   ```

## 📖 使い方

1. **プラグイン本体をインストール**
   - `ifdcm.sph`をあふｗのSusieプラグインフォルダへ

2. **必要なDLLをインストール**
   - 以下のDLLをあふｗのフォルダへ：
   ```
   dcmdata.dll
   dcmimgle.dll
   oficonv.dll
   oflog.dll
   ofstd.dll
   ```
   - 以下からも入手可能です
   - **[Download of DCMTK Tools](https://dicom.offis.de/en/dcmtk/dcmtk-tools/)**: `DCMTK 3.6.9 - executable binaries`

## 📚 参考資料

### 開発に使用したソフトウェア・参考にした情報
- **[Cursor - The AI Code Editor](https://www.cursor.com/ja)**: ソースコード99%書いてもらいました
- **[Susie 32bit / 64bit Plug-in の仕様(2025-1-25版) - TORO's Library](http://toro.d.dooo.jp/dlsphapi.html)**: susie.h を拝借
- **[runspx](https://github.com/toroidj/runspx)**: APIの動作確認用
- **[Extend convert Susie Plug-in for developer](http://toro.d.dooo.jp/slplugin.html#spextend_dev)**: APIの動作確認用
  - これのおかげであふｗのAPI呼び出しが`GetPictureW`ではなく`GetPicture`であることに気付けた
- Susieプラグインに関して様々な情報を公開してくれているTORO氏に感謝
- DICOMに関して参考情報が無いのは仕様把握せずともAIにコードを書いてもらえたから

---

**注意**: このプラグインは医療診断目的での使用は想定していません。医療用途では専用のDICOMビューアをご使用ください。
