#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Formats to be added:
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

/* Features I hope to add in the last stages:
 * Concurrency/threads.
 * Allowing the user to enter a magic number and the number of bytes (offset) to be read from the header
 * to the assumed tail. Files will be readable but some readers will require the file size/length to be fixed first.
 * For example, if you append random bytes to a WAV file (`cat /dev/random >> song.wav` and ctrl+c immediately),
 * import the file to Audacity, if you export it to another WAV file the file size/length will be fixed.
 */

typedef uint8_t byte;
void wave(FILE *);
void webp(FILE *);
char *source;
char *progName;
char *fileFormat;

void Usage(void)
{
    fprintf(stdout, "\nRecover deleted files (currently WAV & WebP only).\n\n");
    fprintf(stdout, "Usage: %s -s source\n\n", progName);
    fprintf(stdout, "Copyright (c) 2024 Fezile Nkuna.\n\n");
    fprintf(stdout, "%s is distributed under the terms of the Apache 2.0 license.\n", progName);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    progName = argv[0];
    /* It is easy to break this arguments source code.
     * Will fix it later. getopt?
     *
     * To be added:
     * -v (verbose) should be off by default because printing to stdout slows down the program.
     *  Would fflushing(stdout) give the output some speed?
     */
    if (argc < 3)
    {
        Usage();
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-s") == 0 /*|| strcmp(argv[i], "--source=<source>") == 0*/)
        {
            if ((i + 1) == argc)
            {
                fprintf(stderr, "Enter source from which to recover files.\n");
                Usage();
            }
            source = argv[i + 1];
            //  break;
        }

        if (strcmp(argv[i], "-f") == 0)
        {
            if ((i + 1) == argc)
            {
                fprintf(stderr, "Enter file format.\n");
                Usage();
            }
            fileFormat = argv[i + 1];
            //	break;
        }
    }
    if (strlen(source) == 0)
    {
        fprintf(stderr, "\nError: No source entered.\n");
        Usage();
    }
    if (strlen(fileFormat) == 0)
    {
        fprintf(stderr, "\nError: No file format entered.\n");
        Usage();
    }
    FILE *DeviceOrFile = fopen(source, "r");
    if (DeviceOrFile == NULL)
    {
        fprintf(stderr, "\nError: Could not open %s.\n", source);
        Usage();
    }

    if (strcmp(fileFormat, "wave") == 0)
    {
        wave(DeviceOrFile);
    }
    if (strcmp(fileFormat, "webp") == 0)
    {
        webp(DeviceOrFile);
    }

    return EXIT_SUCCESS;
}

