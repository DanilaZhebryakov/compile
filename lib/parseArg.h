#ifndef PARSEARG_H_INCLUDED
#define PARSEARG_H_INCLUDED

const int ARG_NOT_FOUND = -1;

int parseArg(int argc, const char* argv[], const char* arg_to_find);

int parseArgBegin(int argc, const char* argv[], const char* arg_to_find);

char* repl_file_extension(const char* filename, const char* extension);
#endif // PARSEARG_H_INCLUDED
