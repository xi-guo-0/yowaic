#include <compare>
#include <gtest/gtest.h>
#include <ranges>

import lexer;

class LexerTest : public ::testing::Test {
protected:
  std::string sourceCode = R"(int main() {
    int x = 42;
    if (x > 10) {
        printf("Hello, World!\n");
    }
    return 0;
}
)";
  Lexer lexer{sourceCode};
};

TEST_F(LexerTest, TokenizeIdentifier) {
  Token token = lexer.nextToken();

  EXPECT_EQ(token.type, TokenType::Keyword);
  EXPECT_EQ(token.value, "int");
  EXPECT_EQ(token.line, 1);
  EXPECT_EQ(token.column, 4);
}

TEST_F(LexerTest, TokenizeKeyword) {
  lexer.nextToken();

  Token token = lexer.nextToken();

  EXPECT_EQ(token.type, TokenType::Identifier);
  EXPECT_EQ(token.value, "main");
  EXPECT_EQ(token.line, 1);
  EXPECT_EQ(token.column, 9);
}

TEST_F(LexerTest, TokenizeInteger) {
  for (auto _ : std::views::iota(0, 8)) {
    lexer.nextToken();
  }

  Token token = lexer.nextToken();

  EXPECT_EQ(token.type, TokenType::Integer);
  EXPECT_EQ(token.value, "42");
  EXPECT_EQ(token.line, 2);
  EXPECT_EQ(token.column, 15);
}

TEST_F(LexerTest, TokenizeOperator) {
  for (auto _ : std::views::iota(0, 7)) {
    lexer.nextToken();
  }

  Token token = lexer.nextToken();

  EXPECT_EQ(token.type, TokenType::Operator);
  EXPECT_EQ(token.value, "=");
  EXPECT_EQ(token.line, 2);
  EXPECT_EQ(token.column, 12);
}

TEST_F(LexerTest, TokenizeCondition) {
  for (auto _ : std::views::iota(0, 13)) {
    lexer.nextToken();
  }

  Token token = lexer.nextToken();

  EXPECT_EQ(token.type, TokenType::Operator);
  EXPECT_EQ(token.value, ">");
  EXPECT_EQ(token.line, 3);
  EXPECT_EQ(token.column, 12);

  token = lexer.nextToken();
  EXPECT_EQ(token.type, TokenType::Integer);
  EXPECT_EQ(token.value, "10");
  EXPECT_EQ(token.line, 3);
  EXPECT_EQ(token.column, 15);
}

TEST_F(LexerTest, TokenizeStringLiteral) {
  for (auto _ : std::views::iota(0, 19)) {
    lexer.nextToken();
  }

  Token token = lexer.nextToken();
  EXPECT_EQ(token.type, TokenType::StringLiteral);
  EXPECT_EQ(token.value, "Hello, World!\\n");
  EXPECT_EQ(token.line, 4);
  EXPECT_EQ(token.column, 33);
}

TEST_F(LexerTest, TokenizeReturnStatement) {
  for (auto _ : std::views::iota(0, 23)) {
    lexer.nextToken();
  }

  Token token = lexer.nextToken();
  EXPECT_EQ(token.type, TokenType::Keyword);
  EXPECT_EQ(token.value, "return");
  EXPECT_EQ(token.line, 6);
  EXPECT_EQ(token.column, 11);

  token = lexer.nextToken();
  EXPECT_EQ(token.type, TokenType::Integer);
  EXPECT_EQ(token.value, "0");
  EXPECT_EQ(token.line, 6);
  EXPECT_EQ(token.column, 13);
}

TEST_F(LexerTest, TokenizeEndOfFile) {
  while (!lexer.isEndOfFile()) {
    lexer.nextToken();
  }

  Token token = lexer.nextToken();
  EXPECT_EQ(token.type, TokenType::EndOfFile);
  EXPECT_EQ(token.value, "");
  EXPECT_EQ(token.line, 8);
  EXPECT_EQ(token.column, 1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
