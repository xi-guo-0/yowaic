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

export class DeclarationAST : public ExprAST {
  std::string Type;
  std::string Name;
  std::unique_ptr<ExprAST> InitExpr;

public:
  DeclarationAST(std::string Type, std::string Name,
                 std::unique_ptr<ExprAST> InitExpr)
      : Type(std::move(Type)), Name(std::move(Name)),
        InitExpr(std::move(InitExpr)) {}
};

export class CompoundStmtAST : public ExprAST {
  std::vector<std::unique_ptr<ExprAST>> Statements;

public:
  CompoundStmtAST(std::vector<std::unique_ptr<ExprAST>> Statements)
      : Statements(std::move(Statements)) {}
};

export class IfStmtAST : public ExprAST {
public:
  std::unique_ptr<ExprAST> Condition;
  std::unique_ptr<ExprAST> ThenBranch;
  std::unique_ptr<ExprAST> ElseBranch;

  IfStmtAST(std::unique_ptr<ExprAST> Condition,
            std::unique_ptr<ExprAST> ThenBranch,
            std::unique_ptr<ExprAST> ElseBranch = nullptr)
      : Condition(std::move(Condition)), ThenBranch(std::move(ThenBranch)),
        ElseBranch(std::move(ElseBranch)) {}
};

export class ReturnStmtAST : public ExprAST {
  std::unique_ptr<ExprAST> ReturnExpr;

public:
  ReturnStmtAST(std::unique_ptr<ExprAST> ReturnExpr)
      : ReturnExpr(std::move(ReturnExpr)) {}
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

    if (cur_tok.type == TokenType::Operator) {
      return ParseBinOpRHS(0, std::move(LHS));
    }

    return LHS;
  }

  std::unique_ptr<ExprAST> ParsePrimary() {
    switch (cur_tok.type) {
    case TokenType::Integer:
      return ParseIntLiteral();
    case TokenType::Identifier:
      return ParseIdentifierExpr();
    case TokenType::OpenParen:
      return ParseParenExpr();
    case TokenType::Operator:
      if (cur_tok.value == ">" || cur_tok.value == "<" ||
          cur_tok.value == "==" || cur_tok.value == "!=" ||
          cur_tok.value == ">=" || cur_tok.value == "<=") {
        return ParseExpression();
      }
      return nullptr;
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
    if (cur_tok.value == ">" || cur_tok.value == "<" || cur_tok.value == ">=" ||
        cur_tok.value == "<=" || cur_tok.value == "==" || cur_tok.value == "!=")
      return 10;
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

      if (cur_tok.type == TokenType::CloseParen)
        break;
    }
    return LHS;
  }

  std::unique_ptr<ExprAST> ParseDeclaration() {
    std::string Type = cur_tok.value;
    getNextToken();

    if (cur_tok.type != TokenType::Identifier)
      return nullptr;

    std::string Name = cur_tok.value;
    getNextToken();

    std::unique_ptr<ExprAST> InitExpr = nullptr;
    if (cur_tok.type == TokenType::Operator && cur_tok.value == "=") {
      getNextToken();
      InitExpr = ParseExpression();
      if (!InitExpr)
        return nullptr;
    }

    if (cur_tok.type != TokenType::Semicolon)
      return nullptr;

    getNextToken();
    return std::make_unique<DeclarationAST>(Type, Name, std::move(InitExpr));
  }

  std::unique_ptr<ExprAST> ParseStatement() {
    switch (cur_tok.type) {
    case TokenType::Keyword:
      if (cur_tok.value == "if")
        return ParseIfStatement();
      if (cur_tok.value == "return")
        return ParseReturnStatement();
      if (cur_tok.value == "int" || cur_tok.value == "void")
        return ParseDeclaration();
      return nullptr;
    case TokenType::OpenBrace:
      return ParseCompoundStatement();
    case TokenType::Identifier:
      return ParseVariable();
    default:
      auto Expr = ParseExpression();
      if (!Expr)
        return nullptr;

      if (cur_tok.type != TokenType::Semicolon)
        return nullptr;

      getNextToken();
      return Expr;
    }
  }

  std::unique_ptr<ExprAST> ParseCompoundStatement() {
    getNextToken();

    std::vector<std::unique_ptr<ExprAST>> Statements;
    while (cur_tok.type != TokenType::CloseBrace && !lexer.isEndOfFile()) {
      auto Stmt = ParseStatement();
      if (!Stmt)
        return nullptr;
      Statements.push_back(std::move(Stmt));
    }

    if (cur_tok.type != TokenType::CloseBrace)
      return nullptr;

    getNextToken();
    return std::make_unique<CompoundStmtAST>(std::move(Statements));
  }

  std::unique_ptr<ExprAST> ParseIfStatement() {
    getNextToken();

    if (cur_tok.type != TokenType::OpenParen)
      return nullptr;
    getNextToken();

    auto LHS = ParsePrimary();
    if (!LHS) {
      return nullptr;
    }

    if (cur_tok.type != TokenType::Operator) {
      return nullptr;
    }

    std::string Op = cur_tok.value;
    getNextToken();

    auto RHS = ParsePrimary();
    if (!RHS) {
      return nullptr;
    }

    auto Condition =
        std::make_unique<BinaryExprAST>(Op, std::move(LHS), std::move(RHS));

    if (cur_tok.type != TokenType::CloseParen) {
      return nullptr;
    }
    getNextToken();

    auto ThenBranch = ParseStatement();
    if (!ThenBranch) {
      return nullptr;
    }

    std::unique_ptr<ExprAST> ElseBranch = nullptr;
    if (cur_tok.type == TokenType::Keyword && cur_tok.value == "else") {
      getNextToken();
      ElseBranch = ParseStatement();
      if (!ElseBranch) {
        return nullptr;
      }
    }

    return std::make_unique<IfStmtAST>(
        std::move(Condition), std::move(ThenBranch), std::move(ElseBranch));
  }

  std::unique_ptr<ExprAST> ParseReturnStatement() {
    getNextToken();

    std::unique_ptr<ExprAST> ReturnExpr = nullptr;
    if (cur_tok.type != TokenType::Semicolon) {
      ReturnExpr = ParseExpression();
      if (!ReturnExpr)
        return nullptr;
    }

    if (cur_tok.type != TokenType::Semicolon)
      return nullptr;

    getNextToken();
    return std::make_unique<ReturnStmtAST>(std::move(ReturnExpr));
  }

private:
  Token getNextToken() { return cur_tok = lexer.nextToken(); }
};
