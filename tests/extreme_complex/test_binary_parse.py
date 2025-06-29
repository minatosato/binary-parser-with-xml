#!/usr/bin/env python3
"""バイナリパース機能のテスト"""
import sys
import os
import subprocess
import struct
import tempfile

def create_test_binary(filename, size):
    """テスト用のバイナリファイルを作成"""
    with open(filename, 'wb') as f:
        # ヘッダー部分
        f.write(struct.pack('<II', 0xDEADBEEF, 1))  # magic, version
        f.write(struct.pack('<Q', 1234567890))      # timestamp
        f.write(b'\x00' * 16)                        # uuid
        
        # 残りをゼロで埋める
        remaining = size - f.tell()
        if remaining > 0:
            f.write(b'\x00' * remaining)

def main():
    print("バイナリパース機能テスト")
    print("=" * 50)
    
    # ExtremelyComplexGameStateをテスト
    struct_name = 'ExtremelyComplexGameState'
    header_file = 'extreme_nested.h'
    
    print(f"\n1. {struct_name}のXML生成...")
    result = subprocess.run([
        sys.executable,
        '../../src/header_to_xml/header_to_xml.py',
        header_file,
        struct_name,
        '-o', 'test.xml'
    ], capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"❌ エラー: {result.stderr}")
        return 1
    
    # XMLから構造体サイズを取得
    import xml.etree.ElementTree as ET
    tree = ET.parse('test.xml')
    root = tree.getroot()
    struct_size = int(root.get('size', 0))
    
    print(f"✅ 成功！ 構造体サイズ: {struct_size:,} バイト")
    
    # テスト用バイナリファイル作成
    print(f"\n2. テスト用バイナリファイル作成...")
    bin_file = 'test.bin'
    create_test_binary(bin_file, struct_size)
    print(f"✅ {bin_file} ({struct_size:,} バイト) を作成")
    
    # バイナリパース実行
    print(f"\n3. バイナリパースを実行...")
    result = subprocess.run([
        '../../build/parse_binary',
        'test.xml',
        bin_file,
        struct_name
    ], capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"❌ エラー: {result.stderr}")
        return 1
    
    print("✅ 成功！")
    
    # 出力の一部を表示
    output_lines = result.stdout.strip().split('\n')
    print(f"\n4. パース結果（最初の20行）:")
    for i, line in enumerate(output_lines[:20]):
        print(f"   {line}")
    
    if len(output_lines) > 20:
        print(f"   ... (他 {len(output_lines) - 20} 行)")
    
    # より小さい構造体でもテスト
    print("\n" + "=" * 50)
    print("シンプルな構造体でのテスト")
    
    simple_structs = [
        ('Timestamp', 'base_types.h'),
        ('NetworkAddress', 'network_types.h'),
        ('Item', 'game_types.h'),
    ]
    
    for struct_name, header in simple_structs:
        print(f"\n● {struct_name} のテスト:")
        
        # XML生成
        result = subprocess.run([
            sys.executable,
            '../../src/header_to_xml/header_to_xml.py',
            header,
            struct_name,
            '-o', f'{struct_name}.xml'
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"  ❌ XML生成エラー: {result.stderr.strip()}")
            continue
        
        # サイズ取得
        tree = ET.parse(f'{struct_name}.xml')
        size = int(tree.getroot().get('size', 0))
        
        # バイナリ作成
        with tempfile.NamedTemporaryFile(mode='wb', suffix='.bin', delete=False) as f:
            # 適当なデータを書き込む
            if struct_name == 'Timestamp':
                f.write(struct.pack('<HBBBBBxI', 2024, 1, 1, 12, 0, 0, 0))
            elif struct_name == 'NetworkAddress':
                f.write(struct.pack('<IHHH', 0x7F000001, 8080, 0, 0))
            else:
                f.write(b'\x00' * size)
            bin_file = f.name
        
        # パース実行
        result = subprocess.run([
            '../../build/parse_binary',
            f'{struct_name}.xml',
            bin_file,
            struct_name
        ], capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"  ✅ 成功！ (サイズ: {size} バイト)")
            lines = result.stdout.strip().split('\n')
            for line in lines[:5]:
                print(f"     {line}")
            if len(lines) > 5:
                print(f"     ...")
        else:
            print(f"  ❌ パースエラー: {result.stderr.strip()}")
        
        # クリーンアップ
        os.unlink(f'{struct_name}.xml')
        os.unlink(bin_file)
    
    # 最終クリーンアップ
    for f in ['test.xml', 'test.bin', 'ultra_extreme.xml']:
        if os.path.exists(f):
            os.unlink(f)
    
    print("\n✅ すべてのテスト完了！")
    return 0

if __name__ == '__main__':
    sys.exit(main())