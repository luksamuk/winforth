#include <iostream>
#include <deque>
#include <vector>
#include <map>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <fstream>

typedef std::vector<std::string> worddef;
typedef std::pair<std::string, worddef> wordentry;
typedef std::vector<wordentry> dictionary;

static std::deque<long> values;
static dictionary      dict;
static std::deque<std::string> tokenstream;

#define STATUS_PUSHED   0
#define STATUS_EVAL_OP  1
#define STATUS_ERR      2

#define op_eq(i, op) (i.compare(op) == 0)

#define enforce_arity(n) \
if(values.size() < n) { \
	std::cerr << "Stack underflow!" << std::endl; \
	return STATUS_ERR; \
}

bool
_internal_read(std::istream& is)
{
	char buffer[500];
	const char *delim = " \n\t\r";
	
	bool hold = false;
	
	buffer[0] = '\0';
	is.getline(buffer, 500);

	char *next = strtok(buffer, delim);

	if((next == NULL) || (strlen(next) == 0))
		return false;

	do {
		std::string token(next);
		tokenstream.push_back(token);
		next = strtok(NULL, delim);

		// Holding criteria
		if(!hold) {
			if(op_eq(token, ":")){
				hold = true;
			}
		} else {
			if(op_eq(token, ";")) {
				hold = false;
			}
		}
	} while((next != NULL));

	return hold;
}

bool
read(void)
{
	return _internal_read(std::cin);
}

void
token_discard(int n)
{
	while(n--) {
		tokenstream.pop_front();
	}
}

inline bool
has_token(void)
{
	return tokenstream.size() > 0;
}

inline std::string
next_token(void)
{
	std::string token = tokenstream.front();
	tokenstream.pop_front();
	return token;
}

worddef *
dict_find(std::string wordname)
{
	dictionary::iterator it;
	for(it = dict.begin(); it != dict.end(); it++) {
		if(it->first.compare(wordname.c_str()) == 0)
			return &it->second;
	}
	return NULL;
}

bool
str_numeric(std::string s)
{
	std::string::iterator it;
	bool is_first = true;
	for(it = s.begin(); it != s.end(); it++) {
		if(is_first) {
			is_first = false;
			if((*it == '-') && (s.size() > 1))
				continue;
		}
		if(!isdigit(*it))
			return false;
	}
	return true;
}


void
print_stack(void)
{
	std::deque<long>::iterator it;
	for(it = values.begin(); it != values.end(); it++) {
		std::cout << *it << ' ';
	}
	std::cout << '<' << values.size() << '>';
}

void
print_tokens(void)
{
	std::deque<std::string>::iterator it;
	for(it = tokenstream.begin(); it != tokenstream.end(); it++) {
		std::cout << (*it).c_str() << ' ';
	}
	std::cout << '<' << tokenstream.size() << '>';
}

void
gulp_until(const char *token)
{
	for(;;) {
		if(op_eq(next_token(), token)) {
			break;
		}
	}
}

int
gulp_consequent()
{
	std::string input;
	for(;;) {
		input = next_token();
		if(op_eq(input, "then"))
			return 0;
		else if(op_eq(input, "else"))
			return 1;
	}
}



worddef
accum_until(const char *token)
{
	worddef def;
	std::string input;
	for(;;) {
		input = next_token();
		if(op_eq(input, "(")) { // ignore comments
			gulp_until(")");
		} else if(op_eq(input, token)) { // break on token
			break;
		} else {
			// accum other tokens
			def.push_back(std::string(input));
		}
	}
	return def;
}

