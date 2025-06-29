import re
import xml.etree.ElementTree as ET
from xml.dom import minidom


class HeaderToXMLConverter:
    def __init__(self):
        pass
    
    def convert(self, header_file, root_struct_name, packed=False):
        with open(header_file, 'r') as f:
            content = f.read()
        
        struct_pattern = rf'struct\s+{root_struct_name}\s*{{([^}}]+)}}'
        match = re.search(struct_pattern, content, re.DOTALL)
        
        if not match:
            raise ValueError(f"Struct '{root_struct_name}' not found in header file")
        
        struct_body = match.group(1)
        
        root = ET.Element('struct', name=root_struct_name)
        if packed:
            root.set('packed', 'true')
        
        field_pattern = r'(\w+)\s+(\w+)\s*;'
        fields = re.findall(field_pattern, struct_body)
        
        for field_type, field_name in fields:
            field_elem = ET.SubElement(root, 'field')
            field_elem.set('name', field_name)
            field_elem.set('type', field_type)
        
        return self._prettify(root)
    
    def _prettify(self, elem):
        rough_string = ET.tostring(elem, encoding='unicode')
        reparsed = minidom.parseString(rough_string)
        return reparsed.toprettyxml(indent="  ", encoding=None).strip()