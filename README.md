# Binary Parser with XML Configuration

XMLで定義された構造体を使用してバイナリデータを解析するシステムです。構造体定義が変更されても再コンパイルが不要です。

## 🎯 プロジェクトの目的

C++でバイナリデータを構造体にキャストする際、構造体定義が頻繁に変わると都度再コンパイルが必要になります。このプロジェクトは、構造体定義をXMLファイルとして外部化することで、この問題を解決します。

## ✅ 実装済み機能

### 1. Header to XML変換ツール（Python）
- C++ヘッダファイルから構造体定義を抽出
- XML形式で構造体情報を出力
- 複雑なネスト構造、union、bitfield対応
- マクロ展開機能（配列サイズ定義）

### 2. バイナリパーサー（C++）
- XML定義を読み込んでバイナリデータを解析
- 再コンパイル不要で構造体定義の変更に対応
- エンディアン変換サポート
- ビットフィールド値の抽出

### 3. サポートする機能
- ✅ stdint型（uint8_t, int16_t, uint32_t等）
- ✅ float, double, char
- ✅ ネストした構造体
- ✅ union（名前付き・無名）
- ✅ 配列（マクロ展開対応）
- ✅ bitfield
- ✅ typedef構造体
- ✅ #includeによる複数ヘッダファイル
- ✅ パック/アンパック（アライメント制御）
- ✅ エンディアン指定（little/big）

## 🚀 使い方

### 1. ヘッダファイルをXMLに変換

```bash
python3 src/header_to_xml/header_to_xml.py input.h StructName -o output.xml
```

オプション:
- `-p, --packed`: パックされた構造体として処理
- `-o, --output`: 出力XMLファイル名を指定

### 2. バイナリデータを解析

```bash
./build/parse_binary output.xml data.bin StructName
```

オプション:
- `--big-endian`: ビッグエンディアンとして解析（デフォルトはリトルエンディアン）

## 🔧 ビルド方法

### 必要な環境
- C++17対応コンパイラ
- CMake 3.10以降
- Python 3.6以降

### ビルド手順

```bash
mkdir build
cd build
cmake ..
make
```

## 📁 ディレクトリ構成

```
binary-parser-with-xml/
├── src/
│   ├── header_to_xml/        # Python: ヘッダ→XML変換
│   │   └── header_to_xml.py
│   └── binary_parser/        # C++: バイナリパーサー
│       ├── binary_parser.cpp
│       ├── binary_parser.h
│       ├── xml_struct_parser.cpp
│       ├── xml_struct_parser.h
│       └── main.cpp
├── tests/                    # テストコード
├── examples/                 # サンプルコード
├── docs/                     # ドキュメント
├── CMakeLists.txt
└── README.md
```

## 📚 例

### 入力ヘッダファイル (example.h)
```c
#include <stdint.h>

#define BUFFER_SIZE 32

struct ExampleStruct {
    uint32_t id;
    struct {
        uint16_t x;
        uint16_t y;
    } position;
    union {
        uint32_t value;
        uint8_t bytes[4];
    } data;
    uint8_t flags : 3;
    uint8_t mode : 5;
    char name[BUFFER_SIZE];
};
```

### 生成されるXML
```xml
<struct name="ExampleStruct" size="48">
  <field name="id" type="uint32_t" offset="0" size="4"/>
  <field name="position" offset="4" size="4">
    <struct>
      <field name="x" type="uint16_t" offset="0" size="2"/>
      <field name="y" type="uint16_t" offset="2" size="2"/>
    </struct>
  </field>
  <field name="data" offset="8" size="4">
    <union>
      <field name="value" type="uint32_t" offset="0" size="4"/>
      <field name="bytes" type="uint8_t" array_size="4" offset="0" size="4"/>
    </union>
  </field>
  <field name="flags" type="uint8_t" bits="3" bit_offset="0" offset="12" size="1"/>
  <field name="mode" type="uint8_t" bits="5" bit_offset="3" offset="12" size="1"/>
  <field name="name" type="char" array_size="32" offset="16" size="32"/>
</struct>
```

## 🧪 テスト

```bash
# Pythonテストの実行
python3 tests/test_header_to_xml.py
python3 tests/test_macro_expansion.py

# 統合テストの実行
python3 tests/integration/test_full_workflow.py
python3 tests/integration/test_macro_integration.py
```

## 📝 制限事項

詳細は[docs/LIMITATIONS.md](docs/LIMITATIONS.md)を参照してください。

主な制限:
- ポインタ型は未対応
- 多次元配列は未対応
- C++のクラス機能は未対応
- enum型は未対応

## 🛠️ 開発

このプロジェクトはTDD + RGRC（Red, Green, Refactor, Commit）の手法で開発されています。

## 📄 ライセンス

このプロジェクトはオープンソースです。