int
eval(std::string input)
{
	if(str_numeric(input)) {
		values.push_back(strtol(input.c_str(), NULL, 10));
		return STATUS_PUSHED;
	} else {
		if(op_eq(input, "true")) // ( -- -1 )
			values.push_back(-1);
		else if(op_eq(input, "false")) // ( -- 0 )
			values.push_back(0);
		else if(op_eq(input, "+")) { // ( n1 n2 -- r )
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back(op1 + op2);
		} else if(op_eq(input, "-")) { // ( n1 n2 -- r )
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back(op1 - op2);
		} else if(op_eq(input, "*")) { // ( n1 n2 -- r )
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back(op1 * op2);
		} else if(op_eq(input, "/")) { // ( n1 n2 -- r )
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			if(op2 == 0) {
				std::cerr << "Arithmetic exception!";
				return STATUS_ERR;
			}
			values.push_back(op1 / op2);
		} else if(op_eq(input, ".")) { // ( n1 -- ), prints to console
			enforce_arity(1);
			int val = values.back(); values.pop_back();
			std::cout << val /*<< std::endl*/;
		} else if(op_eq(input, ".s")) { // ( -- ), prints to console
			print_stack();
			//std::cout << std::endl;
		} else if(op_eq(input, "bye")) { // ( -- )
			exit(0);
		} else if(op_eq(input, "dup")) { // ( n -- n n )
			enforce_arity(1);
			values.push_back(values.back());
		} else if(op_eq(input, "swap")) { // ( n1 n2 -- n2 n1 )
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back(op2);
			values.push_back(op1);
		} else if(op_eq(input, "rot")) { // ( n1 n2 n3 -- n2 n3 n1 )
			enforce_arity(3);
			int op3 = values.back(); values.pop_back();
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back(op2);
			values.push_back(op3);
			values.push_back(op1);
		} else if(op_eq(input, "over")) { // ( n1 n2 -- n1 n2 n1 )
			enforce_arity(2);
			int val = values[values.size() - 2];
			values.push_back(val);
		} else if(op_eq(input, "drop")) { // ( n1 -- )
			enforce_arity(1);
			values.pop_back();
		} else if(op_eq(input, "<")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back((op1 < op2) ? -1 : 0);
		} else if(op_eq(input, "<=")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back((op1 <= op2) ? -1 : 0);
		} else if(op_eq(input, "<>")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back((op1 != op2) ? -1 : 0);
		} else if(op_eq(input, "=")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back((op1 == op2) ? -1 : 0);
		} else if(op_eq(input, ">")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back((op1 > op2) ? -1 : 0);
		} else if(op_eq(input, ">=")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back((op1 >= op2) ? -1 : 0);
		} else if(op_eq(input, "and")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back(op1 & op2);
		} else if(op_eq(input, "or")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back(op1 | op2);
		} else if(op_eq(input, "xor")) {
			enforce_arity(2);
			int op2 = values.back(); values.pop_back();
			int op1 = values.back(); values.pop_back();
			values.push_back(op1 ^ op2);
		} else if(op_eq(input, "invert")) {
			enforce_arity(1);
			int op = values.back(); values.pop_back();
			values.push_back(~op);
		} else if(op_eq(input, ":")) { // ( : <wordname>... ; )
			std::string input = next_token();
			if(op_eq(input, ";")) {
				std::cerr << "Syntax error!";
				return STATUS_ERR;
			}
			std::string myword(input);

			worddef def = accum_until(";");
			if(def.size() < 1) {
				std::cerr << "Syntax error!";
				return STATUS_ERR;
			}

			dict.push_back(wordentry(myword, def));
			std::cout << myword.c_str() << ' ' /*<< std::endl*/;
		} else if(op_eq(input, "if")) { // ( <p> if <c>... then | <p> if <c>... else <a...> then )
			int predicate = values.back(); values.pop_back();
			if(predicate == 0) {
				if(gulp_consequent()) {
					// We found an "else", so accumulate until "then"
					// and execute expression
					worddef acc = accum_until("then");
					worddef::iterator wit;
					for(wit = acc.begin(); wit != acc.end(); wit++) {
						int result;
						if((result = eval(*wit)) == STATUS_ERR)
							return STATUS_ERR;
					}
				} else {
					// We found a "then", so just finish
					// (do nothing here)
				}
			} else {
				// Accumulate args until "then" or "else"
				worddef acc;
				std::string input;
				bool found_then = false;
				for(;;) {
					input = next_token();
					if(op_eq(input, "then")) {
						found_then = true;
						break;
					} else if(op_eq(input, "else"))
						break;
					acc.push_back(std::string(input));
				}

				// if we found "else", save accumulated words,
				// gulp until we find a "then".
				if(!found_then) {
					gulp_until("then");
				}

				// execute accumulated words
				worddef::iterator wit;
				for(wit = acc.begin(); wit != acc.end(); wit++) {
					int result;
					if((result = eval(*wit)) == STATUS_ERR)
						return STATUS_ERR;
				}	
			}
		} else if(op_eq(input, "(")) { // ( comments )
			gulp_until(")");
		} else { // ( Normal evaluation, find word on dictionary )
			worddef *def = dict_find(input);
			if(def == NULL) {
				std::cerr
					<< input.c_str()
					<< " ?"
					/*<< std::endl*/;
				return STATUS_ERR;
			} else {
				worddef::reverse_iterator wit;
				for(wit = def->rbegin(); wit != def->rend(); wit++) {
					tokenstream.push_front(*wit);
				}
			}
		}
	}

	return STATUS_EVAL_OP;
}

void
load_file(const char *filename)
{
	std::ifstream ifs;
	ifs.open(filename);
	if(ifs.fail()) {
		std::cerr << "Unable to open: " << filename << std::endl;
		return;
	}
	while(ifs.good())
		_internal_read(ifs);

	ifs.close();
}

int
main(int argc, char **argv)
{
	std::cout
		<< "winforth 0.1" << std::endl
		<< "Copyright (c) 2023 Lucas S. Vieira <lucasvieira@protonmail.com>"
		<< std::endl << std::endl
		<< "type 'bye' to quit" << std::endl;

	bool loaded_file = false;

	if(argc > 1) {
		int i;
		for(i = 1; i < argc; i++) {
			std::cout << "loading " << argv[i] << std::endl;
			load_file(argv[i]);
			loaded_file = true;
		}
	}

	int result;
	for(;;) {
		if(!loaded_file) {
			while(read()) ;
		} else {
			loaded_file = false;
		}

		while(has_token()) {
			switch(result = eval(next_token())) {
			case STATUS_ERR:
				std::cout << "Stack trace: ";
				print_stack();
				std::cout << std::endl;
				values.clear();
				tokenstream.clear();
				break;
			case STATUS_EVAL_OP:
				if(loaded_file) std::cout << std::endl;
				break;
			default: break;
			}
		}

		if(result == STATUS_EVAL_OP) {
			std::cout << " ok";
		}
		std::cout << std::endl;
	}

	return 0;
}
