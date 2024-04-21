#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* I think trying to read every tag in the header was a mistake.
 * I think I'm gonna read the first 16 bytes (the variants mainly differ in bytes 5-6
 * If they match a WAV file
 * Read further until I reach the "data" chunk
 * then read the next four bytes to see how many bytes to write to file
 */
// Microsoft Wave
struct waveheader_t {
	// All values, save the name of the chunks, are in little-endian mode
	char *waveChunk; // Beginning of the "fmt " subchunk
	uint32_t waveSectionSize; // 0x01000000 (16) for PCM
	uint16_t waveType; // 0x0100 for linear PCM; other values may be other forms of PCM, such as APCM
	uint16_t channelTotal; // 2 for stereo
	uint32_t samplesPerSecond; // Example: 44100, 48000, 96000
	uint32_t bytesPerSecond; // Usually 176400
	uint16_t blockAlignment; // = channelTotal * bytesPerSample (or bitsPerSampe / 8)
	uint16_t bitsPerSample; // Bit depth
	char *dataChunk; // Beginning of the "data" subchunk
	uint32_t rawDataSizeInBytes; // Size of the whole file minus the header
};
typedef struct waveheader_t waveheader;

int main(){
	int carve = 0;
	while (carve != EOF){
		char riff[12];
		char tempriff[12];
		char cmpriff[] = "RIFF";
		char cmpwave[] = "WAVE";
		
		while(1){
			int queryfound = 0;

			// Start searching here
			for (int i = 0; i < 12 ; i++){
				carve = getc(stdin);
				if (carve == EOF){
					fprintf(stdout, "Reached end of file.\n");
					exit(0);
				}
				riff[11] = carve;
				if (memcmp(riff, cmpriff, 4) == 0 && memcmp(&riff[8], cmpwave, 4) == 0){
					fprintf(stdout, "Found the query at riff[%d-%d]\n", i, i + 11);
					queryfound = 1;
					break;
				}
				// Shift array contents
				for (int b = 0; b < 11; b++){
					riff[b] = riff[b + 1];
				}
			}
			if (queryfound){
				break;
			}
		}

		// Outside searching loop
		for (int k = 0; k < 12; k++){
			fprintf(stdout, "%c", riff[k]);
		}
		waveheader waveaudio;
                // Fill waveChunk 
		for (int l = 0; l < 4; l++){
			getc(stdin);	
		}
		waveaudio.waveChunk = "fmt ";
		//fprintf(stdout, "waveaudio.waveChunk = ");
	       	fprintf(stdout, "%s", waveaudio.waveChunk);
		
		puts("");

		// Final version will read from a file, not standard-in.
		fread(&waveaudio.waveSectionSize, 1, 4, stdin);
		fread(&waveaudio.waveType, 1, 2, stdin);
		fread(&waveaudio.channelTotal, 1, 2, stdin); 
		fread(&waveaudio.samplesPerSecond, 1, 4, stdin);
		fread(&waveaudio.bytesPerSecond, 1, 4, stdin);
		fread(&waveaudio.blockAlignment, 1, 2, stdin);
		fread(&waveaudio.bitsPerSample, 1, 2, stdin);
		waveaudio.dataChunk = "data";
		getc(stdin);
		getc(stdin);
		getc(stdin);
		getc(stdin);
		fread(&waveaudio.rawDataSizeInBytes, 1, 4, stdin); 

		puts("");
		fprintf(stdout, "waveaudio.rawDataSizeInBytes = %d\n", waveaudio.rawDataSizeInBytes);

	}
		return 0;
}
	

