#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include "dyggr.h"

/* TODO: Formats to be added:
 * DNG (Losing the hundreds of raw images I took with smartphones would depress me.)
 * FLAC (I read the spec a little: this will be more challenging.)
 * Opus (Fun fact: I struggle to tell apart lossless files and low-bitrate Opus files, but the purist in me refuses to delete the former.)
 * PNG
 * PPM (Magic number, P6, too short, so many false positives to be expected.)
 * SVG
 * TeX
 * TIFF (DNG uses the TIFF container, so I should study the latter before progressing to the former.)
 * WavPack (FLAC does not support DSD and float32 PCM, so WavPack is essential (for archiving and processing; no benefit whatsoever to the human ear.))
 */

/* TODO: Features I hope to add in the last stages:
 * Concurrency/threads.
 * Allow the user to enter a magic number and the number of bytes (offset) to be read from the header
 * to the assumed tail. Files will be readable but some readers will require the file size/length to be fixed first.
 * For example, if you append random bytes to a WAV file (`cat /dev/random >> song.wav` and ctrl+c immediately),
 * import the file to Audacity, if you export it to another WAV file the file size/length will be fixed.
 */
/* TODO: Add printing function which will be affected by the verbose/quiet flag,
 * because writing an if-statement before each print statement is naive.
 * Update: Done-ish.
 */
int file_found_greenify_text = 0;
void Usage(void)
{
	fprintf(stdout, "\nUsage: %s -f <file format> -s <source/device to scan>\n\n", progName);
	fprintf(stdout, "File formats: \n");
	for (int i = 0; i < 2; i++) {
		fprintf(stdout, "\t\t\"%s\"\n", validfileformats[i]);
	}
	exit(EXIT_FAILURE);
}

void Greeting(void)
{
	fprintf(stdout, "\nDyggr: recover deleted files.\n\n");
	fprintf(stdout, "Copyright (c) 2024 Fezile Nkuna.\n\n");
	fprintf(stdout, "Dyggr is distributed under the terms of the Apache 2.0 license.\n\n");
}

void checkfileformat(void){
	int isvalid = 0;
	for (int i = 0; i < 2; i++) {
		if (strcasecmp(fileFormat, validfileformats[i]) == 0) {
			isvalid = 1;	
		}
	}
	if (!isvalid){
		prynt(stderr, "Invalid file format.");
		Usage();
	}
}

/* This function prints only if verbose is true.
 * It should be called by the scanning/carving functions,
 * not by functions that check if command-line args are valid.
 *
 * It is pretty primitive at the moment.
 * I think there should be two verbose modes:
 * one that suppresses only messages sent to stdout,
 * and another that suppresses both stdout and stderr.
 * We'll see.
 */
void prynt(FILE *stream, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	
	if (stream == stderr) {
		printf(redtext);
		/* For some reason the text does not change to red 
		 * if I do not print a newline.
		 * And all subsequent prints (stderr and stdout) are affected.
		 * Consider this a feature, for now.
		 */ 
		puts(""); 
	} else if (stream == stdout && file_found_greenify_text) {
		printf(greentext);
	}

	if (verbose) {
		vfprintf(stream, fmt, args);
		fprintf(stream, "\n");
	}

	va_end(args);
	
	/* Prepare (next) search; we don't know if we'll find a(nother) file*/
	printf(resetcolour);
	file_found_greenify_text = 0;

	/* Colour-handling code potentially insecure, according to Clang 14.0.6 */
}

void CommandLineArguments(int argc, char *argv[], FILE **DeviceOrFile)
{
       /* It works, but getopt is better.
	*
	* TODO:
	* Add -v (verbose), or -q (quiet), in dyggr.h 
	* Colorise (red) stderr messages
	* Done-ish.
	*/
        progName = argv[0];
	if (argc < 2) {
		Greeting();
		Usage();
	}
	
	for (int i = 1; i < argc; i++) {
		if (strcasecmp(argv[i], "-s") == 0 || strcasecmp(argv[i], "--source") == 0) {
			sourceargc++;
			if ((i + 1) == argc) {
				prynt(stderr, "No source entered.");
				Usage();
			}
			source = argv[i + 1];
			//  break;
		}
		if (strcasecmp(argv[i], "-f") == 0 || strcasecmp(argv[i], "--file-format") == 0) {
			formatargc++;
			if ((i + 1) == argc) {
				prynt(stderr, "No file format entered.");
				Usage();
			}
			fileFormat = argv[i + 1];
			checkfileformat();
			// break;
		}
	}
	
	if (sourceargc == 0) {
		prynt(stderr, "No source entered.");
		Usage();
	}
	if (formatargc == 0) {
		prynt(stderr, "No file format entered.");
		Usage();
	}
	
	*DeviceOrFile = fopen(source, "r");
	if (*DeviceOrFile == NULL) {
		prynt(stderr, "Could not open %s", source);
		Usage();
	}
}

