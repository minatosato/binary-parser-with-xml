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
        self.macro_map = {}  # For #define expansion
    
    def convert(self, header_file, root_struct_name, packed=False):
        # Reset state for new conversion
        self.typedef_map = {}
        self.struct_map = {}
        self.processed_files = set()
        self.macro_map = {}
        self._bitfield_state = {}
        
        # Process includes recursively
        self._process_header_file(header_file)
        
        # Find the struct definition  
        match = None
        struct_body = None
        is_typedef = False
        
        # First check if it's a typedef
        if root_struct_name in self.typedef_map:
            typedef_info = self.typedef_map[root_struct_name]
            if isinstance(typedef_info, tuple):
                typedef_type, struct_body = typedef_info
                if typedef_type == 'struct':
                    match = True
                    is_typedef = True
                else:
                    raise ValueError(f"'{root_struct_name}' is a typedef union, not a struct")
            else:
                # Old format compatibility
                struct_body = typedef_info
                match = True
                is_typedef = True
        
        # If not found as typedef, try to find as regular struct
        if not match:
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
        
        # Extract macros (#define)
        self._extract_macros(content)
        
        # Extract typedefs - handle nested braces properly
        # First, find all typedef declarations
        typedef_pattern = r'typedef\s+(struct|union)\s*\{((?:[^{}]|(?:\{[^{}]*\}))*)\}\s*(\w+)\s*;'
        
        # Process the content to find typedefs with proper brace matching
        pos = 0
        while pos < len(content):
            # Find the next typedef
            typedef_match = re.search(r'typedef\s+(struct|union)\s*\{', content[pos:])
            if not typedef_match:
                # Also check for simple typedefs like typedef int MyInt;
                simple_typedef_match = re.search(r'typedef\s+(\w+)\s+(\w+)\s*;', content[pos:])
                if simple_typedef_match:
                    original_type, new_type = simple_typedef_match.groups()
                    self.typedef_map[new_type] = original_type
                    pos += simple_typedef_match.start() + simple_typedef_match.end()
                    continue
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
                end_line = lines[end_idx] if end_idx < len(lines) else ''
                
                # Check if it's an array of structs
                array_match = re.search(r'}\s*(\w*)\s*\[(\w+)\]\s*;', end_line)
                if array_match:
                    field_name = array_match.group(1) if array_match.group(1) else 'unnamed'
                    array_size = array_match.group(2)
                    expanded_size = self._expand_macro(array_size)
                    
                    field_elem = ET.SubElement(parent_elem, 'field')
                    field_elem.set('name', field_name)
                    field_elem.set('offset', str(offset))
                    field_elem.set('array_size', expanded_size)
                    
                    struct_elem = ET.SubElement(field_elem, 'struct')
                    element_size = self._parse_struct_body(struct_body, struct_elem, 0, packed)
                    field_size = element_size * int(expanded_size)
                    field_elem.set('size', str(field_size))
                    
                    offset += field_size
                else:
                    # Regular struct (not array)
                    field_name = self._extract_field_name(end_line)
                    
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
                    
                    # Check for array (now with macro support)
                    array_match = re.match(r'(\w+)\s+(\w+)\[(\w+)\]\s*;', line)
                    if array_match:
                        field_type, field_name, array_size = array_match.groups()
                        # Expand macro if it's a macro name
                        expanded_size = self._expand_macro(array_size)
                        
                        field_elem = ET.SubElement(parent_elem, 'field')
                        field_elem.set('name', field_name)
                        field_elem.set('array_size', expanded_size)
                        field_elem.set('offset', str(offset))
                        
                        # Check if array element is typedef
                        resolved_type_info = self._resolve_typedef(field_type)
                        if isinstance(resolved_type_info, tuple):
                            # It's a typedef struct/union
                            typedef_type, typedef_body = resolved_type_info
                            # For arrays of typedef structs, parse the structure
                            if typedef_type == 'struct':
                                struct_elem = ET.SubElement(field_elem, 'struct')
                                element_size = self._parse_struct_body(typedef_body, struct_elem, 0, packed)
                            else:
                                union_elem = ET.SubElement(field_elem, 'union')
                                element_size = self._parse_union_body(typedef_body, union_elem, 0, packed)
                            field_size = element_size * int(expanded_size)
                        else:
                            # Check if it's a struct type
                            struct_found = False
                            for content in self.struct_map.values():
                                struct_pattern = rf'struct\s+{field_type}\s*\{{'
                                if re.search(struct_pattern, content):
                                    struct_found = True
                                    # Extract and parse the struct
                                    match = re.search(rf'struct\s+{field_type}\s*\{{((?:[^{{}}]|(?:\{{[^{{}}]*\}}))*)\}}', content, re.DOTALL)
                                    if match:
                                        struct_elem = ET.SubElement(field_elem, 'struct')
                                        element_size = self._parse_struct_body(match.group(1), struct_elem, 0, packed)
                                        field_size = element_size * int(expanded_size)
                                    break
                            
                            if not struct_found:
                                # It's a simple type
                                field_elem.set('type', resolved_type_info)
                                type_size = self.type_sizes.get(resolved_type_info, 4)
                                field_size = type_size * int(expanded_size)
                        
                        field_elem.set('size', str(field_size))
                        
                        if not packed:
                            # Get proper alignment
                            if isinstance(resolved_type_info, tuple):
                                align_size = self._get_struct_alignment(temp_elem) if 'temp_elem' in locals() else 4
                            else:
                                align_size = self.type_sizes.get(resolved_type_info, 4)
                            offset = self._align_offset(offset + field_size, align_size)
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
                            resolved_type_info = self._resolve_typedef(field_type)
                            if isinstance(resolved_type_info, tuple):
                                typedef_type, typedef_body = resolved_type_info
                                if typedef_type == 'struct':
                                    struct_elem = ET.SubElement(field_elem, 'struct')
                                    struct_size = self._parse_struct_body(typedef_body, struct_elem, 0, packed)
                                else:  # union
                                    struct_elem = ET.SubElement(field_elem, 'union')
                                    struct_size = self._parse_union_body(typedef_body, struct_elem, offset, packed)
                                field_elem.set('size', str(struct_size))
                                offset += struct_size
                            else:
                                # Check if it's a known struct type from includes
                                struct_found = False
                                for content in self.struct_map.values():
                                    # Look for struct definitions
                                    struct_pattern = rf'struct\s+{field_type}\s*\{{'
                                    if re.search(struct_pattern, content):
                                        struct_found = True
                                        # Extract and parse the struct
                                        full_pattern = rf'struct\s+{field_type}\s*\{{([^{{}}]*(?:\{{[^{{}}]*\}}[^{{}}]*)*)\}}'
                                        match = re.search(full_pattern, content, re.DOTALL)
                                        if match:
                                            struct_elem = ET.SubElement(field_elem, 'struct')
                                            struct_size = self._parse_struct_body(match.group(1), struct_elem, 0, packed)
                                            field_elem.set('size', str(struct_size))
                                            offset += struct_size
                                        break
                                
                                if not struct_found:
                                    # It's a simple typedef or basic type
                                    field_elem.set('type', resolved_type_info)
                                    type_size = self.type_sizes.get(resolved_type_info, 4)
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
    
    def _parse_union_body(self, body, union_elem, offset=0, packed=False):
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
            
            # Check for array in union (with macro support)
            array_match = re.match(r'(\w+)\s+(\w+)\[(\w+)\]\s*;', line)
            if array_match:
                field_type, field_name, array_size = array_match.groups()
                # Expand macro if it's a macro name
                expanded_size = self._expand_macro(array_size)
                
                field_elem = ET.SubElement(union_elem, 'field')
                field_elem.set('name', field_name)
                field_elem.set('array_size', expanded_size)
                field_elem.set('offset', str(0))  # Union fields are relative to union start
                
                # Check if array element is typedef
                resolved_type_info = self._resolve_typedef(field_type)
                if isinstance(resolved_type_info, tuple):
                    typedef_type, typedef_body = resolved_type_info
                    # For arrays of typedefs, we need to get the size once
                    temp_elem = ET.Element('temp')
                    if typedef_type == 'struct':
                        element_size = self._parse_struct_body(typedef_body, temp_elem, 0, packed)
                    else:
                        element_size = self._parse_union_body(typedef_body, temp_elem, 0, packed)
                    # Don't expand typedef arrays - just set type
                    field_elem.set('type', field_type)
                    field_size = element_size * int(expanded_size)
                else:
                    field_elem.set('type', resolved_type_info)
                    type_size = self.type_sizes.get(resolved_type_info, 4)
                    field_size = type_size * int(expanded_size)
                
                field_elem.set('size', str(field_size))
                max_size = max(max_size, field_size)
                i += 1
                continue
            
            field_match = re.match(r'(\w+)\s+(\w+)\s*;', line)
            if field_match:
                field_type, field_name = field_match.groups()
                field_elem = ET.SubElement(union_elem, 'field')
                field_elem.set('name', field_name)
                field_elem.set('offset', str(0))  # Union fields are relative to union start
                
                # Check if it's a typedef or known struct
                resolved_type_info = self._resolve_typedef(field_type)
                if isinstance(resolved_type_info, tuple):
                    typedef_type, typedef_body = resolved_type_info
                    if typedef_type == 'struct':
                        struct_elem = ET.SubElement(field_elem, 'struct')
                        struct_size = self._parse_struct_body(typedef_body, struct_elem, 0, packed)
                    else:  # union
                        struct_elem = ET.SubElement(field_elem, 'union')
                        struct_size = self._parse_union_body(typedef_body, struct_elem, current_offset, packed)
                    field_elem.set('size', str(struct_size))
                    max_size = max(max_size, struct_size)
                else:
                    field_elem.set('type', resolved_type_info)
                    type_size = self.type_sizes.get(resolved_type_info, 4)
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
    
    def _resolve_typedef(self, type_name):
        """Recursively resolves a typedef to its base type or struct/union info."""
        resolved_type = type_name
        while resolved_type in self.typedef_map:
            typedef_info = self.typedef_map[resolved_type]
            if isinstance(typedef_info, tuple):
                # It's a typedef struct/union, return its info
                return typedef_info
            else:
                # It's a simple typedef, continue resolving
                resolved_type = typedef_info
        return resolved_type

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
    
    def _extract_macros(self, content):
        """Extract #define macros from content"""
        # Process defines in order (important for dependent macros)
        lines = content.split('\n')
        
        for line in lines:
            line = line.strip()
            if not line.startswith('#define'):
                continue
                
            # Simple numeric defines
            simple_match = re.match(r'#define\s+(\w+)\s+(\d+)', line)
            if simple_match:
                name, value = simple_match.groups()
                self.macro_map[name] = int(value)
                continue
            
            # Defines with arithmetic expressions
            expr_match = re.match(r'#define\s+(\w+)\s+\(([^)]+)\)', line)
            if expr_match:
                name, expr = expr_match.groups()
                evaluated = self._evaluate_macro_expression(expr)
                if evaluated is not None:
                    self.macro_map[name] = evaluated
                continue
                
            # Defines referencing other macros (no parentheses)
            ref_match = re.match(r'#define\s+(\w+)\s+(\w+)', line)
            if ref_match:
                name, ref = ref_match.groups()
                if ref in self.macro_map:
                    self.macro_map[name] = self.macro_map[ref]
    
    def _evaluate_macro_expression(self, expr):
        """Safely evaluate simple arithmetic expressions"""
        try:
            # Replace known macros in expression
            eval_expr = expr
            for macro_name, macro_value in sorted(self.macro_map.items(), 
                                                 key=lambda x: len(x[0]), reverse=True):
                eval_expr = re.sub(r'\b' + macro_name + r'\b', str(macro_value), eval_expr)
            
            # Only allow numbers, operators, and parentheses
            if not re.match(r'^[\d\s+\-*/()]+$', eval_expr):
                return None
                
            # Safe evaluation using ast
            import ast
            node = ast.parse(eval_expr, mode='eval')
            # Only allow safe operations
            for n in ast.walk(node):
                if not isinstance(n, (ast.Expression, ast.BinOp, ast.UnaryOp, 
                                    ast.Num, ast.Constant, ast.Add, ast.Sub, 
                                    ast.Mult, ast.Div, ast.USub)):
                    return None
            
            result = eval(compile(node, '<string>', 'eval'))
            return int(result)
        except:
            return None
    
    def _expand_macro(self, value):
        """Expand macro if it exists, otherwise return original value"""
        if value in self.macro_map:
            return str(self.macro_map[value])
        return value
    
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
