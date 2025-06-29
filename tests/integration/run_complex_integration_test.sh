#!/bin/bash
set -e

echo "=== Complex Integration Test with JSON Output ==="
echo

# Step 1: Convert complex header to XML
echo "Step 1: Converting complex_test_data.h to XML..."
cd ../..
python src/header_to_xml/main.py tests/integration/complex_test_data.h ComplexTestData -o tests/integration/complex_test_data.xml
echo "✓ Complex XML generated"
echo

# Step 2: Create complex test binary data
echo "Step 2: Creating complex test binary data..."
cd tests/integration
g++ -o create_complex_test_binary create_complex_test_binary.cpp
./create_complex_test_binary
echo "✓ Complex binary data created"
echo

# Step 3: Parse complex binary data with JSON output
echo "Step 3: Parsing complex binary data with JSON output..."
cd ../..
echo "--- JSON output (pretty) ---"
./build/parse_binary tests/integration/complex_test_data.xml tests/integration/complex_test_data.bin --json --pretty
echo

# Step 4: Save to file for verification
echo "Step 4: Saving JSON output to file..."
./build/parse_binary tests/integration/complex_test_data.xml tests/integration/complex_test_data.bin --json --pretty -o tests/integration/complex_output.json
echo "✓ JSON file created"

# Step 5: Verify key features
echo "Step 5: Verifying JSON features..."
echo "- Checking char array converted to string..."
grep -q '"name": "Test Object"' tests/integration/complex_output.json && echo "  ✓ Char array to string conversion working"

echo "- Checking nested structures..."
grep -q '"position": {' tests/integration/complex_output.json && echo "  ✓ Nested structures working"

echo "- Checking arrays..."
grep -q '"values": \[' tests/integration/complex_output.json && echo "  ✓ Arrays working"

echo "- Checking struct arrays..."
grep -q '"items": \[' tests/integration/complex_output.json && echo "  ✓ Struct arrays working"

# Cleanup
rm tests/integration/complex_output.json

echo
echo "=== Complex integration test completed successfully ==="