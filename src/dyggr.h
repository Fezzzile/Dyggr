typedef uint8_t byte;
void wave(FILE *);
void webp(FILE *);
char *source = "";
char *progName;
char *fileFormat = "";
int sourceargc = 0; // Count instance of '-s'
int formatargc = 0; // Count instance of '-f'
char *validfileformats[] = {"wav", "webp"};
int verbose = 1;
