#!/usr/bin/env python3
"""Integration test for macro expansion in complete workflow"""
import sys
import os
import tempfile
import subprocess
import struct

def test_macro_workflow():
    print("TEST: Complete workflow with macros ... ", end='')
    
    # Create header with macros
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

#define VERSION_MAJOR 1
#define VERSION_MINOR 2
#define VERSION_PATCH 3

#define MAX_NAME_LEN 32
#define MAX_ITEMS 16
#define ITEM_SIZE (sizeof(uint32_t))
#define BUFFER_SIZE (MAX_ITEMS * 4)

struct Config {
    struct {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
        uint8_t reserved;
    } version;
    char name[MAX_NAME_LEN];
    uint32_t items[MAX_ITEMS];
    uint8_t buffer[BUFFER_SIZE];
};
""")
        header_file = f.name
    
    # Create binary data
    with tempfile.NamedTemporaryFile(mode='wb', suffix='.bin', delete=False) as f:
        # version: 1.2.3.0
        f.write(struct.pack('<4B', 1, 2, 3, 0))
        
        # name: "TestConfig" + padding
        name_data = b'TestConfig\x00'
        f.write(name_data.ljust(32, b'\x00'))
        
        # items: [100, 200, 300, ...] (16 values)
        for i in range(16):
            f.write(struct.pack('<I', (i + 1) * 100))
        
        # buffer: 64 bytes of data
        f.write(bytes(range(64)))
        
        bin_file = f.name
    
    xml_file = None
    try:
        # Generate XML
        xml_file = tempfile.mktemp(suffix='.xml')
        result = subprocess.run([
            sys.executable,
            '../../src/header_to_xml/header_to_xml.py',
            header_file,
            'Config',
            '-o', xml_file
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"FAILED: XML generation failed: {result.stderr}")
            return False
        
        # Check XML has expanded macros
        with open(xml_file, 'r') as f:
            xml_content = f.read()
            if 'array_size="32"' not in xml_content:
                print("FAILED: MAX_NAME_LEN not expanded")
                return False
            if 'array_size="16"' not in xml_content:
                print("FAILED: MAX_ITEMS not expanded")
                return False
            if 'array_size="64"' not in xml_content:
                print("FAILED: BUFFER_SIZE not expanded")
                return False
        
        # Parse binary
        result = subprocess.run([
            '../../build/parse_binary',
            xml_file,
            bin_file,
            'Config'
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"FAILED: Binary parsing failed: {result.stderr}")
            return False
        
        output = result.stdout
        
        # Verify output
        if "'T' 'e' 's' 't' 'C' 'o' 'n' 'f' 'i' 'g'" not in output:
            print("FAILED: name not found in output")
            print(f"Output was: {output}")
            return False
        
        if '100' not in output:
            print("FAILED: items data not found")
            return False
        
        print("PASSED")
        return True
        
    finally:
        # Cleanup
        for f in [header_file, bin_file, xml_file]:
            if f and os.path.exists(f):
                os.unlink(f)

if __name__ == '__main__':
    print("Running macro integration test...")
    
    if not test_macro_workflow():
        sys.exit(1)
    
    print("Integration test passed!")