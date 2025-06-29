import re
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
        }
    
    def convert(self, header_file, root_struct_name, packed=False):
        with open(header_file, 'r') as f:
            content = f.read()
        
        struct_pattern = rf'struct\s+{root_struct_name}\s*{{([^{{}}]*(?:{{[^{{}}]*}}[^{{}}]*)*)}}'
        match = re.search(struct_pattern, content, re.DOTALL)
        
        if not match:
            raise ValueError(f"Struct '{root_struct_name}' not found in header file")
        
        struct_body = match.group(1)
        
        root = ET.Element('struct', name=root_struct_name)
        if packed:
            root.set('packed', 'true')
        
        self._parse_struct_body(struct_body, root)
        
        return self._prettify(root)
    
    def _parse_struct_body(self, body, parent_elem):
        lines = body.strip().split('\n')
        i = 0
        
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
                
                union_elem = ET.SubElement(field_elem, 'union')
                self._parse_struct_body(union_body, union_elem)
                
                i = end_idx + 1
            elif 'struct' in line and '{' in line:
                struct_body, end_idx = self._extract_block(lines, i)
                field_name = self._extract_field_name(lines[end_idx])
                
                field_elem = ET.SubElement(parent_elem, 'field')
                field_elem.set('name', field_name)
                
                struct_elem = ET.SubElement(field_elem, 'struct')
                self._parse_struct_body(struct_body, struct_elem)
                
                i = end_idx + 1
            else:
                field_match = re.match(r'(\w+)\s+(\w+)\s*;', line)
                if field_match:
                    field_type, field_name = field_match.groups()
                    field_elem = ET.SubElement(parent_elem, 'field')
                    field_elem.set('name', field_name)
                    field_elem.set('type', field_type)
                
                i += 1
    
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
        match = re.search(r'}\s*(\w+)\s*;', line)
        if match:
            return match.group(1)
        return 'unnamed'
    
    def _prettify(self, elem):
        rough_string = ET.tostring(elem, encoding='unicode')
        reparsed = minidom.parseString(rough_string)
        return reparsed.toprettyxml(indent="  ", encoding=None).strip()