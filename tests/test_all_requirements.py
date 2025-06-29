#!/usr/bin/env python3
"""Test all requirements from やりたいこと.txt"""
import sys
import os
import tempfile
import subprocess
import struct
import xml.etree.ElementTree as ET

def run_header_to_xml(header_file, struct_name, packed=False, output=None):
    """Run header_to_xml.py and return XML content"""
    cmd = [
        sys.executable,
        '../src/header_to_xml/header_to_xml.py',
        header_file,
        struct_name
    ]
    if packed:
        cmd.append('-p')
    if output:
        cmd.extend(['-o', output])
    
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise Exception(f"header_to_xml failed: {result.stderr}")
    
    if output:
        with open(output, 'r') as f:
            return f.read()
    return result.stdout

def run_binary_parser(xml_file, bin_file, struct_name, big_endian=False):
    """Run parse_binary and return output"""
    cmd = ['../build/parse_binary', xml_file, bin_file, struct_name]
    if big_endian:
        cmd.append('--big-endian')
    
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise Exception(f"parse_binary failed: {result.stderr}")
    return result.stdout

def test_basic_types():
    """Test: 基本的な型 (stdint types, float, double, char)"""
    print("TEST: Basic types ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

struct BasicTypes {
    uint8_t  u8;
    int8_t   i8;
    uint16_t u16;
    int16_t  i16;
    uint32_t u32;
    int32_t  i32;
    uint64_t u64;
    int64_t  i64;
    float    f32;
    double   f64;
    char     ch;
};
""")
        header_file = f.name
    
    try:
        xml_content = run_header_to_xml(header_file, 'BasicTypes')
        
        # Verify all types are present
        required = ['uint8_t', 'int8_t', 'uint16_t', 'int16_t', 
                   'uint32_t', 'int32_t', 'uint64_t', 'int64_t',
                   'float', 'double', 'char']
        for type_name in required:
            if f'type="{type_name}"' not in xml_content:
                print(f"FAILED: Type {type_name} not found")
                return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def test_nested_structures():
    """Test: ネストした構造体、無名構造体"""
    print("TEST: Nested structures and anonymous structs ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

struct Nested {
    uint32_t id;
    struct {
        uint16_t x;
        uint16_t y;
    };  // Anonymous struct
    struct Inner {
        uint8_t data[4];
    } inner;
};
""")
        header_file = f.name
    
    try:
        xml_content = run_header_to_xml(header_file, 'Nested')
        
        # Check for anonymous struct
        if 'name="unnamed"' not in xml_content:
            print("FAILED: Anonymous struct not found")
            return False
            
        # Check for named inner struct
        if 'name="inner"' not in xml_content:
            print("FAILED: Named inner struct not found")
            return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def test_unions():
    """Test: Union、無名union"""
    print("TEST: Unions and anonymous unions ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

struct WithUnion {
    uint32_t type;
    union {
        uint32_t i;
        float f;
        uint8_t bytes[4];
    };  // Anonymous union
    union Named {
        uint64_t val;
        uint8_t data[8];
    } named;
};
""")
        header_file = f.name
    
    try:
        xml_content = run_header_to_xml(header_file, 'WithUnion')
        
        # Check for union tags
        if '<union>' not in xml_content:
            print("FAILED: Union not found")
            return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def test_complex_nesting():
    """Test: 複雑にネストする構造"""
    print("TEST: Complex nested structures ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

struct Complex {
    struct {
        union {
            struct {
                uint16_t a;
                uint16_t b;
            } pair;
            uint32_t value;
        };
        uint8_t flag;
    } nested;
    union {
        struct {
            uint8_t type;
            uint8_t data[3];
        } typed;
        uint32_t raw;
    } data;
};
""")
        header_file = f.name
    
    try:
        xml_content = run_header_to_xml(header_file, 'Complex')
        
        # Should have nested structures and unions
        struct_count = xml_content.count('<struct>')
        union_count = xml_content.count('<union>')
        
        if struct_count < 3 or union_count < 2:
            print(f"FAILED: Expected complex nesting, got {struct_count} structs, {union_count} unions")
            return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def test_bitfields():
    """Test: 複雑なbitfield"""
    print("TEST: Complex bitfields ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

struct Bitfields {
    uint32_t flag1 : 1;
    uint32_t value : 7;
    uint32_t flag2 : 1;
    uint32_t : 7;  // Unnamed bitfield
    uint32_t data : 16;
    
    uint8_t small : 3;
    uint8_t tiny : 2;
    uint8_t bit : 1;
};
""")
        header_file = f.name
    
    try:
        xml_content = run_header_to_xml(header_file, 'Bitfields')
        
        # Check for bits attribute
        if 'bits="1"' not in xml_content:
            print("FAILED: Bitfield bits attribute not found")
            return False
            
        # Check for bit_offset attribute
        if 'bit_offset=' not in xml_content:
            print("FAILED: Bitfield bit_offset not found")
            return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def test_arrays():
    """Test: 配列、マクロ展開"""
    print("TEST: Arrays with macro expansion ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

#define SIZE 32
#define COUNT 8
#define TOTAL (SIZE * COUNT)

struct Arrays {
    uint8_t buffer[SIZE];
    uint32_t values[COUNT];
    char data[TOTAL];
};
""")
        header_file = f.name
    
    try:
        xml_content = run_header_to_xml(header_file, 'Arrays')
        
        # Check macro expansion
        if 'array_size="32"' not in xml_content:
            print("FAILED: SIZE macro not expanded")
            return False
            
        if 'array_size="8"' not in xml_content:
            print("FAILED: COUNT macro not expanded")
            return False
            
        if 'array_size="256"' not in xml_content:
            print("FAILED: TOTAL macro not expanded")
            return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def test_offset_and_size():
    """Test: XMLにoffsetとsizeが記載される"""
    print("TEST: Offset and size in XML ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

struct OffsetTest {
    uint32_t first;   // offset 0, size 4
    uint16_t second;  // offset 4, size 2
    uint8_t third;    // offset 6, size 1
    uint8_t fourth;   // offset 7, size 1
    uint64_t fifth;   // offset 8 (aligned), size 8
};
""")
        header_file = f.name
    
    try:
        xml_content = run_header_to_xml(header_file, 'OffsetTest')
        root = ET.fromstring(xml_content)
        
        # Check offsets
        fields = root.findall('.//field')
        expected_offsets = [0, 4, 6, 7, 8]
        
        for i, field in enumerate(fields):
            if 'type' in field.attrib:  # Skip non-basic fields
                offset = int(field.get('offset', -1))
                if i < len(expected_offsets) and offset != expected_offsets[i]:
                    print(f"FAILED: Expected offset {expected_offsets[i]}, got {offset}")
                    return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def test_pack_option():
    """Test: packオプション（アライメント制御）"""
    print("TEST: Pack option (alignment control) ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

struct PackTest {
    uint8_t a;
    uint32_t b;
    uint16_t c;
};
""")
        header_file = f.name
    
    try:
        # Without packing
        xml_unpacked = run_header_to_xml(header_file, 'PackTest')
        root_unpacked = ET.fromstring(xml_unpacked)
        size_unpacked = int(root_unpacked.get('size'))
        
        # With packing
        xml_packed = run_header_to_xml(header_file, 'PackTest', packed=True)
        root_packed = ET.fromstring(xml_packed)
        size_packed = int(root_packed.get('size'))
        
        # Packed size should be smaller (1+4+2=7 vs aligned 12)
        if size_packed >= size_unpacked:
            print(f"FAILED: Packed size {size_packed} >= unpacked size {size_unpacked}")
            return False
            
        if root_packed.get('packed') != 'true':
            print("FAILED: packed attribute not set")
            return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def test_output_file_option():
    """Test: 出力XMLファイル名の指定"""
    print("TEST: Output file option ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>
struct Test { uint32_t value; };
""")
        header_file = f.name
    
    output_file = None
    try:
        output_file = tempfile.mktemp(suffix='_custom.xml')
        run_header_to_xml(header_file, 'Test', output=output_file)
        
        if not os.path.exists(output_file):
            print("FAILED: Output file not created")
            return False
            
        with open(output_file, 'r') as f:
            content = f.read()
            if '<struct name="Test"' not in content:
                print("FAILED: Invalid output file content")
                return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)
        if output_file and os.path.exists(output_file):
            os.unlink(output_file)

def test_endianness():
    """Test: エンディアン対応（little endianデフォルト）"""
    print("TEST: Endianness support ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>
struct EndianTest {
    uint32_t value;
    uint16_t short_val;
};
""")
        header_file = f.name
    
    xml_file = None
    bin_file = None
    try:
        # Generate XML
        xml_file = tempfile.mktemp(suffix='.xml')
        run_header_to_xml(header_file, 'EndianTest', output=xml_file)
        
        # Create test binary (little endian)
        with tempfile.NamedTemporaryFile(mode='wb', delete=False) as f:
            # Need to account for padding - struct is 8 bytes total
            f.write(struct.pack('<IHxx', 0x12345678, 0xABCD))
            bin_file = f.name
        
        # Parse with default (little endian)
        output_le = run_binary_parser(xml_file, bin_file, 'EndianTest')
        
        # Parse with big endian
        output_be = run_binary_parser(xml_file, bin_file, 'EndianTest', big_endian=True)
        
        # Check values are different
        if '305419896' not in output_le:  # 0x12345678 in decimal
            print("FAILED: Little endian parsing incorrect")
            return False
            
        if '305419896' in output_be:  # Should be different in big endian
            print("FAILED: Big endian parsing same as little endian")
            return False
        
        print("PASSED")
        return True
    finally:
        for f in [header_file, xml_file, bin_file]:
            if f and os.path.exists(f):
                os.unlink(f)

