# Binary Parser with XML

バイナリデータをXML定義に基づいて構造体にパースするツールセット。

## 概要

構造体定義が頻繁に変わる環境で、再コンパイルなしにバイナリデータをパースできるようにするツールです。

1. **header_to_xml**: C++ヘッダファイルから構造体定義を抽出してXMLに変換（Python実装）
2. **binary_parser**: XML定義を使用してバイナリデータをパース（C++実装）

## 機能

- 複数ヘッダファイルにまたがる構造体定義の解析（#include追跡）
- 複雑にネストした構造体とunionのサポート
- 無名構造体・無名unionの対応
- bitfieldの対応
- 配列の対応
- offset/size情報の自動計算
- パック/アライメントオプション

## 必要な環境

- Python 3.x（header_to_xml用）
- C++17対応コンパイラ
- CMake 3.20以上
- vcpkg（C++パッケージ管理）

## ビルド方法

### 1. vcpkgのセットアップ（未インストールの場合）

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # macOS/Linux
# または
./bootstrap-vcpkg.bat  # Windows
```

### 2. C++プロジェクトのビルド

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
make
```

## 使用方法

### 1. ヘッダファイルをXMLに変換

```bash
python src/header_to_xml/main.py input.h StructName -o output.xml

# パックされた構造体の場合
python src/header_to_xml/main.py input.h StructName -o output.xml --packed
```

### 2. バイナリデータをパース

```bash
./build/parse_binary output.xml binary_data.bin
```

## 統合テスト

```bash
cd tests/integration
./run_integration_test.sh
```

## サポートされる型

- stdint型: uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t
- 浮動小数点: float, double
- 文字: char（uint8_tとして扱う）