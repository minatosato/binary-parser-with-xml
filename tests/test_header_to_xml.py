import unittest
import tempfile
import os
import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'src', 'header_to_xml'))
from header_to_xml import HeaderToXMLConverter
import xml.etree.ElementTree as ET


class TestHeaderToXMLConverter(unittest.TestCase):
    def test_simple_struct(self):
        header_content = """
        #include <stdint.h>
        
        struct SimpleStruct {
            uint32_t field1;
            uint16_t field2;
            uint8_t field3;
        };
        """
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
            f.write(header_content)
            header_file = f.name
        
        try:
            converter = HeaderToXMLConverter()
            xml_string = converter.convert(header_file, "SimpleStruct")
            
            root = ET.fromstring(xml_string)
            self.assertEqual(root.tag, 'struct')
            self.assertEqual(root.get('name'), 'SimpleStruct')
            
            fields = root.findall('field')
            self.assertEqual(len(fields), 3)
            
            self.assertEqual(fields[0].get('name'), 'field1')
            self.assertEqual(fields[0].get('type'), 'uint32_t')
            
            self.assertEqual(fields[1].get('name'), 'field2')
            self.assertEqual(fields[1].get('type'), 'uint16_t')
            
            self.assertEqual(fields[2].get('name'), 'field3')
            self.assertEqual(fields[2].get('type'), 'uint8_t')
            
        finally:
            os.unlink(header_file)


    def test_struct_with_union(self):
        header_content = """
        #include <stdint.h>
        
        struct StructWithUnion {
            uint32_t type;
            union {
                uint32_t int_value;
                float float_value;
            } data;
        };
        """
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
            f.write(header_content)
            header_file = f.name
        
        try:
            converter = HeaderToXMLConverter()
            xml_string = converter.convert(header_file, "StructWithUnion")
            
            root = ET.fromstring(xml_string)
            self.assertEqual(root.tag, 'struct')
            self.assertEqual(root.get('name'), 'StructWithUnion')
            
            type_field = root.find("./field[@name='type']")
            self.assertIsNotNone(type_field)
            self.assertEqual(type_field.get('type'), 'uint32_t')
            
            union_field = root.find("./field[@name='data']")
            self.assertIsNotNone(union_field)
            
            union_elem = union_field.find('union')
            self.assertIsNotNone(union_elem)
            
            union_fields = union_elem.findall('field')
            self.assertEqual(len(union_fields), 2)
            
        finally:
            os.unlink(header_file)

if __name__ == '__main__':
    unittest.main()