void webp(FILE *DeviceOrFile)
{
    int numFilesRecovered = 0;
    int carve = 0;
    while (carve != EOF)
    {
        byte riff[12];
        char tempriff[12];
        char cmpriff[] = "RIFF";
        char cmpwebp[] = "WEBP";
        uint32_t fileSize;
        char destName[100]; //"Recovered";
        char tempName[1000]; /*"Used to catenate numFilesRecovered to "Recovered"*/
        sprintf(destName, "%d", numFilesRecovered);
        riff[11] = carve;

        while (!(memcmp(riff, cmpriff, 4) == 0 && memcmp(&riff[8], cmpwebp, 4) == 0))
        {
            carve = getc(DeviceOrFile);
            if (carve == EOF)
            {
                fprintf(stdout, "End of %s reached.\n", source);
                exit(EXIT_SUCCESS);
            }
            for (int b = 0; b < 11; b++)
            {
                riff[b] = riff[b + 1];
            }
            riff[11] = carve;
        }

        numFilesRecovered++;
        //fileSize = (((riff[7] << 24) | (riff[6] << 16)) | (riff[5] << 8)) | riff[4];
        fileSize = (((((riff[7] << 8) | riff[6]) << 8) | riff[5]) << 8) | riff[4];

        /* All the remaining lines have to be converted to a funtion
         * since I use the same steps in the wave(..) function.
         */
        sprintf(destName, "Recovered ");
        sprintf(tempName, "%d", numFilesRecovered);
        strcat(destName, tempName);
        strcat(destName, ".webp");

        /* Write to the new file */
        FILE *restoredfile = fopen(destName, "ab");

        /* Write "RIFFfileSizeWEBP" first */
        for (int v = 0; v < 12; v++)
        {
            fprintf(restoredfile, "%c", riff[v]);
        }
        /* Minus 4 because the four bytes "WEBP", which are included in the fileSize,
         * have already been written to the file as part of the RIFF header.
         */
        /* I used fread(..), to buffer, and fwrite(..), to restoredfile, in the wave(..) function. */
        for (int i = 0; i < fileSize - 4; i++)
        {
            carve = getc(DeviceOrFile);
            putc(carve, restoredfile);
        }
        if (fileSize < 1048576)
        {
            fprintf(stdout, "Recovered %d KiB WebP file.\n", fileSize >> 10);
        }
        else
        {
            fprintf(stdout, "Recovered %d MiB WebP file.\n", fileSize >> 20 /* / 1024^2 */);
        }

        fclose(restoredfile);

        if (carve == EOF)
        {
            fprintf(stdout, "Reached end of %s\n", source);
        }
    }
}
void wave(FILE *DeviceOrFile)
{
    int numFilesRecovered = 0;
    int carve = 0;

    while (carve != EOF)
    {
        uint8_t riff[12];
        char tempriff[12];
        char cmpriff[] = "RIFF";
        char cmpwave[] = "WAVE";
        char cmpdata[] = "data";
        char data[4];
        uint32_t rawSizeInBytes;

        /* Arbitrary array size for now */
        char destName[100]; //"Recovered";
        char tempName[1000]; // Used to concatenate "Recovered" and numFilesRecovered
        sprintf(destName, "%d", numFilesRecovered);

        while(1)
        {
            carve = getc(DeviceOrFile);
            if (carve == EOF)
            {
                fprintf(stdout, "End of %s reached.\n", source);
                exit(EXIT_SUCCESS);
            }
            riff[11] = carve;
            if (memcmp(riff, cmpriff, 4) == 0 && memcmp(&riff[8], cmpwave, 4) == 0)  /* WAVE header found. */
            {
                /* To be added: Avoid appending bytes to an existing file
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
                for (int v = 0; v < 12; v++)
                {
                    fprintf(restoredfile, "%c", riff[v]);
                }
                while(1)
                {
                    carve = getc(DeviceOrFile);
                    if (carve == EOF)
                    {
                        fprintf(stdout, "End of %s reached.\n", source);
                        exit(EXIT_SUCCESS);
                    }
                    /* Keep writing bytes until the "data" chunk is reached */
                    putc(carve, restoredfile);
                    data[3] = carve;
                    if (memcmp(data, cmpdata, 4) == 0)  /* "data" chunk found */
                    {
                        fread(&rawSizeInBytes, 1, 4, DeviceOrFile);

                        /* Write raw data size
                         * Bit-shifting to the right because of little-endianness
                         * Surely there is a libC function that does this more efficiently.
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
                         * The for_loop immidiately following fwrite(..) is the old version
                         * that appends the raw PCM data to restoredfile.
                         * I'll have to do some tests to check which is faster.
                         */

                        /* Potentially use 4 GiB (minus header size) of memory,
                         * in which case the non-buffered version below, putc(..), may be more appropriate.
                         */
                        uint8_t *buffer = (uint8_t *) malloc(rawSizeInBytes);
                        fread(buffer, 1, rawSizeInBytes, DeviceOrFile);
                        fwrite(buffer, 1, rawSizeInBytes, restoredfile);

                        /*for(int h = 0; h < rawSizeInBytes; h++)
                        {
                            putc(getc(DeviceOrFile), restoredfile); // Append the raw audio data
                        }*/

                        fclose(restoredfile);
                        fprintf(stdout, "%d MiB WAVE file recovered.\n", rawSizeInBytes >> 20); /* /1024^2 */
                        break; /* Break loop to avoid trailing bytes */
                    }
                    /* Search: shift array contents leftward
                     * to avoid overriding, since bytes are written on the the last position.
                     */
                    for (int m = 0; m < 3; m++)
                    {
                        data[m] = data[m + 1];
                    }
                }
            }
            /* Shift array contents leftward
            */
            for (int b = 0; b < 11; b++)
            {
                riff[b] = riff[b + 1];
            }
        }
    }
}
