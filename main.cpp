#include <iostream>
#include <deque>
#include <vector>
#include <map>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <fstream>

// Byte sizes:
// - Address unit = 1 byte
// - Cell = 8 bytes

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	typedef __int8    au;
	typedef __int64   cell;
#else
	typedef char          au;
	typedef long long int cell;
#endif

typedef cell   a_addr;
typedef cell   c_addr;

typedef std::vector<std::string>        worddef;
typedef std::pair<std::string, worddef> wordentry;
typedef std::vector<wordentry>          dictionary;

typedef std::pair<std::string, a_addr>  varentry;
typedef std::vector<varentry>           varindex;

static std::deque<cell>        values;
static std::vector<au>         dataspc;
static dictionary              dict;
static varindex                varidx;
static std::string             stringbuffer;

static std::deque<std::string> tokenstream;
static bool nested_do_loop = false;
static bool nested_begin_loop = false;

#define STATUS_PUSHED   0
#define STATUS_EVAL_OP  1
#define STATUS_ERR      2

#define op_eq(i, op) (i.compare(op) == 0)

#define enforce_arity(n) \
if(values.size() < n) { \
	std::cerr << "Stack underflow!"; \
	return STATUS_ERR; \
}

inline bool
is_dataspc_aligned(void)
{
	return ((a_addr)dataspc.size() % (a_addr)sizeof(a_addr)) == 0;
}

a_addr
dataspc_next_aligned(void)
{
	a_addr here    = (a_addr)dataspc.size();
	a_addr size    = (a_addr)sizeof(cell);
	a_addr diff    = here % size;
	a_addr missing = (diff > 0) ? (size - (here % size)) : 0;
	return here + missing;
}

void
dataspc_align(void)
{
	a_addr aligned = dataspc_next_aligned();
	while(dataspc.size() < aligned) {
		dataspc.push_back(0);
	}
}

varentry
make_var(std::string name, a_addr addr)
{
	varentry idx = varentry(name, addr);
	varidx.push_back(idx);
	return idx;
}

inline a_addr
here()
{
	return (a_addr)dataspc.size();
}

int
allot(cell size)
{
	if(size > 0) {
		while(size > 0) {
			dataspc.push_back(0);
			size--;
		}
	} else {
		while(size < 0) {
			if(dataspc.size() == 0) {
				std::cerr << "Access violation!";
				return STATUS_ERR;
			}
			dataspc.pop_back();
			size++;
		}
	}
	return STATUS_EVAL_OP;
}

