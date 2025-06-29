#!/usr/bin/env python3
import os
import sys

def test_no_unnecessary_files():
    """Test that there are no unnecessary files in the project"""
    print("TEST: No unnecessary files ... ", end='')
    
    # Check for files that shouldn't be committed
    unnecessary_patterns = [
        '.DS_Store',
        '*.o',
        '*.a',
        '*.pyc',
        '__pycache__',
        '*.swp',
        '*~',
        'test_endianness',
        'test_signed',
        'test_char'
    ]
    
    found_files = []
    for root, dirs, files in os.walk('.'):
        # Skip .git and build directories
        if '.git' in root or 'build' in root:
            continue
            
        for file in files:
            for pattern in unnecessary_patterns:
                if pattern.startswith('*'):
                    if file.endswith(pattern[1:]):
                        found_files.append(os.path.join(root, file))
                elif pattern.endswith('*'):
                    if file.startswith(pattern[:-1]):
                        found_files.append(os.path.join(root, file))
                elif file == pattern:
                    found_files.append(os.path.join(root, file))
    
    if found_files:
        print(f"FAILED: Found unnecessary files: {found_files}")
        return False
    
    print("PASSED")
    return True

def test_directory_structure():
    """Test that directory structure is clean and organized"""
    print("TEST: Directory structure ... ", end='')
    
    expected_dirs = [
        'src',
        'src/binary_parser',
        'src/header_to_xml',
        'tests',
        'tests/integration',
        'tests/integration/extreme',
        'build'
    ]
    
    for dir_path in expected_dirs:
        if not os.path.isdir(dir_path):
            print(f"FAILED: Missing directory: {dir_path}")
            return False
    
    print("PASSED")
    return True

if __name__ == '__main__':
    print("Running project structure tests...")
    
    all_passed = True
    all_passed &= test_no_unnecessary_files()
    all_passed &= test_directory_structure()
    
    if not all_passed:
        sys.exit(1)
    
    print("All tests passed!")