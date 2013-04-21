#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <cctype>

namespace TokenTypes
{
	enum eTokenType 
	{
		IDENT,		// L{L|C}
		NUMBER,		// c{c}
		STRING,		// "{dowolny ciag}",
		COMMENT,	// ;{dowolny ciag}
		R_BRACKET,	// ]
		L_BRACKET,	// [
		ASSIGN,
		END_OF_FILE,
		ERROR
	};
};

typedef TokenTypes::eTokenType TokenType;

std::string to_string(TokenType type)
{
	switch(type)
	{
		case TokenTypes::IDENT: return "IDENT";
		case TokenTypes::NUMBER: return "NUMBER";
		case TokenTypes::STRING: return "STRING";
		case TokenTypes::COMMENT: return "COMMENT";
		case TokenTypes::R_BRACKET: return "R_BRACKET";
		case TokenTypes::L_BRACKET: return "L_BRACKET";
		case TokenTypes::ASSIGN: return "ASSIGN";
		case TokenTypes::END_OF_FILE: return "EOF";
		default: return "ERROR";
	}
}

struct Token 
{
	TokenType type;
	std::string content;

	int line;
	int col;
};

// Helper functions.

bool is_char(char a)
{
	return (a>='a' && a<='z') || (a>='A' && a<='Z');
}

bool is_digit(char a)
{
	return a>='0' && a<='9';
}

bool is_space(char a)
{
	return isspace(a) != 0;
}

std::string get_file_contents(const char *filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if(in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

std::string contents;
int pos = 0;
int line = 1;
int col = 1;

bool has_next()
{
	return pos < (int)contents.size();
}

char curr_char()
{
	return contents[pos];
}

char next_char()
{
	char k = contents[++pos];
	if(k == '\n')
	{
		line++;
		col = 0;
	} else 
	{
		col++;
	}
	return k;
}

Token get_next_token()
{
	Token token;

	char c = curr_char();
	while(is_space(c) && has_next()) c = next_char();

	token.line = line;
	token.col = col;

	if(has_next() == false) { token.type = TokenTypes::END_OF_FILE; }
	else if(c == '[') { token.type = TokenTypes::L_BRACKET; token.content = c; next_char();}
	else if(c == ']') { token.type = TokenTypes::R_BRACKET; token.content = c; next_char(); }
	else if(c == '=') { token.type = TokenTypes::ASSIGN;	token.content = c; next_char(); }
	else if(is_digit(c))
	{
		token.type = TokenTypes::NUMBER;
		while(is_digit(c = curr_char()))
		{
			token.content += c;
			next_char();
		}

		/*if(!is_space(c) && c != 0) {
			token.type = TokenTypes::ERROR;
			token.content = "Bad suffix on number";
		}*/
	} else if(c == '\"')
	{
		next_char();
		token.content = "";
		token.type = TokenTypes::STRING;
		while((c = curr_char()) != '\"' && has_next())
		{
			token.content += c;
			next_char();
		}
		if(c != '\"')
		{
			token.type = TokenTypes::ERROR;
			token.content = "Unexpected end of file";
		} else
			next_char();
	} else if(is_char(c))
	{
		token.type = TokenTypes::IDENT;
		while(is_char(c = curr_char()) || is_digit(c))
		{
			token.content += c;
			next_char();
		}
	} else if(c == ';')
	{
		token.type = TokenTypes::COMMENT;
		while((c = curr_char()) != '\n' && c != '\r' && has_next())
		{
			token.content += c;
			next_char();
		}
	} else {
		token.type = TokenTypes::ERROR;
		token.content = "Undeclared identifier: ";
		token.content += c;
	}

	return token;
}

int main() 
{
	contents = get_file_contents("test.ini");
	std::ofstream file("out.txt");

	Token token;
	int i = 0;
	while(true)
	{
		token = get_next_token();

		if(token.type == TokenTypes::END_OF_FILE)
			break;

		file << to_string(token.type) <<" (line="<<token.line<<", col="<<token.col<<")\n"<<token.content<<"\n\n";
	
		if(token.type == TokenTypes::ERROR)
			break;
	}
	return 0;
}