def test_multiple_headers():
    """Test: 複数ヘッダファイルの#include対応"""
    print("TEST: Multiple header files with #include ... ", end='')
    
    # Create included header
    with tempfile.NamedTemporaryFile(mode='w', suffix='_types.h', delete=False) as f:
        f.write("""
#pragma once
#include <stdint.h>

typedef struct {
    uint16_t x;
    uint16_t y;
} Point;

struct Color {
    uint8_t r, g, b, a;
};
""")
        types_header = f.name
    
    # Create main header
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False, 
                                     dir=os.path.dirname(types_header)) as f:
        f.write(f"""
#include <stdint.h>
#include "{os.path.basename(types_header)}"

struct Graphics {{
    Point position;
    struct Color color;
    uint32_t flags;
}};
""")
        main_header = f.name
    
    try:
        xml_content = run_header_to_xml(main_header, 'Graphics')
        
        # Check that Point typedef was resolved
        if 'name="position"' not in xml_content:
            print("FAILED: Point typedef not resolved")
            return False
            
        # Check that Color struct was resolved
        if 'name="color"' not in xml_content:
            print("FAILED: Color struct not resolved")
            return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(types_header)
        os.unlink(main_header)

def test_typedef_support():
    """Test: typedef構造体とunion"""
    print("TEST: Typedef struct and union ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

typedef struct {
    uint32_t id;
    char name[16];
} TypedefStruct;

typedef union {
    uint32_t dword;
    uint16_t words[2];
    uint8_t bytes[4];
} TypedefUnion;

struct Container {
    TypedefStruct info;
    TypedefUnion data;
};
""")
        header_file = f.name
    
    try:
        xml_content = run_header_to_xml(header_file, 'Container')
        
        # Both typedefs should be expanded inline
        if 'name="info"' not in xml_content:
            print("FAILED: TypedefStruct not expanded")
            return False
            
        if 'name="data"' not in xml_content:
            print("FAILED: TypedefUnion not expanded")
            return False
            
        # Check for proper struct/union tags
        if xml_content.count('<struct>') < 2:
            print("FAILED: Typedef struct not properly expanded")
            return False
            
        if xml_content.count('<union>') < 1:
            print("FAILED: Typedef union not properly expanded")
            return False
        
        print("PASSED")
        return True
    finally:
        os.unlink(header_file)

def main():
    print("Running comprehensive requirements test...\n")
    
    tests = [
        test_basic_types,
        test_nested_structures,
        test_unions,
        test_complex_nesting,
        test_bitfields,
        test_arrays,
        test_offset_and_size,
        test_pack_option,
        test_output_file_option,
        test_endianness,
        test_multiple_headers,
        test_typedef_support,
    ]
    
    passed = 0
    total = len(tests)
    
    for test in tests:
        if test():
            passed += 1
    
    print(f"\n{passed}/{total} tests passed")
    
    if passed == total:
        print("\n✅ All requirements from やりたいこと.txt are implemented!")
        return 0
    else:
        print(f"\n❌ {total - passed} requirements still need work")
        return 1

if __name__ == '__main__':
    sys.exit(main())