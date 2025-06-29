
#include <gtest/gtest.h>
#include "binary_parser.h"

// BinaryParserクラスのプライベートメソッドにアクセスするためにテスト用の派生クラスを作成
class BinaryParserTest : public ::testing::Test {
protected:
    binary_parser::BinaryParser parser;
};

TEST_F(BinaryParserTest, ByteSwap16) {
    EXPECT_EQ(parser.byteSwap16(0x1234), 0x3412);
    EXPECT_EQ(parser.byteSwap16(0xFF00), 0x00FF);
    EXPECT_EQ(parser.byteSwap16(0x0000), 0x0000);
    EXPECT_EQ(parser.byteSwap16(0xFFFF), 0xFFFF);
}

TEST_F(BinaryParserTest, ByteSwap32) {
    EXPECT_EQ(parser.byteSwap32(0x12345678), 0x78563412);
    EXPECT_EQ(parser.byteSwap32(0xFF000000), 0x000000FF);
    EXPECT_EQ(parser.byteSwap32(0x00FF00FF), 0xFF00FF00);
}

TEST_F(BinaryParserTest, ByteSwap64) {
    EXPECT_EQ(parser.byteSwap64(0x123456789ABCDEF0), 0xF0DEBC9A78563412);
    EXPECT_EQ(parser.byteSwap64(0xFF00000000000000), 0x00000000000000FF);
}
