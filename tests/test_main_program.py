#!/usr/bin/env python3
import sys
import os
import tempfile
import subprocess

# Test the main binary parser executable
def test_main_parser_program():
    print("TEST: Main parser executable ... ", end='')
    
    # Create test header
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
struct SimpleStruct {
    uint32_t magic;
    uint16_t version;
    uint16_t flags;
};
""")
        header_file = f.name
    
    # Create test binary data
    with tempfile.NamedTemporaryFile(mode='wb', suffix='.bin', delete=False) as f:
        # magic=0xDEADBEEF, version=0x0102, flags=0x8000 (little endian)
        f.write(b'\xEF\xBE\xAD\xDE\x02\x01\x00\x80')
        bin_file = f.name
    
    # Create XML
    with tempfile.NamedTemporaryFile(mode='w', suffix='.xml', delete=False) as f:
        xml_file = f.name
    
    try:
        # Generate XML
        result = subprocess.run([
            sys.executable,
            'src/header_to_xml/header_to_xml.py',
            header_file,
            'SimpleStruct',
            '-o', xml_file
        ], capture_output=True, text=True)
        assert result.returncode == 0
        
        # Check if parse_binary executable exists
        parse_binary = './build/parse_binary'
        if not os.path.exists(parse_binary):
            parse_binary = './parse_binary'
        
        if not os.path.exists(parse_binary):
            raise FileNotFoundError("parse_binary executable not found")
        
        # Run parser
        result = subprocess.run([
            parse_binary,
            xml_file,
            bin_file,
            'SimpleStruct'
        ], capture_output=True, text=True)
        
        assert result.returncode == 0, f"Parser failed: {result.stderr}"
        
        # Check output
        output = result.stdout
        assert 'magic' in output
        assert 'deadbeef' in output.lower() or '0xdeadbeef' in output.lower()
        
        print("PASSED")
        
    finally:
        # Cleanup
        for f in [header_file, bin_file, xml_file]:
            if os.path.exists(f):
                os.unlink(f)

if __name__ == '__main__':
    print("Running main program tests...")
    
    try:
        test_main_parser_program()
    except Exception as e:
        print(f"FAILED: {e}")
        sys.exit(1)
    
    print("All tests passed!")