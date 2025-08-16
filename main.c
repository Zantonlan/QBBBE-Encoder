#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

int16_t createSample(int freq, int n, int samplerate, int amplitude) {
	double t = (double)n/samplerate;
	return (int16_t)(amplitude*sin(2.0*M_PI*freq*t));
}

int main(int argc, char* argv[]) {
	if (argc < 5) goto ret400;
	goto skip400;
	ret400:
		printf("Only %i arguments given when 4 are required!\n", argc-1);
		if (argc <= 1) printf("Error! No from filename given!\n");
		if (argc <= 2) printf("Error! No to filename given!\n");
		if (argc <= 3) printf("Error! No base frequency given!\n");
		if (argc <= 4) printf("Error! No length in seconds given!\n");
		printf("Usage: qbbbe-enc fromfile tofile basefreq seconds (freqstep)\n");
		return 400;
	skip400: {}

	const int samplerate = 44100;

	FILE *ff = fopen(argv[1], "rb"); //for from file
	if (!ff) {
		printf("404, from file not found!");
		return 404;
	}

	int freqStep = atoi(argv[5]);
	freqStep = freqStep ? freqStep:1;
	int *freqs = malloc(sizeof(int)*2);
	freqs[0] = atoi(argv[3]);
	freqs[1] = atoi(argv[3])+freqStep;
	int numFreqs = 2;
	int b; //for byte
	while ((b = fgetc(ff)) != EOF) {
		freqs = realloc(freqs, sizeof(int)*(numFreqs+1));
		freqs[numFreqs] = b+atoi(argv[3]);
		numFreqs++;
	}

	fclose(ff);

	printf("Done reading source file and outputting frequencies into temp file.\n");

	FILE *tf = fopen(argv[2], "wb");
	FILE *tmp1 = fopen("tmp1", "rb");
	FILE *tmp2 = fopen("tmp2", "wb");

	//start writing file
	int bytesInFile = 0;
	//"fmt " chunk
	fprintf(tmp2, "fmt "); //id
	uint32_t sixteen32 = 16; fwrite(&sixteen32, sizeof(uint32_t), 1, tmp2); //remaining size
	uint16_t one16 = 1; fwrite(&one16, sizeof(uint16_t), 1, tmp2); //type(PCM)
	fwrite(&one16, sizeof(uint16_t), 1, tmp2); //#channels
	uint32_t samplerate32 = samplerate; fwrite(&samplerate32, sizeof(uint32_t), 1, tmp2); //samplerate
	uint32_t byterate32 = (uint32_t)(samplerate32*one16*16/8); fwrite(&byterate32, sizeof(uint32_t), 1, tmp2); //byterate
	uint16_t blockAlign16 = (uint16_t)(one16*16/8); fwrite(&blockAlign16, sizeof(uint16_t), 1, tmp2); //block align
	uint16_t sixteen16 = 16; fwrite(&sixteen16, sizeof(uint16_t), 1, tmp2); //bits per sample
	bytesInFile += 24;

	//"data" chunk
	int numOfSamples = samplerate*atoi(argv[4]);
	int freqLength = numOfSamples/numFreqs;
	uint16_t amplitude = 32767;
	fprintf(tmp2, "data"); //id
	uint32_t dataLen32 = (uint32_t)(numOfSamples*1*16/8); fwrite(&dataLen32, sizeof(uint32_t), 1, tmp2); //remaining size
	//write in samples
	for (int n = 0; n < numOfSamples; n++) {
		int freq = freqs[0];
		int16_t sample = createSample(freqs[n/freqLength], n, samplerate, amplitude);
		fwrite(&sample, sizeof(int16_t), 1, tmp2);
		bytesInFile += sizeof(int16_t);
	}

	//header
	fprintf(tf, "RIFF"); //RIFF thingy
	uint32_t remLen32 = (uint32_t)bytesInFile+4; fwrite(&remLen32, 1, 4, tf); //remaining size (file-wide)
	fprintf(tf, "WAVE"); //WAVE thingy

	//concatenate tmp2 and test.file


	fclose(tmp1);
	fclose(tmp2);
	if (remove("tmp1") != 0) {
		return 500;
	} //else if (remove("tmp2") != 0) {
//		return 500;
//	}
	return 200;
}