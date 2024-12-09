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

export class BinaryExprAST : public ExprAST {
  std::string Op;
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(std::string Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : Op(std::move(Op)), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

export class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(Callee), Args(std::move(Args)) {}
};

export class Parser {
private:
  Token cur_tok;
  Lexer lexer;

public:
  Parser(const std::string &src)
      : cur_tok(TokenType::EndOfFile, "", 0, 0), lexer(src) {
    getNextToken();
  }

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

  std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
      return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
  }

  std::unique_ptr<ExprAST> ParsePrimary() {
    switch (cur_tok.type) {
    case TokenType::Integer:
      return ParseIntLiteral();
    case TokenType::Identifier:
      return ParseIdentifierExpr();
    case TokenType::OpenParen:
      return ParseParenExpr();
    default:
      return nullptr;
    }
  }

  std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = cur_tok.value;
    getNextToken();

    if (cur_tok.type != TokenType::OpenParen)
      return std::make_unique<VariableExprAST>(IdName);

    getNextToken();
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (cur_tok.type != TokenType::CloseParen) {
      while (true) {
        if (auto Arg = ParseExpression())
          Args.push_back(std::move(Arg));
        else
          return nullptr;

        if (cur_tok.type == TokenType::CloseParen)
          break;

        if (cur_tok.type != TokenType::Comma)
          return nullptr;
        getNextToken();
      }
    }
    getNextToken();

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
  }

  std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken();
    auto V = ParseExpression();
    if (!V)
      return nullptr;

    if (cur_tok.type != TokenType::CloseParen)
      return nullptr;
    getNextToken();
    return V;
  }

  int GetTokenPrecedence() {
    if (cur_tok.type != TokenType::Operator)
      return -1;

    if (cur_tok.value == "+")
      return 20;
    if (cur_tok.value == "-")
      return 20;
    if (cur_tok.value == "*")
      return 40;
    if (cur_tok.value == "/")
      return 40;
    return -1;
  }

  std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                         std::unique_ptr<ExprAST> LHS) {
    while (true) {
      int TokenPrec = GetTokenPrecedence();

      if (TokenPrec < ExprPrec)
        return LHS;

      std::string BinOp = cur_tok.value;
      getNextToken();

      auto RHS = ParsePrimary();
      if (!RHS)
        return nullptr;

      int NextPrec = GetTokenPrecedence();
      if (TokenPrec < NextPrec) {
        RHS = ParseBinOpRHS(TokenPrec + 1, std::move(RHS));
        if (!RHS)
          return nullptr;
      }

      LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
                                            std::move(RHS));
    }
  }

private:
  Token getNextToken() { return cur_tok = lexer.nextToken(); }
};
