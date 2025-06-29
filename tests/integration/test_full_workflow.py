#!/usr/bin/env python3
"""Integration test for the complete workflow: header -> XML -> parse binary"""
import sys
import os
import tempfile
import subprocess
import struct

def test_complete_workflow():
    print("TEST: Complete workflow (header -> XML -> parse) ... ", end='')
    
    # Create complex header file
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

struct Header {
    uint32_t magic;
    uint16_t version;
    uint16_t flags : 4;
    uint16_t type : 4;
    uint16_t reserved : 8;
};

struct Data {
    Header header;
    char name[16];
    uint32_t values[4];
    union {
        uint64_t timestamp;
        struct {
            uint32_t date;
            uint32_t time;
        };
    };
};
""")
        header_file = f.name
    
    # Create binary data
    with tempfile.NamedTemporaryFile(mode='wb', suffix='.bin', delete=False) as f:
        # Header: magic=0xCAFEBABE, version=0x0102, flags=0xF, type=0x3, reserved=0
        f.write(struct.pack('<I', 0xCAFEBABE))  # magic
        f.write(struct.pack('<H', 0x0102))      # version
        f.write(struct.pack('<H', 0x003F))      # flags=15, type=3, reserved=0
        
        # name: "TestData" + padding
        f.write(b'TestData\x00\x00\x00\x00\x00\x00\x00\x00')
        
        # values: [100, 200, 300, 400]
        f.write(struct.pack('<4I', 100, 200, 300, 400))
        
        # timestamp/date+time: 0x0123456789ABCDEF
        f.write(struct.pack('<Q', 0x0123456789ABCDEF))
        
        bin_file = f.name
    
    xml_file = None
    try:
        # Generate XML
        xml_file = tempfile.mktemp(suffix='.xml')
        result = subprocess.run([
            sys.executable,
            'src/header_to_xml/header_to_xml.py',
            header_file,
            'Data',
            '-o', xml_file
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"FAILED: XML generation failed: {result.stderr}")
            return False
        
        # Parse binary
        result = subprocess.run([
            './build/parse_binary',
            xml_file,
            bin_file,
            'Data'
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"FAILED: Binary parsing failed: {result.stderr}")
            return False
        
        output = result.stdout.lower()
        
        # Verify output contains expected values
        checks = [
            ('cafebabe' in output, "magic value not found"),
            ('0102' in output or '258' in output, "version not found"),
            ('testdata' in output, "name not found"),
            ('100' in output, "values[0] not found"),
            ('0123456789abcdef' in output, "timestamp not found")
        ]
        
        for check, msg in checks:
            if not check:
                print(f"FAILED: {msg}")
                print(f"Output was: {result.stdout}")
                return False
        
        print("PASSED")
        return True
        
    finally:
        # Cleanup
        for f in [header_file, bin_file, xml_file]:
            if f and os.path.exists(f):
                os.unlink(f)

if __name__ == '__main__':
    print("Running full workflow integration test...")
    
    if not test_complete_workflow():
        sys.exit(1)
    
    print("Integration test passed!")