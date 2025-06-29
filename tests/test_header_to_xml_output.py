#!/usr/bin/env python3
import sys
import os
import tempfile
import subprocess

# Test that header_to_xml.py can output to custom filename
def test_custom_output_filename():
    print("TEST: Custom output filename ... ", end='')
    
    # Create a test header file
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
struct TestStruct {
    uint32_t value;
};
""")
        header_file = f.name
    
    # Create custom output filename
    custom_output = tempfile.mktemp(suffix='_custom.xml')
    
    try:
        # Run header_to_xml.py with custom output option
        result = subprocess.run([
            sys.executable,
            'src/header_to_xml/header_to_xml.py',
            header_file,
            'TestStruct',
            '-o', custom_output
        ], capture_output=True, text=True)
        
        # Check if command succeeded
        assert result.returncode == 0, f"Command failed: {result.stderr}"
        
        # Check if custom output file was created
        assert os.path.exists(custom_output), f"Output file {custom_output} was not created"
        
        # Check if content is valid XML
        with open(custom_output, 'r') as f:
            content = f.read()
            assert '<?xml version' in content
            assert '<struct name="TestStruct"' in content
        
        print("PASSED")
        
    finally:
        # Cleanup
        if os.path.exists(header_file):
            os.unlink(header_file)
        if os.path.exists(custom_output):
            os.unlink(custom_output)

if __name__ == '__main__':
    print("Running header_to_xml output tests...")
    
    try:
        test_custom_output_filename()
    except Exception as e:
        print(f"FAILED: {e}")
        sys.exit(1)
    
    print("All tests passed!")