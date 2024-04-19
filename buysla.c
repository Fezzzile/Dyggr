#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct waveheader_t {
	// All values, save the name of the chunks, are in little-endian mode
	char *waveChunk; // "fmt "; // Beginning of the "fmt " subchunk
	uint32_t waveSectionSize; // 0x01000000 (16) for PCM
	uint16_t waveType; // 0x0100 for linear PCM; other values may be other forms of PCM, such as APCM
	uint16_t channelTotal; // 2 for stereo
	uint32_t samplesPerSecond; // Example: 44100, 48000, 96000
	uint32_t bytesPerSecond; // Usually 176400
	uint16_t blockAlignment; // = channelTotal * bytesPerSample (or bitsPerSampe / 8)
	uint16_t bitsPerSample; // Bit depth
	char *dataChunk; // Beginning of the data subchunk
	uint32_t rawdatasizeinbytes; // Size of the whole file minus the header
};
typedef struct waveheader_t waveheader;

int main(){
	int carve = 0;
	while (carve != EOF){
		uint8_t riff[24];
		char cmpriff[] = "RIFF";
		char cmpwave[] = "WAVE";
		//int comparebytes = memcmp(riff, cmpriff, 4) == 0 && memcmp(&riff[8], cmpwave, 4) == 0;
		//int num = 0;
		while(1){
			// Set up much bigger 24-char array
			// Read into riff[11-23]
			// Search for cmpriff
			int queryfound = 0;

			// First copy riff[12-23] to riff[0-11]
			for (int a = 0; a < 12; a++){
				riff[a] = riff[12 + a];
			}
			// Now getchar() only to the second half of riff, that is, riff[12-23]
			for (int b = 12; b < 24; b++){
				carve = getc(stdin);
				if (carve == EOF){
					fprintf(stdout, "Reached end of file.\n");
					exit(0);
				}
				riff[b] = carve;
			}
			// search for cmpriff and cmpwave
			for (int i = 0; i < 24; i++){
				if (memcmp(&riff[i], cmpriff, 4) == 0 && memcmp(&riff[i + 8], cmpwave, 4) == 0){
					fprintf(stdout, "Found the query at riff[%d-%d]\n", i, i + 11);
					queryfound = 1;
					// I should also store the value of i somewhere
					// The code works perfectly
					// Let's store position of i
					// So I can store the next few bytes in the struct
					break;
				}
			}
			if (queryfound){
				break;
			}
		}

		// Outside searching loop
		for (int k = 0; k < 24; k++){
			fprintf(stdout, "%c", riff[k]);
		}
		fprintf(stdout, "\n");

	}
		return 0;
}
	

