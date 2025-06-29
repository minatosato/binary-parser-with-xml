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
    
    def test_struct_with_bitfield(self):
        header_content = """
        #include <stdint.h>
        
        struct BitfieldStruct {
            uint32_t flag1 : 1;
            uint32_t flag2 : 1;
            uint32_t value : 14;
            uint32_t reserved : 16;
        };
        """
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
            f.write(header_content)
            header_file = f.name
        
        try:
            converter = HeaderToXMLConverter()
            xml_string = converter.convert(header_file, "BitfieldStruct")
            
            root = ET.fromstring(xml_string)
            fields = root.findall('field')
            
            self.assertEqual(fields[0].get('name'), 'flag1')
            self.assertEqual(fields[0].get('bits'), '1')
            
            self.assertEqual(fields[1].get('name'), 'flag2')
            self.assertEqual(fields[1].get('bits'), '1')
            
            self.assertEqual(fields[2].get('name'), 'value')
            self.assertEqual(fields[2].get('bits'), '14')
            
        finally:
            os.unlink(header_file)
    
    def test_struct_with_array(self):
        header_content = """
        #include <stdint.h>
        
        struct ArrayStruct {
            uint8_t header[4];
            uint32_t data[16];
            char name[32];
        };
        """
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
            f.write(header_content)
            header_file = f.name
        
        try:
            converter = HeaderToXMLConverter()
            xml_string = converter.convert(header_file, "ArrayStruct")
            
            root = ET.fromstring(xml_string)
            fields = root.findall('field')
            
            self.assertEqual(fields[0].get('name'), 'header')
            self.assertEqual(fields[0].get('type'), 'uint8_t')
            self.assertEqual(fields[0].get('array_size'), '4')
            
            self.assertEqual(fields[1].get('name'), 'data')
            self.assertEqual(fields[1].get('type'), 'uint32_t')
            self.assertEqual(fields[1].get('array_size'), '16')
            
        finally:
            os.unlink(header_file)
    
    def test_struct_with_offset_and_size(self):
        header_content = """
        #include <stdint.h>
        
        struct OffsetStruct {
            uint32_t field1;
            uint16_t field2;
            uint8_t field3;
            uint8_t field4;
        };
        """
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
            f.write(header_content)
            header_file = f.name
        
        try:
            converter = HeaderToXMLConverter()
            xml_string = converter.convert(header_file, "OffsetStruct")
            
            root = ET.fromstring(xml_string)
            
            # Check total size
            self.assertIsNotNone(root.get('size'))
            
            fields = root.findall('field')
            # Check offsets
            self.assertEqual(fields[0].get('offset'), '0')
            self.assertEqual(fields[1].get('offset'), '4')
            self.assertEqual(fields[2].get('offset'), '6')
            self.assertEqual(fields[3].get('offset'), '7')
            
        finally:
            os.unlink(header_file)
    
    def test_struct_with_includes(self):
        converter = HeaderToXMLConverter()
        header_file = os.path.join(os.path.dirname(__file__), 'test_includes', 'main_struct.h')
        
        xml_string = converter.convert(header_file, "ComplexStruct")
        root = ET.fromstring(xml_string)
        
        fields = root.findall('field')
        self.assertEqual(len(fields), 4)
        
        # Check id field
        self.assertEqual(fields[0].get('name'), 'id')
        self.assertEqual(fields[0].get('type'), 'uint32_t')
        
        # Check position field (should be expanded)
        position_field = fields[1]
        self.assertEqual(position_field.get('name'), 'position')
        position_struct = position_field.find('struct')
        self.assertIsNotNone(position_struct)
        
        # Check color field (should be expanded)
        color_field = fields[2]
        self.assertEqual(color_field.get('name'), 'color')
        color_struct = color_field.find('struct')
        self.assertIsNotNone(color_struct)
    
    def test_anonymous_struct_and_union(self):
        header_content = """
        #include <stdint.h>
        
        struct AnonymousTest {
            uint32_t id;
            struct {
                uint16_t x;
                uint16_t y;
            };  // anonymous struct
            union {
                uint32_t raw;
                struct {
                    uint8_t r;
                    uint8_t g;
                    uint8_t b;
                    uint8_t a;
                };
            };  // anonymous union
        };
        """
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
            f.write(header_content)
            header_file = f.name
        
        try:
            converter = HeaderToXMLConverter()
            xml_string = converter.convert(header_file, "AnonymousTest")
            
            root = ET.fromstring(xml_string)
            fields = root.findall('field')
            
            # Check anonymous struct
            anon_struct = None
            for field in fields:
                if field.find('struct') is not None and field.get('name') == 'unnamed':
                    anon_struct = field
                    break
            
            self.assertIsNotNone(anon_struct)
            struct_elem = anon_struct.find('struct')
            sub_fields = struct_elem.findall('field')
            self.assertEqual(len(sub_fields), 2)
            self.assertEqual(sub_fields[0].get('name'), 'x')
            self.assertEqual(sub_fields[1].get('name'), 'y')
            
        finally:
            os.unlink(header_file)

if __name__ == '__main__':
    unittest.main()