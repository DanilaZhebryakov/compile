

#define DEFAULT_ARG_DESCRIPTION          \
"-q  : quiet (no output to stderr) \n"  \
"-v  : verbose \n"                      \
"-h, --help : print this info \n"       \

struct BasicProgramRunFlags{
    bool msglvl:2 ; // 0=quiet 1=default 2+ =verbose
};

static BasicProgramRunFlags basic_program_flags = {1};

#define IF_NQ(...) if(basic_program_flags.msglvl > 0) { __VA_ARGS__ }
#define IF_VB(...) if(basic_program_flags.msglvl > 1) { __VA_ARGS__ }

#define HANDLE_DEFAULT_ARGS {              \
    if (parseArg(argc, argv, "--help") >= 0 || parseArg(argc, argv, "-h") >= 0 ) { \
        puts(help_str);                     \
        return 0;                           \
    }                                       \
    if (parseArg(argc, argv, "-v") >= 0) {  \
        basic_program_flags.msglvl = 2;           \
    }                                       \
    if (parseArg(argc, argv, "-q") >= 0) {  \
        basic_program_flags.msglvl = 0;           \
    }                                       \
}
