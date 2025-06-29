#!/bin/bash
set -e

echo "=== Integration Test ==="
echo

# Step 1: Convert header to XML
echo "Step 1: Converting test_data.h to XML..."
cd ../..
python src/header_to_xml/main.py tests/integration/test_data.h TestData -o tests/integration/test_data.xml
echo "✓ XML generated"
echo

# Step 2: Create test binary data
echo "Step 2: Creating test binary data..."
cd tests/integration
g++ -o create_test_binary create_test_binary.cpp
./create_test_binary
echo "✓ Binary data created"
echo

# Step 3: Parse binary data using XML definition
echo "Step 3: Parsing binary data with XML definition..."
cd ../..
if [ -f "build/parse_binary" ]; then
    ./build/parse_binary tests/integration/test_data.xml tests/integration/test_data.bin
else
    echo "Error: parse_binary not built. Please build the C++ project first."
    echo "Run: mkdir build && cd build && cmake .. && make"
    exit 1
fi

echo
echo "=== Integration test completed successfully ==="