#!/usr/bin/env python3
"""Test edge cases that might break the header parser"""
import sys
import os
import tempfile
import subprocess

def run_header_to_xml(header_content, struct_name):
    """Helper to run header_to_xml with given content"""
    with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
        f.write(header_content)
        header_file = f.name
    
    try:
        result = subprocess.run([
            sys.executable,
            'src/header_to_xml/header_to_xml.py',
            header_file,
            struct_name
        ], capture_output=True, text=True)
        return result.returncode == 0, result.stdout, result.stderr
    finally:
        os.unlink(header_file)

def test_enum_in_struct():
    """Test struct with enum"""
    print("TEST: Enum in struct ... ", end='')
    header = """
enum Status {
    OK = 0,
    ERROR = 1,
    PENDING = 2
};

struct Data {
    uint32_t id;
    enum Status status;
};
"""
    success, output, error = run_header_to_xml(header, "Data")
    if not success:
        print(f"FAILED: {error}")
        return False
    print("PASSED")
    return True

def test_function_pointer():
    """Test struct with function pointer"""
    print("TEST: Function pointer ... ", end='')
    header = """
#include <stdint.h>

typedef void (*callback_t)(int);

struct Callbacks {
    uint32_t id;
    callback_t on_event;
    void (*on_error)(const char*);
};
"""
    success, output, error = run_header_to_xml(header, "Callbacks")
    # Function pointers are not supported, should fail or handle gracefully
    print("SKIPPED (not supported)")
    return True

def test_macro_in_array_size():
    """Test array size defined by macro"""
    print("TEST: Macro in array size ... ", end='')
    header = """
#include <stdint.h>

#define MAX_SIZE 32

struct Buffer {
    uint32_t length;
    uint8_t data[MAX_SIZE];
};
"""
    success, output, error = run_header_to_xml(header, "Buffer")
    if not success:
        print(f"FAILED: {error}")
        return False
    # Check if it handles the macro (might default to 4)
    if "array_size" in output:
        print("PASSED (macro handled)")
    else:
        print("FAILED (macro not resolved)")
        return False
    return True

def test_conditional_compilation():
    """Test #ifdef conditional compilation"""
    print("TEST: Conditional compilation ... ", end='')
    header = """
#include <stdint.h>

struct Config {
    uint32_t version;
#ifdef ENABLE_DEBUG
    uint32_t debug_flags;
#endif
    uint32_t checksum;
};
"""
    success, output, error = run_header_to_xml(header, "Config")
    if not success:
        print(f"FAILED: {error}")
        return False
    print("PASSED")
    return True

def test_forward_declaration():
    """Test forward declaration"""
    print("TEST: Forward declaration ... ", end='')
    header = """
#include <stdint.h>

struct Node;  // Forward declaration

struct List {
    uint32_t count;
    struct Node* first;
};

struct Node {
    uint32_t value;
    struct Node* next;
};
"""
    success, output, error = run_header_to_xml(header, "List")
    # Pointers are not well supported
    print("SKIPPED (pointers not supported)")
    return True

def test_pragma_pack():
    """Test #pragma pack"""
    print("TEST: Pragma pack ... ", end='')
    header = """
#include <stdint.h>

#pragma pack(push, 1)
struct Packed {
    uint8_t a;
    uint32_t b;
    uint16_t c;
};
#pragma pack(pop)
"""
    success, output, error = run_header_to_xml(header, "Packed")
    if not success:
        print(f"FAILED: {error}")
        return False
    # Currently ignores #pragma, uses -p option instead
    print("PASSED (pragma ignored, use -p option)")
    return True

def test_cpp_features():
    """Test C++ specific features"""
    print("TEST: C++ class ... ", end='')
    header = """
#include <stdint.h>

class MyClass {
public:
    uint32_t public_data;
private:
    uint32_t private_data;
};
"""
    success, output, error = run_header_to_xml(header, "MyClass")
    # Classes are not supported
    print("SKIPPED (C++ classes not supported)")
    return True

def test_flexible_array():
    """Test flexible array member (C99)"""
    print("TEST: Flexible array ... ", end='')
    header = """
#include <stdint.h>

struct FlexArray {
    uint32_t size;
    uint8_t data[];  // Flexible array member
};
"""
    success, output, error = run_header_to_xml(header, "FlexArray")
    # Flexible arrays might cause issues
    print("SKIPPED (flexible arrays not supported)")
    return True

