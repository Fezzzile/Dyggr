#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	int carve = 0;
	if (argc < 2){
		fprintf(stdout, "Buysla: Restore deleted files (currently WAVE only)\n\n");
		fprintf(stdout, "Copyright (c) 2024 Fezile Nkuna\n\n");
		fprintf(stdout, "Usage: %s -s <source>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	while (carve != EOF){
		char riff[12];
		char tempriff[12];
		char cmpriff[] = "RIFF";
		char cmpwave[] = "WAVE";
		char cmpdata[] = "data";
		char data[4];
		uint32_t rawSizeInBytes;
		int numBytesBeforeDataChunk = 0;

		char *source;
	
		for (int i = 1; i < argc; i++){
			if (strcmp(argv[i], "-s") == 0 /*|| strcmp(argv[i], "--source=<source>") == 0*/){

				if ((i + 1) == argc){
					fprintf(stderr, "Enter a source (call Usage(..))\n");
					exit(EXIT_FAILURE);
				}
				source = argv[i + 1];
				break;
			}
		}

		FILE *restoredfile = fopen("Restored.wav", "a"); // Currently all files are appended to this file; need to fix
		FILE *DeviceOrFile = fopen(source /*rgv[1]*/, "r");
		if (DeviceOrFile == NULL){
			fprintf(stderr, "Error opening file. Call Usage(..)\n");
			exit(EXIT_FAILURE);
		}
		
		while(1){
			int queryfound = 0;

			// Start searching here
			        carve = getc(DeviceOrFile);
				if (carve == EOF){
					fprintf(stdout, "End of %s reached.\n", source);
					exit(0);
				}
				riff[11] = carve;
				if (memcmp(riff, cmpriff, 4) == 0 && memcmp(&riff[8], cmpwave, 4) == 0){ /* WAVE header found. */
					// Write header first
					for (int v = 0; v < 12; v++){
						fprintf(restoredfile, "%c", riff[v]);
					}
					while(1){
						carve = getc(DeviceOrFile);
						if (carve == EOF){
							fprintf(stdout, "End of %s reached.\n", source);
							exit(0);
						}

						putc(carve, restoredfile); // Write the bytes between the "fmt " and "data" chunks
						data[3] = carve;
						if (memcmp(data, cmpdata, 4) == 0){
							//puts("Data chunk found");
							fread(&rawSizeInBytes, 1, 4, DeviceOrFile);
							fprintf(stdout, "%d MiB WAVE file found.\n", (rawSizeInBytes/1024)/1024);
							
							// Write raw data size, following "data" chunk. 
							// Bitshifting to the right because of little-endianness.
							// I am sure there's a libC function that does this quicker.
							fputc(rawSizeInBytes & 0x000000ff, restoredfile);
							fputc((rawSizeInBytes & 0x0000ff00) >> 8, restoredfile);
							fputc((rawSizeInBytes & 0x00ff0000) >> 16, restoredfile);
							fputc((rawSizeInBytes & 0xff000000) >> 24, restoredfile);

							for(int h = 0; h < rawSizeInBytes; h++){
								putc(getc(DeviceOrFile), restoredfile); // Append the raw audio data
							}
							break; // Break loop to avoid trailing bytes 
						}
						// Search: shift array contents leftward
						// to avoid overriding, since I write only to the last position
						for (int m = 0; m < 3; m++){
							data[m] = data[m + 1];
						}

					}
				}
				// Shift array contents leftward
				for (int b = 0; b < 11; b++){
					riff[b] = riff[b + 1];
				}
		}

	}
		return EXIT_SUCCESS;
}