int main(int argc, char *argv[])
{
	FILE *DeviceOrFile;
	CommandLineArguments(argc, argv, &DeviceOrFile);
	if (strcasecmp(fileFormat, "wav") == 0 || strcasecmp(fileFormat, "wave") == 0) {
		wave(DeviceOrFile);
	}
	if (strcasecmp(fileFormat, "webp") == 0) {
		webp(DeviceOrFile);
	}
	
	return EXIT_SUCCESS;
}

void webp(FILE *DeviceOrFile)
{
	int numFilesRecovered = 0;
	int carve = 0;
	while (carve != EOF) {
		byte riff[12];
		char cmpriff[] = "RIFF";
		char cmpwebp[] = "WEBP";
		uint32_t fileSize;
		char destName[100]; //"Recovered";
		char tempName[1000]; /*"Used to catenate numFilesRecovered to "Recovered"*/
		sprintf(destName, "%d", numFilesRecovered);
		riff[11] = carve;
		while (!(memcmp(riff, cmpriff, 4) == 0 && memcmp(&riff[8], cmpwebp, 4) == 0)) {
			carve = getc(DeviceOrFile);
			if (carve == EOF) {
				prynt(stdout, "End of %s reached.", source);
				exit(EXIT_SUCCESS);
			}
			for (int b = 0; b < 11; b++) {
				riff[b] = riff[b + 1];
			}
			riff[11] = carve;
		}

		/* Used by the prynt function to make text green if a file was found */
		file_found_greenify_text = 1;
		numFilesRecovered++;

		/* fileSize = (((riff[7] << 24) | (riff[6] << 16)) | (riff[5] << 8)) | riff[4]; */
		fileSize = (((((riff[7] << 8) | riff[6]) << 8) | riff[5]) << 8) | riff[4];
		
		/* All the remaining lines have to be converted to a funtion
		 * since I use similar steps in the wave(..) function.
		 */
		sprintf(destName, "Recovered ");
		sprintf(tempName, "%d", numFilesRecovered);
		strcat(destName, tempName);
		strcat(destName, ".webp");

		/* Write to the new file */
		FILE *restoredfile = fopen(destName, "ab");
		/* Write "RIFFfileSizeWEBP" first */
		for (int v = 0; v < 12; v++) {
			fprintf(restoredfile, "%c", riff[v]);
		}
		/* Create buffer.
		 * Minus 4 because the four bytes "WEBP", which are included in the fileSize,
		 * have already been written to the file as part of the RIFF header.
		 */
	       	uint8_t *buffer = (uint8_t *) malloc(fileSize - 4);
		if (buffer != NULL) {
			fread(buffer, 1, fileSize - 4, DeviceOrFile);
			fwrite(buffer, 1, fileSize - 4, restoredfile);
			/* A whole lot of weirdness occurs if carve is not updated */
			carve = getc(DeviceOrFile); 
		} else {
			prynt(stderr, "%sFailed to create %d-byte buffer in memory. Writing one byte a time...%s", redtext, fileSize - 4, resetcolour);
	    
			for (int h = 0; h < fileSize - 4; h++)
			{
				/* Buggy if carve is not updated.
				 * I mast take this into consideration when creating a write
				 * function for wave() and webp().
				 * This is because of the while loops. wave() uses while(1).
				 * Solution: wave() has to use while-loops similar to webp()'s.
				 */
				carve = getc(DeviceOrFile);
				/* Append the raw audio data */
				putc(carve, restoredfile); 
			}
		}

		if (fileSize < 1048576) {
			prynt(stdout, "%sRecovered %d KiB WebP file.", greentext, fileSize >> 10 /* / 1024 */, resetcolour);
		} else {
			prynt(stdout, "Recovered %d MiB WebP file.", fileSize >> 20 /* / 1024^2 */);
		}

		fclose(restoredfile);
		if (carve == EOF) {
			prynt(stdout, "Reached end of %s", source);
		}
	}
}

