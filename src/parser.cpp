export module parser;

import lexer;
import std;

export class ExprAST {
public:
  virtual ~ExprAST() = default;
};

export class IntLiteral : public ExprAST {
public:
  int value;
  IntLiteral(int v) : value(v) {}
};

export class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
};

export class Parser {
private:
  Token cur_tok;
  Lexer lexer;

public:
  Parser(const std::string &src)
      : cur_tok(TokenType::EndOfFile, "", 0, 0), lexer(src) {}

  std::unique_ptr<ExprAST> ParseIntLiteral() {
    auto Result = std::make_unique<IntLiteral>(std::stoi(cur_tok.value));
    getNextToken();
    return std::move(Result);
  }

  std::unique_ptr<ExprAST> ParseVariable() {
    auto Result = std::make_unique<VariableExprAST>(cur_tok.value);
    getNextToken();
    return std::move(Result);
  }

private:
  Token getNextToken() { return cur_tok = lexer.nextToken(); }
};
