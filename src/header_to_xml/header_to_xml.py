import re
import os
import sys
import argparse
import xml.etree.ElementTree as ET
from xml.dom import minidom


class HeaderToXMLConverter:
    def __init__(self):
        self.type_sizes = {
            'uint8_t': 1,
            'int8_t': 1,
            'uint16_t': 2,
            'int16_t': 2,
            'uint32_t': 4,
            'int32_t': 4,
            'uint64_t': 8,
            'int64_t': 8,
            'float': 4,
            'double': 8,
            'char': 1,
        }
        self.typedef_map = {}
        self.struct_map = {}
        self.processed_files = set()
    
    def convert(self, header_file, root_struct_name, packed=False):
        # Reset state for new conversion
        self.typedef_map = {}
        self.struct_map = {}
        self.processed_files = set()
        self._bitfield_state = {}
        
        # Process includes recursively
        self._process_header_file(header_file)
        
        # Find the struct definition  
        match = None
        struct_body = None
        
        # Try to find the struct in each file
        for content in self.struct_map.values():
            # Use a more robust regex that handles nested structures
            pattern = rf'struct\s+{root_struct_name}\s*{{'
            start_match = re.search(pattern, content)
            
            if start_match:
                # Find the matching closing brace
                start_pos = start_match.end() - 1
                brace_count = 1
                pos = start_pos + 1
                
                while pos < len(content) and brace_count > 0:
                    if content[pos] == '{':
                        brace_count += 1
                    elif content[pos] == '}':
                        brace_count -= 1
                    pos += 1
                
                if brace_count == 0:
                    struct_body = content[start_pos + 1:pos - 1]
                    match = True
                    break
        
        if not match or struct_body is None:
            raise ValueError(f"Struct '{root_struct_name}' not found in header file")
        
        root = ET.Element('struct', name=root_struct_name)
        if packed:
            root.set('packed', 'true')
        
        # Calculate offsets and sizes
        total_size = self._parse_struct_body(struct_body, root, 0, packed)
        root.set('size', str(total_size))
        
        return self._prettify(root)
    
    def _process_header_file(self, header_file):
        if header_file in self.processed_files:
            return
        
        self.processed_files.add(header_file)
        
        with open(header_file, 'r') as f:
            content = f.read()
        
        self.struct_map[header_file] = content
        
        # Extract typedefs - handle nested braces properly
        # First, find all typedef declarations
        typedef_pattern = r'typedef\s+(struct|union)\s*\{((?:[^{}]|(?:\{[^{}]*\}))*)\}\s*(\w+)\s*;'
        
        # Process the content to find typedefs with proper brace matching
        pos = 0
        while pos < len(content):
            # Find the next typedef
            typedef_match = re.search(r'typedef\s+(struct|union)\s*\{', content[pos:])
            if not typedef_match:
                break
                
            start_pos = pos + typedef_match.start()
            struct_or_union = typedef_match.group(1)
            
            # Find the matching closing brace
            brace_start = pos + typedef_match.end() - 1
            brace_count = 1
            current_pos = brace_start + 1
            
            while current_pos < len(content) and brace_count > 0:
                if content[current_pos] == '{':
                    brace_count += 1
                elif content[current_pos] == '}':
                    brace_count -= 1
                current_pos += 1
            
            if brace_count == 0:
                # Extract the body
                body = content[brace_start + 1:current_pos - 1]
                
                # Find the type name after the closing brace
                name_match = re.match(r'\s*(\w+)\s*;', content[current_pos:])
                if name_match:
                    type_name = name_match.group(1)
                    self.typedef_map[type_name] = (struct_or_union, body)
                    pos = current_pos + name_match.end()
                else:
                    pos = current_pos
            else:
                pos = start_pos + 1
        
        # Process includes
        include_pattern = r'#include\s*[\"<]([^\">]+)[\">]'
        for match in re.finditer(include_pattern, content):
            include_file = match.group(1)
            
            # Skip system headers
            if include_file.startswith('<') or not include_file.endswith('.h'):
                continue
            
            # Try to find the include file
            include_path = None
            base_dir = os.path.dirname(header_file)
            
            # Try relative path first
            potential_path = os.path.join(base_dir, include_file)
            if os.path.exists(potential_path):
                include_path = potential_path
            
            if include_path:
                self._process_header_file(include_path)
    
    def _parse_struct_body(self, body, parent_elem, current_offset=0, packed=False):
        lines = body.strip().split('\n')
        i = 0
        offset = current_offset
        last_was_bitfield = False
        
        while i < len(lines):
            line = lines[i].strip()
            
            if not line or line.startswith('//'):
                i += 1
                continue
            
            if 'union' in line:
                union_body, end_idx = self._extract_block(lines, i)
                field_name = self._extract_field_name(lines[end_idx])
                
                field_elem = ET.SubElement(parent_elem, 'field')
                field_elem.set('name', field_name)
                field_elem.set('offset', str(offset))
                
                union_elem = ET.SubElement(field_elem, 'union')
                union_size = self._parse_union_body(union_body, union_elem, offset, packed)
                field_elem.set('size', str(union_size))
                
                offset += union_size
                i = end_idx + 1
                
            elif 'struct' in line and '{' in line:
                struct_body, end_idx = self._extract_block(lines, i)
                field_name = self._extract_field_name(lines[end_idx])
                
                field_elem = ET.SubElement(parent_elem, 'field')
                field_elem.set('name', field_name)
                field_elem.set('offset', str(offset))
                
                struct_elem = ET.SubElement(field_elem, 'struct')
                struct_size = self._parse_struct_body(struct_body, struct_elem, 0, packed)
                field_elem.set('size', str(struct_size))
                
                # For anonymous structs, advance offset only if it has a name
                if field_name != 'unnamed':
                    offset += struct_size
                else:
                    # Anonymous struct members are at the parent level
                    offset += struct_size
                i = end_idx + 1
                
            else:
                # Check for bitfield
                bitfield_match = re.match(r'(\w+)\s+(\w+)\s*:\s*(\d+)\s*;', line)
                if bitfield_match:
                    field_type, field_name, bits = bitfield_match.groups()
                    field_elem = ET.SubElement(parent_elem, 'field')
                    field_elem.set('name', field_name)
                    field_elem.set('type', field_type)
                    field_elem.set('bits', bits)
                    
                    # Track bitfield offset within the current field type
                    if not hasattr(self, '_bitfield_state'):
                        self._bitfield_state = {}
                    
                    key = (offset, field_type)
                    if key not in self._bitfield_state:
                        self._bitfield_state[key] = {
                            'bit_offset': 0,
                            'base_offset': offset
                        }
                    
                    field_elem.set('bit_offset', str(self._bitfield_state[key]['bit_offset']))
                    field_elem.set('offset', str(self._bitfield_state[key]['base_offset']))
                    
                    # Update bit offset for next bitfield
                    self._bitfield_state[key]['bit_offset'] += int(bits)
                    
                    # Set size based on base type
                    type_size = self.type_sizes.get(field_type, 4)
                    field_elem.set('size', str(type_size))
                    last_was_bitfield = True
                else:
                    # Check if previous field was a bitfield and update offset
                    if last_was_bitfield and hasattr(self, '_bitfield_state'):
                        # Find the bitfield group that was just completed
                        max_offset = offset
                        for key, state in self._bitfield_state.items():
                            if key[0] <= offset:
                                type_size = self.type_sizes.get(key[1], 4)
                                max_offset = max(max_offset, key[0] + type_size)
                        offset = max_offset
                    last_was_bitfield = False
                    
                    # Check for array
                    array_match = re.match(r'(\w+)\s+(\w+)\[(\d+)\]\s*;', line)
                    if array_match:
                        field_type, field_name, array_size = array_match.groups()
                        field_elem = ET.SubElement(parent_elem, 'field')
                        field_elem.set('name', field_name)
                        field_elem.set('type', field_type)
                        field_elem.set('array_size', array_size)
                        field_elem.set('offset', str(offset))
                        
                        type_size = self.type_sizes.get(field_type, 4)
                        field_size = type_size * int(array_size)
                        field_elem.set('size', str(field_size))
                        
                        if not packed:
                            offset = self._align_offset(offset + field_size, type_size)
                        else:
                            offset += field_size
                    else:
                        # Regular field
                        field_match = re.match(r'(\w+)\s+(\w+)\s*;', line)
                        if field_match:
                            field_type, field_name = field_match.groups()
                            field_elem = ET.SubElement(parent_elem, 'field')
                            field_elem.set('name', field_name)
                            field_elem.set('offset', str(offset))
                            
                            # Check if it's a typedef
                            if field_type in self.typedef_map:
                                typedef_info = self.typedef_map[field_type]
                                if isinstance(typedef_info, tuple):
                                    typedef_type, typedef_body = typedef_info
                                    if typedef_type == 'struct':
                                        struct_elem = ET.SubElement(field_elem, 'struct')
                                        struct_size = self._parse_struct_body(typedef_body, struct_elem, 0, packed)
                                    else:  # union
                                        struct_elem = ET.SubElement(field_elem, 'union')
                                        struct_size = self._parse_union_body(typedef_body, struct_elem, offset, packed)
                                else:
                                    # Old format, assume struct
                                    struct_elem = ET.SubElement(field_elem, 'struct')
                                    struct_size = self._parse_struct_body(typedef_info, struct_elem, 0, packed)
                                field_elem.set('size', str(struct_size))
                                offset += struct_size
                            else:
                                # Check if it's a known struct type from includes
                                struct_found = False
                                for content in self.struct_map.values():
                                    # Look for struct definitions
                                    struct_pattern = rf'struct\s+{field_type}\s*{{'
                                    if re.search(struct_pattern, content):
                                        struct_found = True
                                        # Extract and parse the struct
                                        full_pattern = rf'struct\s+{field_type}\s*\{{([^\{{\}}]*(?:\{{[^\{{\}}]*\}}[^\{{\}}]*)*)\}}'
                                        match = re.search(full_pattern, content, re.DOTALL)
                                        if match:
                                            struct_elem = ET.SubElement(field_elem, 'struct')
                                            struct_size = self._parse_struct_body(match.group(1), struct_elem, 0, packed)
                                            field_elem.set('size', str(struct_size))
                                            offset += struct_size
                                        break
                                
                                if not struct_found:
                                    field_elem.set('type', field_type)
                                    type_size = self.type_sizes.get(field_type, 4)
                                    field_elem.set('size', str(type_size))
                                    
                                    if not packed:
                                        offset = self._align_offset(offset + type_size, type_size)
                                    else:
                                        offset += type_size
                
                i += 1
        
        # Align struct size
        if not packed and offset > current_offset:
            max_alignment = self._get_struct_alignment(parent_elem)
            offset = self._align_offset(offset, max_alignment)
        
        return offset - current_offset
    
    def _parse_union_body(self, body, union_elem, current_offset=0, packed=False):
        max_size = 0
        lines = body.strip().split('\n')
        i = 0
        
        while i < len(lines):
            line = lines[i].strip()
            if not line or line.startswith('//'):
                i += 1
                continue
            
            # Check for nested struct in union
            if 'struct' in line and '{' in line:
                struct_body, end_idx = self._extract_block(lines, i)
                field_name = self._extract_field_name(lines[end_idx])
                
                field_elem = ET.SubElement(union_elem, 'field')
                field_elem.set('name', field_name)
                field_elem.set('offset', str(0))
                
                struct_elem = ET.SubElement(field_elem, 'struct')
                struct_size = self._parse_struct_body(struct_body, struct_elem, 0, packed)
                field_elem.set('size', str(struct_size))
                
                max_size = max(max_size, struct_size)
                i = end_idx + 1
                continue
            
            # Check for array in union
            array_match = re.match(r'(\w+)\s+(\w+)\[(\d+)\]\s*;', line)
            if array_match:
                field_type, field_name, array_size = array_match.groups()
                field_elem = ET.SubElement(union_elem, 'field')
                field_elem.set('name', field_name)
                field_elem.set('array_size', array_size)
                field_elem.set('offset', str(0))  # Union fields are relative to union start
                
                # Check if array element is typedef
                if field_type in self.typedef_map:
                    typedef_info = self.typedef_map[field_type]
                    if isinstance(typedef_info, tuple):
                        typedef_type, typedef_body = typedef_info
                        # For arrays of typedefs, we need to get the size once
                        temp_elem = ET.Element('temp')
                        if typedef_type == 'struct':
                            element_size = self._parse_struct_body(typedef_body, temp_elem, 0, packed)
                        else:
                            element_size = self._parse_union_body(typedef_body, temp_elem, 0, packed)
                    else:
                        temp_elem = ET.Element('temp')
                        element_size = self._parse_struct_body(typedef_info, temp_elem, 0, packed)
                    # Don't expand typedef arrays - just set type
                    field_elem.set('type', field_type)
                    field_size = element_size * int(array_size)
                else:
                    field_elem.set('type', field_type)
                    type_size = self.type_sizes.get(field_type, 4)
                    field_size = type_size * int(array_size)
                
                field_elem.set('size', str(field_size))
                max_size = max(max_size, field_size)
                continue
            
            field_match = re.match(r'(\w+)\s+(\w+)\s*;', line)
            if field_match:
                field_type, field_name = field_match.groups()
                field_elem = ET.SubElement(union_elem, 'field')
                field_elem.set('name', field_name)
                field_elem.set('offset', str(0))  # Union fields are relative to union start
                
                # Check if it's a typedef or known struct
                if field_type in self.typedef_map:
                    typedef_info = self.typedef_map[field_type]
                    if isinstance(typedef_info, tuple):
                        typedef_type, typedef_body = typedef_info
                        if typedef_type == 'struct':
                            struct_elem = ET.SubElement(field_elem, 'struct')
                            struct_size = self._parse_struct_body(typedef_body, struct_elem, 0, packed)
                        else:  # union
                            struct_elem = ET.SubElement(field_elem, 'union')
                            struct_size = self._parse_union_body(typedef_body, struct_elem, current_offset, packed)
                    else:
                        # Old format, assume struct
                        struct_elem = ET.SubElement(field_elem, 'struct')
                        struct_size = self._parse_struct_body(typedef_info, struct_elem, 0, packed)
                    field_elem.set('size', str(struct_size))
                    max_size = max(max_size, struct_size)
                else:
                    field_elem.set('type', field_type)
                    type_size = self.type_sizes.get(field_type, 4)
                    field_elem.set('size', str(type_size))
                    max_size = max(max_size, type_size)
            
            i += 1
        
        return max_size
    
    def _get_struct_alignment(self, struct_elem):
        max_alignment = 1
        for field in struct_elem.findall('.//field[@type]'):
            field_type = field.get('type')
            if field_type in self.type_sizes:
                max_alignment = max(max_alignment, self.type_sizes[field_type])
        return max_alignment
    
    def _align_offset(self, offset, alignment):
        if alignment == 0:
            return offset
        return ((offset + alignment - 1) // alignment) * alignment
    
    def _extract_block(self, lines, start_idx):
        brace_count = 0
        start_found = False
        body_lines = []
        
        for i in range(start_idx, len(lines)):
            line = lines[i]
            
            if '{' in line:
                brace_count += line.count('{')
                start_found = True
                if brace_count == 1:
                    continue
            
            if start_found and brace_count > 0:
                if '}' in line:
                    brace_count -= line.count('}')
                    if brace_count == 0:
                        return '\n'.join(body_lines), i
                else:
                    body_lines.append(line)
            elif start_found:
                body_lines.append(line)
        
        return '\n'.join(body_lines), len(lines) - 1
    
    def _extract_field_name(self, line):
        match = re.search(r'}\s*(\w*)\s*;', line)
        if match and match.group(1):
            return match.group(1)
        return 'unnamed'
    
    def _prettify(self, elem):
        rough_string = ET.tostring(elem, encoding='unicode')
        reparsed = minidom.parseString(rough_string)
        return reparsed.toprettyxml(indent="  ", encoding=None).strip()


def main():
    parser = argparse.ArgumentParser(description='Convert C++ header struct to XML')
    parser.add_argument('header_file', help='Input header file path')
    parser.add_argument('struct_name', help='Root struct or union name to convert')
    parser.add_argument('-p', '--packed', action='store_true', help='Use packed alignment')
    parser.add_argument('-o', '--output', help='Output XML file name')
    
    args = parser.parse_args()
    
    converter = HeaderToXMLConverter()
    try:
        xml_content = converter.convert(args.header_file, args.struct_name, args.packed)
        
        if args.output:
            with open(args.output, 'w') as f:
                f.write(xml_content)
        else:
            print(xml_content)
            
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()