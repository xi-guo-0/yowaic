#include <compare>
#include <gtest/gtest.h>
#include <ranges>

import lexer;
import parser;

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

class ParserTest : public ::testing::Test {
protected:
  std::string sourceCode = R"(int main() {
    int x = 42;
    if (x > 10) {
        printf("Hello, World!\n");
    }
    return 0;
}
)";
  Parser parser{sourceCode};
};

TEST(ParserTest, ParseIntLiteral_ShouldReturnCorrectIntLiteral) {
  Parser parser("42");
  EXPECT_EQ(dynamic_cast<IntLiteral *>(parser.ParseIntLiteral().get())->value,
            42);
}

TEST(ParserTest, ParseVariable) {
  Parser parser("x");
  auto result = parser.ParseVariable();

  EXPECT_TRUE(result != nullptr);
  auto varExpr = dynamic_cast<VariableExprAST *>(result.get());
  EXPECT_TRUE(varExpr != nullptr);
  EXPECT_EQ(varExpr->Name, "x");
}

TEST(ParserTest, ParseBinaryExpression) {
  std::string source = "3 + 4 * 2";
  Parser parser(source);

  auto expr = parser.ParseExpression();
  ASSERT_NE(expr, nullptr);

  auto *binExpr = dynamic_cast<BinaryExprAST *>(expr.get());
  ASSERT_NE(binExpr, nullptr);
  EXPECT_EQ(binExpr->Op, "+");

  auto *lhs = dynamic_cast<IntLiteral *>(binExpr->LHS.get());
  ASSERT_NE(lhs, nullptr);
  EXPECT_EQ(lhs->value, 3);

  auto *rhs = dynamic_cast<BinaryExprAST *>(binExpr->RHS.get());
  ASSERT_NE(rhs, nullptr);
  EXPECT_EQ(rhs->Op, "*");

  auto *rhs_lhs = dynamic_cast<IntLiteral *>(rhs->LHS.get());
  ASSERT_NE(rhs_lhs, nullptr);
  EXPECT_EQ(rhs_lhs->value, 4);

  auto *rhs_rhs = dynamic_cast<IntLiteral *>(rhs->RHS.get());
  ASSERT_NE(rhs_rhs, nullptr);
  EXPECT_EQ(rhs_rhs->value, 2);
}

TEST(ParserTest, ParseFunctionCall) {
  std::string source = "printf(42, x)";
  Parser parser(source);

  auto expr = parser.ParseExpression();
  ASSERT_NE(expr, nullptr);

  auto *callExpr = dynamic_cast<CallExprAST *>(expr.get());
  ASSERT_NE(callExpr, nullptr);
  EXPECT_EQ(callExpr->Callee, "printf");
  ASSERT_EQ(callExpr->Args.size(), 2);

  auto *firstArg = dynamic_cast<IntLiteral *>(callExpr->Args[0].get());
  ASSERT_NE(firstArg, nullptr);
  EXPECT_EQ(firstArg->value, 42);

  auto *secondArg = dynamic_cast<VariableExprAST *>(callExpr->Args[1].get());
  ASSERT_NE(secondArg, nullptr);
  EXPECT_EQ(secondArg->Name, "x");
}

TEST(ParserTest, ParseParenExpression) {
  std::string source = "(1 + 2) * 3";
  Parser parser(source);

  auto expr = parser.ParseExpression();
  ASSERT_NE(expr, nullptr);

  auto *binExpr = dynamic_cast<BinaryExprAST *>(expr.get());
  ASSERT_NE(binExpr, nullptr);
  EXPECT_EQ(binExpr->Op, "*");

  auto *rhs = dynamic_cast<IntLiteral *>(binExpr->RHS.get());
  ASSERT_NE(rhs, nullptr);
  EXPECT_EQ(rhs->value, 3);

  auto *lhs = dynamic_cast<BinaryExprAST *>(binExpr->LHS.get());
  ASSERT_NE(lhs, nullptr);
  EXPECT_EQ(lhs->Op, "+");

  auto *lhs_lhs = dynamic_cast<IntLiteral *>(lhs->LHS.get());
  ASSERT_NE(lhs_lhs, nullptr);
  EXPECT_EQ(lhs_lhs->value, 1);

  auto *lhs_rhs = dynamic_cast<IntLiteral *>(lhs->RHS.get());
  ASSERT_NE(lhs_rhs, nullptr);
  EXPECT_EQ(lhs_rhs->value, 2);
}

TEST(ParserTest, ParseDeclaration) {
  Parser parser("int x = 42;");
  auto Result = parser.ParseDeclaration();
  ASSERT_NE(Result, nullptr);
  auto *Decl = dynamic_cast<DeclarationAST *>(Result.get());
  ASSERT_NE(Decl, nullptr);
  auto *Init = dynamic_cast<IntLiteral *>(Decl->InitExpr.get());
  ASSERT_NE(Init, nullptr);
  EXPECT_EQ(Init->value, 42);
}

TEST(ParserTest, ParseIfStatement) {
  Parser parser("if (x > 10) { return 1; }");

  auto Result = parser.ParseIfStatement();

  ASSERT_NE(Result, nullptr);
  auto *If = dynamic_cast<IfStmtAST *>(Result.get());
  ASSERT_NE(If, nullptr);

  auto *Cond = dynamic_cast<BinaryExprAST *>(If->Condition.get());
  ASSERT_NE(Cond, nullptr);

  auto *Then = dynamic_cast<CompoundStmtAST *>(If->ThenBranch.get());
  ASSERT_NE(Then, nullptr);
}

TEST(ParserTest, ParseCompoundStatement) {
  Parser parser("{ int x = 42; return x; }");
  auto Result = parser.ParseCompoundStatement();
  ASSERT_NE(Result, nullptr);
  auto *Compound = dynamic_cast<CompoundStmtAST *>(Result.get());
  ASSERT_NE(Compound, nullptr);
}

TEST(ParserTest, ParseReturnStatement) {
  Parser parser("return 42;");
  auto Result = parser.ParseReturnStatement();
  ASSERT_NE(Result, nullptr);
  auto *Return = dynamic_cast<ReturnStmtAST *>(Result.get());
  ASSERT_NE(Return, nullptr);
  auto *Value = dynamic_cast<IntLiteral *>(Return->ReturnExpr.get());
  ASSERT_NE(Value, nullptr);
  EXPECT_EQ(Value->value, 42);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
