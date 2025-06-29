import re
import os
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
        
        # Extract typedefs
        typedef_pattern = r'typedef\s+struct\s*{([^}]+)}\s*(\w+)\s*;'
        for match in re.finditer(typedef_pattern, content, re.DOTALL):
            struct_body, type_name = match.groups()
            self.typedef_map[type_name] = struct_body
        
        # Process includes
        include_pattern = r'#include\s*["<]([^">]+)[">]'
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
                    # Bitfields don't increment offset in simple cases
                    # This is a simplification - real bitfield layout is complex
                else:
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
                                struct_elem = ET.SubElement(field_elem, 'struct')
                                struct_size = self._parse_struct_body(self.typedef_map[field_type], struct_elem, 0, packed)
                                field_elem.set('size', str(struct_size))
                                offset += struct_size
                            else:
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
        
        for line in lines:
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            
            field_match = re.match(r'(\w+)\s+(\w+)\s*;', line)
            if field_match:
                field_type, field_name = field_match.groups()
                field_elem = ET.SubElement(union_elem, 'field')
                field_elem.set('name', field_name)
                field_elem.set('type', field_type)
                field_elem.set('offset', str(current_offset))
                
                type_size = self.type_sizes.get(field_type, 4)
                field_elem.set('size', str(type_size))
                max_size = max(max_size, type_size)
        
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