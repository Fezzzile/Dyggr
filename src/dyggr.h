typedef uint8_t byte;
void prynt(FILE *stream, const char *fmt, ...);
void wave(FILE *);
void webp(FILE *);
char *source = "";
char *progName;
char *fileFormat = "";
char *validfileformats[] = {"wav", "webp"};
int progress_should_be_printed = 1; // The default
int error_messages_should_be_ignored = 0;

/*Coloured text*/
int text_should_be_coloured = 0;
char *redtext = "\033[31m";
char *greentext = "\033[32m";
char *resetcolour = "\033[0m";
int file_found_greenify_text = 0;

