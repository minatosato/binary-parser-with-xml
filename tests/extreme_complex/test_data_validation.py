#!/usr/bin/env python3
"""実際のデータでパース結果を検証"""
import sys
import os
import subprocess
import struct

def main():
    print("データ検証テスト - 実際の値でパース結果を確認")
    print("=" * 60)
    
    # Timestampで詳細なテスト
    print("\n1. Timestamp構造体の詳細テスト")
    
    # XML生成
    subprocess.run([
        sys.executable,
        '../../src/header_to_xml/header_to_xml.py',
        'base_types.h',
        'Timestamp',
        '-o', 'timestamp.xml'
    ], capture_output=True)
    
    # 特定の値でバイナリ作成
    with open('timestamp.bin', 'wb') as f:
        # year=2024, month=6, day=29, hour=20, minute=30, second=45
        # padding=0, microseconds=123456
        f.write(struct.pack('<H', 2024))      # year
        f.write(struct.pack('<B', 6))         # month
        f.write(struct.pack('<B', 29))        # day
        f.write(struct.pack('<B', 20))        # hour
        f.write(struct.pack('<B', 30))        # minute
        f.write(struct.pack('<B', 45))        # second
        f.write(struct.pack('<B', 0))         # padding
        f.write(struct.pack('<I', 123456))    # microseconds
    
    # パース実行
    result = subprocess.run([
        '../../build/parse_binary',
        'timestamp.xml',
        'timestamp.bin',
        'Timestamp'
    ], capture_output=True, text=True)
    
    print("期待値:")
    print("  year = 2024")
    print("  month = 6")
    print("  day = 29")
    print("  hour = 20")
    print("  minute = 30")
    print("  second = 45")
    print("  microseconds = 123456")
    
    print("\n実際の出力:")
    for line in result.stdout.strip().split('\n'):
        if any(field in line for field in ['year', 'month', 'day', 'hour', 'minute', 'second', 'micro']):
            print(f"  {line.strip()}")
    
    # Bitfield詳細テスト
    print("\n\n2. Bitfield（ConfigHeader）の詳細テスト")
    
    subprocess.run([
        sys.executable,
        '../../src/header_to_xml/header_to_xml.py',
        'base_types.h',
        'ConfigHeader',
        '-o', 'config.xml'
    ], capture_output=True)
    
    with open('config.bin', 'wb') as f:
        # version=42 (8bit), flags=0xABCDEF (24bit)
        # 合計32bit = 0xABCDEF2A (little endian: 2A EF CD AB)
        value = (0xABCDEF << 8) | 42
        f.write(struct.pack('<I', value))
    
    result = subprocess.run([
        '../../build/parse_binary',
        'config.xml',
        'config.bin',
        'ConfigHeader'
    ], capture_output=True, text=True)
    
    print("期待値:")
    print("  version = 42 (0x2A)")
    print("  flags = 11259375 (0xABCDEF)")
    
    print("\n実際の出力:")
    for line in result.stdout.strip().split('\n'):
        if 'version' in line or 'flags' in line:
            print(f"  {line.strip()}")
    
    # 複雑なネスト構造のテスト
    print("\n\n3. 深いネスト構造（DeepestNest）のテスト")
    
    subprocess.run([
        sys.executable,
        '../../src/header_to_xml/header_to_xml.py',
        'ultra_extreme.h',
        'DeepestNest',
        '-o', 'deepest.xml'
    ], capture_output=True)
    
    # XMLのサイズを確認
    import xml.etree.ElementTree as ET
    tree = ET.parse('deepest.xml')
    size = int(tree.getroot().get('size', 0))
    
    with open('deepest.bin', 'wb') as f:
        # 最深部に特定の値を設定
        f.write(struct.pack('<I', 0xDEADBEEF))  # deepest_value
        # 残りは0で埋める
        f.write(b'\x00' * (size - 4))
    
    result = subprocess.run([
        '../../build/parse_binary',
        'deepest.xml',
        'deepest.bin',
        'DeepestNest'
    ], capture_output=True, text=True)
    
    print("期待値:")
    print("  20層の深さにdepest_value = 3735928559 (0xDEADBEEF)")
    
    print("\n実際の出力（deepest_valueを探す）:")
    lines = result.stdout.strip().split('\n')
    found = False
    for i, line in enumerate(lines):
        if 'deepest_value' in line:
            print(f"  行{i}: {line.strip()}")
            found = True
            # 前後の行も表示
            if i > 0:
                print(f"  行{i-1}: {lines[i-1].strip()}")
            if i < len(lines) - 1:
                print(f"  行{i+1}: {lines[i+1].strip()}")
    
    if not found:
        print("  ⚠️  deepest_valueが見つかりませんでした")
        print("  出力行数:", len(lines))
    
    # クリーンアップ
    for f in ['timestamp.xml', 'timestamp.bin', 'config.xml', 'config.bin', 
              'deepest.xml', 'deepest.bin']:
        if os.path.exists(f):
            os.unlink(f)
    
    print("\n✅ データ検証テスト完了！")
    return 0

if __name__ == '__main__':
    sys.exit(main())