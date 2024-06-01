typedef uint8_t byte;
void prynt(FILE *stream, const char *fmt, ...);
void wave(FILE *);
void webp(FILE *);
char *source = "";
char *progName;
char *fileFormat = "";
int sourceargc = 0; // Count instance of '-s'
int formatargc = 0; // Count instance of '-f'
char *validfileformats[] = {"wav", "webp"};
int verbose = 1; // The default

/*Coloured text*/
char *redtext = "\033[31m";
char *greentext = "\033[32m";
char *resetcolour = "\033[0m";
