#!/usr/bin/env python3
"""Test macro expansion in header_to_xml"""
import sys
import os
import tempfile
import subprocess

def test_simple_macro_expansion():
    print("TEST: Simple macro expansion ... ", end='')
    
    # Create test header with macros
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

#define BUFFER_SIZE 256
#define MAX_ITEMS 32

struct TestStruct {
    uint32_t count;
    uint8_t buffer[BUFFER_SIZE];
    uint32_t items[MAX_ITEMS];
};
""")
        header_file = f.name
    
    try:
        # Run header_to_xml.py
        result = subprocess.run([
            sys.executable,
            'src/header_to_xml/header_to_xml.py',
            header_file,
            'TestStruct'
        ], capture_output=True, text=True)
        
        # Check if command succeeded
        if result.returncode != 0:
            print(f"FAILED: {result.stderr}")
            return False
        
        xml_output = result.stdout
        
        # Check for expanded array sizes
        if 'array_size="256"' not in xml_output:
            print("FAILED: BUFFER_SIZE not expanded to 256")
            return False
            
        if 'array_size="32"' not in xml_output:
            print("FAILED: MAX_ITEMS not expanded to 32")
            return False
        
        print("PASSED")
        return True
        
    finally:
        # Cleanup
        if os.path.exists(header_file):
            os.unlink(header_file)

def test_macro_arithmetic():
    print("TEST: Macro arithmetic expansion ... ", end='')
    
    # Create test header with arithmetic macros
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

#define BASE_SIZE 16
#define DOUBLE_SIZE (BASE_SIZE * 2)
#define TOTAL_SIZE (BASE_SIZE + DOUBLE_SIZE)

struct TestStruct {
    uint8_t base[BASE_SIZE];
    uint8_t doubled[DOUBLE_SIZE];
    uint8_t total[TOTAL_SIZE];
};
""")
        header_file = f.name
    
    try:
        result = subprocess.run([
            sys.executable,
            'src/header_to_xml/header_to_xml.py',
            header_file,
            'TestStruct'
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"FAILED: {result.stderr}")
            return False
        
        xml_output = result.stdout
        
        # Check for calculated sizes
        if 'array_size="16"' not in xml_output:
            print("FAILED: BASE_SIZE not expanded to 16")
            return False
            
        if 'array_size="32"' not in xml_output:
            print("FAILED: DOUBLE_SIZE not expanded to 32")
            return False
            
        if 'array_size="48"' not in xml_output:
            print("FAILED: TOTAL_SIZE not expanded to 48")
            return False
        
        print("PASSED")
        return True
        
    finally:
        if os.path.exists(header_file):
            os.unlink(header_file)

def test_nested_macros():
    print("TEST: Nested macro expansion ... ", end='')
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write("""
#include <stdint.h>

#define BLOCK_SIZE 8
#define NUM_BLOCKS 4
#define TOTAL_SIZE (BLOCK_SIZE * NUM_BLOCKS)

struct TestStruct {
    uint8_t data[TOTAL_SIZE];
};
""")
        header_file = f.name
    
    try:
        result = subprocess.run([
            sys.executable,
            'src/header_to_xml/header_to_xml.py',
            header_file,
            'TestStruct'
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"FAILED: {result.stderr}")
            return False
        
        xml_output = result.stdout
        
        if 'array_size="32"' not in xml_output:
            print("FAILED: TOTAL_SIZE not expanded to 32")
            return False
        
        print("PASSED")
        return True
        
    finally:
        if os.path.exists(header_file):
            os.unlink(header_file)

if __name__ == '__main__':
    print("Running macro expansion tests...")
    
    all_passed = True
    all_passed &= test_simple_macro_expansion()
    all_passed &= test_macro_arithmetic()
    all_passed &= test_nested_macros()
    
    if not all_passed:
        sys.exit(1)
    
    print("All tests passed!")