bool
_internal_read(std::istream& is)
{
	char buffer[500];
	const char *delim = " \n\t\r";
	
	bool hold = false;
	bool read_string_next = false;
	
	buffer[0] = '\0';
	is.getline(buffer, 500);

	char *next = strtok(buffer, delim);

	if((next == NULL) || (strlen(next) == 0))
		return false;

	do {
		// Process current token
		std::string token(next);
		tokenstream.push_back(token);

		// Set up next token
		if(op_eq(token, "s\"") || op_eq(token, ".\"")) {
			// Gulp an entire string token
			read_string_next = true;
			next = strtok(NULL, "\"");
		} else {
			read_string_next = false;
			next = strtok(NULL, delim);
		}

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

cell
var_find(std::string varname)
{
	varindex::iterator it;
	for(it = varidx.begin(); it != varidx.end(); it++) {
		if(it->first.compare(varname.c_str()) == 0)
			return it->second;
	}
	return -1;
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
	std::deque<cell>::iterator it;
	for(it = values.begin(); it != values.end(); it++) {
		printf("%lld", *it);
		putchar(32);
	}
	printf("<%lld>", values.size());
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

bool
load_file(const char *filename)
{
	std::ifstream ifs;
	ifs.open(filename);
	if(ifs.fail()) {
		std::cerr << "Unable to open: " << filename << std::endl;
		return false;
	}
	while(ifs.good())
		_internal_read(ifs);

	ifs.close();
	return true;
}

int
eval(std::string input)
{
	/* Default numeric values */
	if(str_numeric(input)) {
		values.push_back(strtol(input.c_str(), NULL, 10));
		return STATUS_PUSHED;
	} else {
		/* CONSTANTS */
		if(op_eq(input, "true")) // ( -- -1 )
			values.push_back(-1);
		else if(op_eq(input, "false")) // ( -- 0 )
			values.push_back(0);
		else if(op_eq(input, "cell")) // ( -- s )
			values.push_back((a_addr)(sizeof(cell)));


		/* PRIMITIVE WORDS */
		else if(op_eq(input, "+")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back(op1 + op2);
		} else if(op_eq(input, "-")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back(op1 - op2);
		} else if(op_eq(input, "*")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back(op1 * op2);
		} else if(op_eq(input, "/")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			if(op2 == 0) {
				std::cerr << "Arithmetic exception!";
				return STATUS_ERR;
			}
			values.push_back(op1 / op2);
		} else if(op_eq(input, ".")) { // ( n1 -- ), prints to console
			enforce_arity(1);
			cell val = values.back(); values.pop_back();
			printf("%lld", val);
		} else if(op_eq(input, ".s")) { // ( -- ), prints to console
			print_stack();
		} else if(op_eq(input, "bye")) { // ( -- )
			std::cerr << "Quaerendo invenietis." << std::endl;
			exit(0);
		} else if(op_eq(input, "dup")) { // ( n -- n n )
			enforce_arity(1);
			values.push_back(values.back());
		} else if(op_eq(input, "swap")) { // ( n1 n2 -- n2 n1 )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back(op2);
			values.push_back(op1);
		} else if(op_eq(input, "rot")) { // ( n1 n2 n3 -- n2 n3 n1 )
			enforce_arity(3);
			cell op3 = values.back(); values.pop_back();
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back(op2);
			values.push_back(op3);
			values.push_back(op1);
		} else if(op_eq(input, "over")) { // ( n1 n2 -- n1 n2 n1 )
			enforce_arity(2);
			cell val = values[values.size() - 2];
			values.push_back(val);
		} else if(op_eq(input, "drop")) { // ( n1 -- )
			enforce_arity(1);
			values.pop_back();
		} else if(op_eq(input, "<")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back((op1 < op2) ? -1 : 0);
		} else if(op_eq(input, "<=")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back((op1 <= op2) ? -1 : 0);
		} else if(op_eq(input, "<>")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back((op1 != op2) ? -1 : 0);
		} else if(op_eq(input, "=")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back((op1 == op2) ? -1 : 0);
		} else if(op_eq(input, ">")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back((op1 > op2) ? -1 : 0);
		} else if(op_eq(input, ">=")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back((op1 >= op2) ? -1 : 0);
		} else if(op_eq(input, "and")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back(op1 & op2);
		} else if(op_eq(input, "or")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back(op1 | op2);
		} else if(op_eq(input, "xor")) { // ( n1 n2 -- r )
			enforce_arity(2);
			cell op2 = values.back(); values.pop_back();
			cell op1 = values.back(); values.pop_back();
			values.push_back(op1 ^ op2);
		} else if(op_eq(input, "invert")) { // ( n -- r )
			enforce_arity(1);
			cell op = values.back(); values.pop_back();
			values.push_back(~op);
		} else if(op_eq(input, "emit")) { // ( c -- )
			// print char to console
			char c = (char)values.back(); values.pop_back();
			std::cout << c;
		} else if(op_eq(input, "!")) { // ( <value> <var&> ! )
			enforce_arity(2);
			a_addr var = values.back(); values.pop_back();
			cell   val = values.back(); values.pop_back();
			if((var < 0) || (var >= dataspc.size())) {
				std::cerr << "Access violation!";
				return STATUS_ERR;
			}
			
			// This assumes that an std::vector is contiguous.
			cell *varptr = (cell*)&dataspc[var];
			*varptr = val;
		} else if(op_eq(input, "@")) { // ( <var&> @ -- n )
			enforce_arity(1);
			a_addr var = values.back(); values.pop_back();
			if((var < 0) || (var >= dataspc.size())) {
				std::cerr << "Invalid variable!";
				return STATUS_ERR;
			}
			cell *c = (cell*)&dataspc[var];
			values.push_back(*c);
		} else if(op_eq(input, "here")) { // ( -- n )
			values.push_back(here());
		} else if(op_eq(input, "allot")) { // ( n -- )
			enforce_arity(1);
			cell size = values.back(); values.pop_back();
			return allot(size);
		} else if(op_eq(input, "align")) { // ( -- )
			dataspc_align();
		} else if(op_eq(input, "aligned")) { // ( -- a-addr )
			values.push_back((a_addr)dataspc_next_aligned());
		} else if(op_eq(input, "s\"")) { // ( -- )
			stringbuffer = next_token();
			// Gforth pushes a string address and its size
			// onto the stack, but we won't do this here because
			// the string is not being held at a manipulable memory
			// location. :^P
		} else if(op_eq(input, ".\"")) { // ( -- )
			std::cout << next_token().c_str();
		} else if(op_eq(input, "loadfile")) { // NON-STANDARD
			if(stringbuffer.compare("") == 0) {
				std::cerr << "String buffer is empty!";
				return STATUS_ERR;
			}
			// Stores current token stream elsewhere
			std::deque<std::string> copystream(tokenstream);
			tokenstream.clear();
			// Reads file path from stringbuffer.
			if(!load_file(stringbuffer.c_str()))
				return STATUS_ERR;

			while(has_token()) {
				int result;
				result = eval(next_token());
				if(result == STATUS_ERR) {
					// Return error and purposely lose
					// copystream on the process
					return result;
				}
			}
			
			// Recover stream
			tokenstream = copystream;

		/* SPECIAL FORMS-LIKE WORDS */
		} else if(op_eq(input, "constant")) {
			a_addr addr      = values.back(); values.pop_back();
			std::string name = next_token();
			varentry idx = make_var(name, addr);
			std::cout << name.c_str() << ' ';

		} else if(op_eq(input, ":")) {
			// ( : <wordname>... ; )

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
			std::cout << myword.c_str() << ' ';

		} else if(op_eq(input, "if")) {
			// ( <p> if <c>... then )
			// ( <p> if <c>... else <a...> then )
			cell predicate = values.back(); values.pop_back();
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

		} else if(op_eq(input, "(")) {
			// ( comments )
			gulp_until(")");
		} else if(op_eq(input, "do")) { // ( <lim> <idx> do <a...> loop
			enforce_arity(2);
			if(nested_do_loop) {
				std::cerr << "Nested loops are not supported!";
				nested_do_loop = false;
				return STATUS_ERR;
			}

			nested_do_loop = true;
			cell idx   = values.back(); values.pop_back();
			cell limit = values.back(); values.pop_back();

			// Accumulate until "loop" keyword
			worddef acc = accum_until("loop");
			tokenstream.push_front(std::string("loop"));
			while(idx != limit) {
				worddef::reverse_iterator wit;
				for(wit = acc.rbegin(); wit != acc.rend(); wit++) {
					tokenstream.push_front(*wit);
				}

				// Evaluate until next token is "loop"
				while(!op_eq(tokenstream.front(), "loop")) {
					int result = eval(next_token());
					if(result == STATUS_ERR) {
						nested_do_loop = false;
						return STATUS_ERR;
					}
				}

				idx++;
			}
			nested_do_loop = false;
			// Consume "loop"
			next_token();
		} else if(op_eq(input, "begin")) { // begin <a...> <p> until
			if(nested_begin_loop) {
				std::cerr << "Nested loops are not supported!";
				nested_begin_loop = false;
				return STATUS_ERR;
			}

			nested_begin_loop = true;
			// Accumulate until "until" keyword
			worddef acc = accum_until("until");
			tokenstream.push_front(std::string("until"));
			cell pred = 0;
			do {
				worddef::reverse_iterator wit;
				for(wit = acc.rbegin(); wit != acc.rend(); wit++) {
					tokenstream.push_front(*wit);
				}

				while(!op_eq(tokenstream.front(), "until")) {
					int result = eval(next_token());
					if(result == STATUS_ERR) {
						nested_begin_loop = false;
						return STATUS_ERR;
					}
				}

				// Evaluate top of stack
				enforce_arity(1);
				pred = values.back(); values.pop_back();
			} while(pred != -1);
			nested_begin_loop = false;
			// Consume "until"
			next_token();


		/* DEFAULT EVALUATION */
		} else {
			cell varaddr;
			worddef *def;
			// Find variable on index
			if((varaddr = var_find(input)) >= 0) {
				values.push_back(varaddr);
			}
			// Find word on dictionary
			else if((def = dict_find(input)) == NULL) {
				std::cerr
					<< input.c_str()
					<< " ??"
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

/*
int
init_defs(void)
{
	int result;
	a_addr aux;

	// Word "i"
	aux = here();
	result = allot(sizeof(cell));
	if(result == STATUS_ERR)
		return STATUS_ERR;
	make_var(std::string("i"), aux);
}
*/

int
main(int argc, char **argv)
{
	std::cout
		<< "Winforth 0.1" << std::endl
		<< "Copyright (c) 2023 Lucas S. Vieira"
		<< std::endl
		<< "type 'bye' to quit" << std::endl << std::endl;

	// Reserve 1024 cells at startup
	dataspc.reserve(1024 * sizeof(cell));

	//init_defs();

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
				std::cout << std::endl << "Stack dump: ";
				print_stack();
				values.clear();
				tokenstream.clear();
				stringbuffer = "";
				break;
			case STATUS_EVAL_OP:
				if(loaded_file)
					std::cout << std::endl;
				break;
			default: break;
			}
		}

		if(result != STATUS_ERR) {
			std::cout << " ok";
		} else {
			std::cerr << '\a';
		}
		std::cout << std::endl;
	}

	return 0;
}
