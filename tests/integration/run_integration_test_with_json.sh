#!/bin/bash
set -e

echo "=== Integration Test with JSON Output ==="
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

# Step 3: Parse binary data using XML definition (text output)
echo "Step 3: Parsing binary data with XML definition (text output)..."
cd ../..
if [ -f "build/parse_binary" ]; then
    echo "--- Text output ---"
    ./build/parse_binary tests/integration/test_data.xml tests/integration/test_data.bin
    echo
else
    echo "Error: parse_binary not built. Please build the C++ project first."
    echo "Run: mkdir build && cd build && cmake .. && make"
    exit 1
fi

# Step 4: Test JSON output
echo "Step 4: Testing JSON output..."
echo "--- JSON output (compact) ---"
./build/parse_binary tests/integration/test_data.xml tests/integration/test_data.bin --json
echo
echo

# Step 5: Test pretty JSON output
echo "Step 5: Testing pretty JSON output..."
echo "--- JSON output (pretty) ---"
./build/parse_binary tests/integration/test_data.xml tests/integration/test_data.bin --json --pretty
echo

# Step 6: Test JSON file output
echo "Step 6: Testing JSON file output..."
./build/parse_binary tests/integration/test_data.xml tests/integration/test_data.bin --json --pretty -o tests/integration/output.json
if [ -f "tests/integration/output.json" ]; then
    echo "✓ JSON file created successfully"
    echo "--- Contents of output.json ---"
    cat tests/integration/output.json
    rm tests/integration/output.json
else
    echo "✗ Failed to create JSON file"
    exit 1
fi

echo
echo "=== Integration test with JSON completed successfully ==="