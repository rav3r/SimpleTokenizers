#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <cctype>
#include <algorithm>

namespace TokenTypes
{
	enum eTokenType 
	{
		IDENT,		// L{L|C}
		NUMBER,		// c{c}
		OP,
		WHITESPACE,
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
		case TokenTypes::OP: return "OP";
		case TokenTypes::WHITESPACE: return "WHITESPACE";
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

bool contains(const char* word, const char* words[], int wordsCount)
{
	std::string data = word;
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);

	for(int i=0; i<wordsCount; i++)
	{
		if(data == words[i])
			return true;
	}
	return false;
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
	return pos < contents.size();
}

char curr_char()
{
	return contents[pos];
}

char next_char()
{
	char k = contents[++pos];
	if(k == '\n') {
		line++;
		col = 0;
	} else {
		col++;
	}
	return k;
}

Token get_next_token()
{
	Token token;

	char c = curr_char();
	while(is_space(c) && has_next())
	{
		token.content = c;
		if(c == '\r') { c = next_char(); continue; };

		token.line = line;
		token.col = col;
		token.type = TokenTypes::WHITESPACE;

		c = next_char();
		return token;
	}

	token.line = line;
	token.col = col;

	std::string ops = ".=,;:#<>+-*/()";

	if(has_next() == false) { token.type = TokenTypes::END_OF_FILE; }
	else if(ops.find(c) != std::string::npos) {token.type = TokenTypes::OP; token.content = curr_char(); next_char(); }
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
	} else if(is_char(c))
	{
		token.type = TokenTypes::IDENT;
		while(is_char(c = curr_char()) || is_digit(c))
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
	contents = get_file_contents("test.txt");
	Token token;
	int i = 0;

	std::ofstream file("index.html");
	file << "<html><body style=\"color:#ffffff; background:#000000\"><pre>";


	const int keywords_count = 11;
	const char *keywords[keywords_count] = {"const", "var", "procedure", "call", "begin", "end", "if", "then", "while", "do", "odd"};

	TokenType lastToken = TokenTypes::WHITESPACE;

	while(true)
	{
		token = get_next_token();

		std::cout <<to_string(token.type) <<" at line "<<token.line<<" col "<<token.col<<"\n"<<token.content<<"\n\n";

		if(token.type == TokenTypes::IDENT && contains(token.content.c_str(), keywords, keywords_count))
			file << "<b><span style=\"color:#ff00ff\">"<<token.content<<"</span></b>";
		else if(token.type == TokenTypes::NUMBER)
			file << "<span style=\"color:#00ff00\">"<<token.content<<"</span>";
		else if(token.type == TokenTypes::OP)
		{
			if(token.content == ">") token.content =  "&gt";
			if(token.content == "<") token.content =  "&lt";
			file << "<span style=\"color:#ffff00\">"<<token.content<<"</span>";
		}
		else if(token.type == TokenTypes::IDENT || token.type == TokenTypes::WHITESPACE)
			file << token.content;
		else
			file << "<span style=\"color:#ff0000\">"<<token.content<<"</span>";
		

		if(token.type == TokenTypes::END_OF_FILE || token.type == TokenTypes::ERROR)
			break;
	}

	file <<"</pre></body></html>";

	return 0;
}