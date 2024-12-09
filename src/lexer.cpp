export module lexer;

import std;

export enum class TokenType {
  Keyword,
  Identifier,
  Integer,
  Operator,
  OpenParen,
  CloseParen,
  OpenBrace,
  CloseBrace,
  OpenBracket,
  CloseBracket,
  Comma,
  Semicolon,
  StringLiteral,
  Invalid,
  EndOfFile
};

export struct Token {
  TokenType type;
  std::string value;
  std::size_t line;
  std::size_t column;

  Token(TokenType type, const std::string &value, std::size_t line,
        std::size_t column)
      : type(type), value(value), line(line), column(column) {}
};

export class Lexer {
private:
  std::string source;
  std::size_t pos;
  std::size_t line;
  std::size_t column;

  static const std::unordered_set<std::string> keywords;

public:
  Lexer(const std::string &src) : source(src), pos(0), line(1), column(1) {}

  bool isEndOfFile() const { return pos >= source.size(); }

  char currentChar() const { return isEndOfFile() ? '\0' : source[pos]; }

  char peekChar(std::size_t offset) const {
    return pos + offset < source.size() ? source[pos + offset] : '\0';
  }

  void advance() {
    if (currentChar() == '\n') {
      ++line;
      column = 1;
    } else {
      ++column;
    }
    ++pos;
  }

  void skipWhitespaceAndComments() {
    while (true) {
      char c = currentChar();
      if (std::isspace(c)) {
        advance();
      } else if (c == '/' && peekChar(1) == '/') {
        while (currentChar() != '\n' && !isEndOfFile()) {
          advance();
        }
      } else if (c == '/' && peekChar(1) == '*') {
        advance();
        advance();
        while (!(currentChar() == '*' && peekChar(1) == '/') &&
               !isEndOfFile()) {
          advance();
        }
        if (currentChar() == '*' && peekChar(1) == '/') {
          advance();
          advance();
        }
      } else {
        break;
      }
    }
  }

  Token nextToken() {
    skipWhitespaceAndComments();

    if (isEndOfFile()) {
      return Token(TokenType::EndOfFile, "", line, column);
    }

    char c = currentChar();

    if (std::isalpha(c) || c == '_') {
      return parseIdentifierOrKeyword();
    }

    if (std::isdigit(c)) {
      return parseInteger();
    }

    if (isOperator(c)) {
      return parseOperator();
    }

    if (isPunctuation(c)) {
      return parsePunctuation();
    }

    if (c == '"') {
      return parseStringLiteral();
    }

    return Token(TokenType::Invalid, std::string(1, c), line, column);
  }

  Token parseIdentifierOrKeyword() {
    std::string value;
    while (std::isalnum(currentChar()) || currentChar() == '_') {
      value.push_back(currentChar());
      advance();
    }

    if (keywords.count(value)) {
      return Token(TokenType::Keyword, value, line, column);
    } else {
      return Token(TokenType::Identifier, value, line, column);
    }
  }

  Token parseInteger() {
    std::string value;
    while (std::isdigit(currentChar())) {
      value.push_back(currentChar());
      advance();
    }
    return Token(TokenType::Integer, value, line, column);
  }

  Token parseOperator() {
    std::string value(1, currentChar());
    advance();

    if ((value == "+" || value == "-" || value == "*" || value == "/" ||
         value == "=") &&
        currentChar() == '=') {
      value.push_back(currentChar());
      advance();
    }

    return Token(TokenType::Operator, value, line, column);
  }

  Token parsePunctuation() {
    std::size_t startColumn = column;
    char c = currentChar();
    advance();

    TokenType type;
    switch (c) {
    case '(':
      type = TokenType::OpenParen;
      break;
    case ')':
      type = TokenType::CloseParen;
      break;
    case '{':
      type = TokenType::OpenBrace;
      break;
    case '}':
      type = TokenType::CloseBrace;
      break;
    case '[':
      type = TokenType::OpenBracket;
      break;
    case ']':
      type = TokenType::CloseBracket;
      break;
    case ',':
      type = TokenType::Comma;
      break;
    case ';':
      type = TokenType::Semicolon;
      break;
    default:
      return Token(TokenType::Invalid, std::string(1, c), line, startColumn);
    }
    return Token(type, std::string(1, c), line, startColumn);
  }

  Token parseStringLiteral() {
    std::string value;
    advance();
    while (currentChar() != '"' && !isEndOfFile()) {
      value.push_back(currentChar());
      advance();
    }
    if (currentChar() == '"') {
      advance();
      return Token(TokenType::StringLiteral, value, line, column);
    }
    return Token(TokenType::Invalid, value, line, column);
  }

  bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
           c == '>' || c == '<' || c == '!' || c == '&' || c == '|';
  }

  bool isPunctuation(char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' ||
           c == ']' || c == ',' || c == ';';
  }
};

const std::unordered_set<std::string> Lexer::keywords = {
    "auto",     "break",    "case",     "char",   "const",   "continue",
    "default",  "do",       "double",   "else",   "enum",    "extern",
    "float",    "for",      "goto",     "if",     "inline",  "int",
    "long",     "register", "restrict", "return", "short",   "signed",
    "sizeof",   "static",   "struct",   "switch", "typedef", "union",
    "unsigned", "void",     "volatile", "while"};