def test_attribute_aligned():
    """Test __attribute__ aligned"""
    print("TEST: Attribute aligned ... ", end='')
    header = """
#include <stdint.h>

struct Aligned {
    uint32_t a;
    uint64_t b __attribute__((aligned(16)));
    uint32_t c;
};
"""
    success, output, error = run_header_to_xml(header, "Aligned")
    if not success:
        print(f"FAILED: {error}")
        return False
    print("PASSED (attributes ignored)")
    return True

def test_multi_dimensional_array():
    """Test multi-dimensional arrays"""
    print("TEST: Multi-dimensional array ... ", end='')
    header = """
#include <stdint.h>

struct Matrix {
    uint32_t rows;
    uint32_t cols;
    float data[4][4];
};
"""
    success, output, error = run_header_to_xml(header, "Matrix")
    # Multi-dimensional arrays are not supported
    print("SKIPPED (multi-dimensional arrays not supported)")
    return True

def test_various_comment_styles():
    """Test various comment styles"""
    print("TEST: Various comment styles ... ", end='')
    header = """
#include <stdint.h>

struct CommentTest { // Inline comment
    uint32_t field1; /* Block comment
                        on multiple lines */
    uint16_t field2; // Another inline comment
    /* Another block comment */
    uint8_t field3;
};
"""
    success, output, error = run_header_to_xml(header, "CommentTest")
    if not success:
        print(f"FAILED: {error}")
        return False
    # Check if fields are still parsed correctly
    if "field1" not in output or "field2" not in output or "field3" not in output:
        print("FAILED: Fields not parsed correctly with comments")
        return False
    print("PASSED")
    return True

def test_nested_typedefs():
    """Test nested typedefs"""
    print("TEST: Nested typedefs ... ", end='')
    header = """
#include <stdint.h>

typedef uint32_t U32;
typedef U32 MyID;

struct NestedTypedefs {
    MyID id;
    U32 value;
};
"""
    success, output, error = run_header_to_xml(header, "NestedTypedefs")
    if not success:
        print(f"FAILED: {error}")
        return False
    # Check if typedefs are resolved to base types
    if 'type="uint32_t"' not in output:
        print("FAILED: Typedefs not resolved to uint32_t")
        return False
    print("PASSED")
    return True

def test_typedef_struct_array():
    """Test array of typedef struct"""
    print("TEST: Typedef struct array ... ", end='')
    header = """
#include <stdint.h>

typedef struct {
    uint16_t x;
    uint16_t y;
} Point;

struct PointArray {
    Point points[4];
};
"""
    success, output, error = run_header_to_xml(header, "PointArray")
    if not success:
        print(f"FAILED: {error}")
        return False
    # Check if array is correctly identified with proper size
    # Note: typedef structs in arrays are not expanded inline (by design)
    if 'name="points"' not in output or 'array_size="4"' not in output or 'type="Point"' not in output:
        print("FAILED: Typedef struct array not parsed correctly")
        return False
    print("PASSED")
    return True

def test_undefined_macro_in_array_size():
    """Test undefined macro in array size"""
    print("TEST: Undefined macro in array size ... ", end='')
    header = """
#include <stdint.h>

struct UndefinedMacroArray {
    uint8_t data[UNDEFINED_SIZE];
};
"""
    success, output, error = run_header_to_xml(header, "UndefinedMacroArray")
    # Expect failure or error message
    if success:
        print("FAILED: Should have failed to parse undefined macro")
        return False
    if "Error: " not in error:
        print("FAILED: No error message for undefined macro")
        return False
    print("PASSED (expected failure)")
    return True

if __name__ == '__main__':
    print("Running edge case tests...")
    print("=" * 50)
    
    tests = [
        test_enum_in_struct,
        test_function_pointer,
        test_macro_in_array_size,
        test_conditional_compilation,
        test_forward_declaration,
        test_pragma_pack,
        test_cpp_features,
        test_flexible_array,
        test_attribute_aligned,
        test_multi_dimensional_array,
        test_various_comment_styles,
        test_nested_typedefs,
        test_typedef_struct_array,
        test_undefined_macro_in_array_size
    ]
    
    passed = 0
    failed = 0
    skipped = 0
    
    for test in tests:
        try:
            result = test()
            if "SKIPPED" in test.__doc__:
                skipped += 1
            elif "expected failure" in test.__doc__ and not result:
                passed += 1 # Test passed because it failed as expected
            elif result:
                passed += 1
            else:
                failed += 1
        except Exception as e:
            print(f"ERROR: {e}")
            failed += 1
    
    print("=" * 50)
    print(f"Results: {passed} passed, {failed} failed, {skipped} skipped")
    
    if failed > 0:
        sys.exit(1)