void wave(FILE *DeviceOrFile)
{
	int numFilesRecovered = 0;
	int carve = 0;
	while (carve != EOF) {
		/*These have to be moved to dyggr.h*/
		uint8_t riff[12];
		char cmpriff[] = "RIFF";
		char cmpwave[] = "WAVE";
		char cmpdata[] = "data";
		char data[4];
		uint32_t rawSizeInBytes;
	
		/* Arbitrary array size for now */
		char destName[100]; //"Recovered";
		char tempName[1000]; // Used to concatenate "Recovered" and numFilesRecovered
		sprintf(destName, "%d", numFilesRecovered);
		
		while(1) {
			carve = getc(DeviceOrFile);
			if (carve == EOF) {
				prynt(stdout, "End of %s reached.", source);
				exit(EXIT_SUCCESS);
			}
			riff[11] = carve;
			if (memcmp(riff, cmpriff, 4) == 0 && memcmp(&riff[8], cmpwave, 4) == 0)  /* WAVE header found. */ {
				file_found_greenify_text = 1;
				/* TODO: Avoid appending bytes to an existing file
				 * if user runs the program multiple times.
				 */
				/* Update restoredfile name here */
				numFilesRecovered++;
				sprintf(destName, "Recovered ");
				sprintf(tempName, "%d", numFilesRecovered);
				strcat(destName, tempName);
				strcat(destName, ".wav");
				
				/* Write to the new file */
				FILE *restoredfile = fopen(destName, "a");
				/* Write "RIFF....WAVE" first */
				for (int v = 0; v < 12; v++) {
					fprintf(restoredfile, "%c", riff[v]);
				}
				while(1) {
					carve = getc(DeviceOrFile);
					if (carve == EOF) {
						prynt(stdout, "End of %s reached.", source);
						exit(EXIT_SUCCESS);
					}
					/* Keep writing bytes until the "data" chunk is reached */
					putc(carve, restoredfile);
					data[3] = carve;
					if (memcmp(data, cmpdata, 4) == 0)  /* "data" chunk found */ {
						fread(&rawSizeInBytes, 1, 4, DeviceOrFile);
						/* Write raw data size
						 * Bit-shifting to the right because of little-endianness
						 * Surely there is a libC function, maybe in <endian.h>, that does this more efficiently.
						 *
						 * Or I could reposition the file position indicator position back 4 bytes
						 * with fseek(..), because this could fail in a big-endian system (not tested).
						 */
						fputc(rawSizeInBytes & 0x000000ff, restoredfile);
						fputc((rawSizeInBytes & 0x0000ff00) >> 8, restoredfile);
						fputc((rawSizeInBytes & 0x00ff0000) >> 16, restoredfile);
						fputc((rawSizeInBytes & 0xff000000) >> 24, restoredfile);
						
						/* Append the raw audio data */
						/* fwrite(..) requires me to first read the data into a buffer in memory.
						 * Simply trying to read from the DeviceOrFile always fails,
						 * even if I try to cast "FILE *" to "uint8_t *".
						 * 
						 * The for-loop immidiately following fwrite(..) is the old version
						 * that appends the raw PCM data to restoredfile.
						 */
						
						/* Potentially use 4 GiB (minus header size) of memory */
						uint8_t *buffer = (uint8_t *) malloc(rawSizeInBytes);
						if (buffer != NULL) {
							fread(buffer, 1, rawSizeInBytes, DeviceOrFile);
							fwrite(buffer, 1, rawSizeInBytes, restoredfile);
						} else {
							prynt(stderr, "Failed to create %d-byte buffer in memory. Writing one byte a time...", rawSizeInBytes);
							for (int h = 0; h < rawSizeInBytes; h++) {
								/* Append the raw audio data, one byte each time we loop */
								putc(getc(DeviceOrFile), restoredfile);
							}
						}
						fclose(restoredfile);
						prynt(stdout, "%d MiB WAVE file recovered.", rawSizeInBytes >> 20); /* /1024^2 */
						break; /* Break loop to avoid trailing bytes */
					}
					/* Search: shift array contents leftward
					 * to avoid overriding, since bytes are written on the last position.
					 */
					for (int m = 0; m < 3; m++) {
						data[m] = data[m + 1];
					}
				}
			}
			/* Shift array contents leftward */
			for (int b = 0; b < 11; b++) {
				riff[b] = riff[b + 1];
			}
		}
	}
}
