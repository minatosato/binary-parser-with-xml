#!/usr/bin/env python3
import argparse
import sys
import os
from header_to_xml import HeaderToXMLConverter


def main():
    parser = argparse.ArgumentParser(
        description='Convert C++ header struct definitions to XML'
    )
    parser.add_argument('header_file', help='Input C++ header file')
    parser.add_argument('struct_name', help='Name of the root struct to convert')
    parser.add_argument('-o', '--output', default=None,
                        help='Output XML file (default: <struct_name>.xml)')
    parser.add_argument('-p', '--packed', action='store_true',
                        help='Mark struct as packed (no alignment)')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.header_file):
        print(f"Error: Header file '{args.header_file}' not found", file=sys.stderr)
        sys.exit(1)
    
    output_file = args.output or f"{args.struct_name}.xml"
    
    try:
        converter = HeaderToXMLConverter()
        xml_content = converter.convert(args.header_file, args.struct_name, args.packed)
        
        with open(output_file, 'w') as f:
            f.write(xml_content)
        
        print(f"Successfully converted '{args.struct_name}' to '{output_file}'")
        
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()