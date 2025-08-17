#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#define bufferSize 65536

//give a file that is set to rb perms, an int* to store in, and the number of items in the array
int** readFileBBB(FILE *rf, int *arr, int items) {
	int b; //for byte
	while ((b = fgetc(rf)) != EOF) {
		arr = realloc(arr, sizeof(int)*(items+1));
		arr[items] = b;
		items++;
	}

	int **retVal = malloc(sizeof(int*)*2);
	retVal[0] = arr; retVal[1] = &items;
	return retVal;
}

int16_t createSample(int freq, int n, int samplerate, int amplitude) {
	double t = (double)n/samplerate;
	return (int16_t)(amplitude*sin(2.0*M_PI*freq*t));
}

int main(int argc, char* argv[]) {
	if (!(argc > 1)) goto ret400;
	else if ((argc < 4 && strcmp(argv[1], "decode") == 0) || (argc < 6 && strcmp(argv[1], "encode") == 0)) goto ret400;
	else goto skip400;
	ret400:
		if (argc >= 2) {
			if (strcmp(argv[1], "encode") == 0) printf("Only %i arguments given when 5 are required!\n", argc-1);
			else if (strcmp(argv[1], "decode") == 0) printf("Only %i arguments given when 3 are required!\n", argc-1);
			else printf("Only %i arguments given when 5 are required for encoding and 3 are required for decoding!\n", argc-1);
		} else printf("Only %i arguments given when 5 are required for encoding and 3 are required for decoding!\n", argc-1);
		if (argc <= 1) printf("Error! No mode given! Use \"encode\" or \"decode\" (without the quotes).\n");
		if (argc <= 2) printf("Error! No from filename given!\n");
		if (argc <= 3) printf("Error! No to filename given!\n");
		if (argc >= 2 && strcmp(argv[1], "encode") == 0) {
			if (argc <= 4) printf("Error! No base frequency given!\n");
			if (argc <= 5) printf("Error! No length in seconds given!\n");
		}
		printf("\nUsage:\nqbbbe-enc encode fromfile tofile basefreq seconds (freqstep)\nqbbbe-enc decode fromfile tofile\n");
		return 400;
	skip400:
	
	if (strcmp(argv[1], "encode") == 0) goto encode;
	else if (strcmp(argv[1], "decode") == 0) goto decode;
	else goto ret400;

	encode: {
		printf("Beginning encoding process...\n");

		const int samplerate = 44100;

		FILE *ff = fopen(argv[2], "rb"); //for from file
		if (!ff) {
			printf("404, from file not found!\n");
			return 404;
		}

		int freqStep = atoi(argv[6]);
		freqStep = freqStep ? freqStep:1;
		int *freqs = malloc(sizeof(int)*3);
		freqs[0] = atoi(argv[4]);
		freqs[1] = atoi(argv[4])+freqStep;
		freqs[2] = atoi(argv[4])+(freqStep*atoi(argv[5]));
		int numFreqs = 3;
		int **freqData = readFileBBB(ff, freqs, numFreqs);
		freqs = freqData[0];
		numFreqs = *freqData[1];
		int b; //for byte
		for (int i = 0; i < numFreqs; i++) {
			freqs[i] = (freqs[i]*freqStep)+atoi(argv[4]);
		}
		printf("Modified frequencies\n");

		fclose(ff);

		printf("Done reading source file and storing required frequencies into ram.\n");

		FILE *tf = fopen(argv[3], "wb");
		FILE *tmp = fopen("tmp", "wb");

		//start writing file
		int bytesInFile = 0;
		//"fmt " chunk
		fprintf(tmp, "fmt "); //id
		uint32_t sixteen32 = 16; fwrite(&sixteen32, sizeof(uint32_t), 1, tmp); //remaining size
		uint16_t one16 = 1; fwrite(&one16, sizeof(uint16_t), 1, tmp); //type(PCM)
		fwrite(&one16, sizeof(uint16_t), 1, tmp); //#channels
		uint32_t samplerate32 = samplerate; fwrite(&samplerate32, sizeof(uint32_t), 1, tmp); //samplerate
		uint32_t byterate32 = (uint32_t)(samplerate32*one16*16/8); fwrite(&byterate32, sizeof(uint32_t), 1, tmp); //byterate
		uint16_t blockAlign16 = (uint16_t)(one16*16/8); fwrite(&blockAlign16, sizeof(uint16_t), 1, tmp); //block align
		uint16_t sixteen16 = 16; fwrite(&sixteen16, sizeof(uint16_t), 1, tmp); //bits per sample
		bytesInFile += 24;

		//"data" chunk
		int numOfSamples = samplerate*atoi(argv[5]);
		int freqLength = numOfSamples/numFreqs;
		uint16_t amplitude = 32767;
		fprintf(tmp, "data"); //id
		uint32_t dataLen32 = (uint32_t)(numOfSamples*1*16/8); fwrite(&dataLen32, sizeof(uint32_t), 1, tmp); //remaining size
		//write in samples
		for (int n = 0; n < numOfSamples; n++) {
			int freq = freqs[0];
			int16_t sample = createSample(freqs[n/freqLength], n, samplerate, amplitude);
			fwrite(&sample, sizeof(int16_t), 1, tmp);
			bytesInFile += sizeof(int16_t);
		}

		printf("Done temporarily storing data in tmp file.\n");

		//header
		fprintf(tf, "RIFF"); //RIFF thingy
		uint32_t remLen32 = (uint32_t)bytesInFile+4; fwrite(&remLen32, 1, 4, tf); //remaining size (file-wide)
		fprintf(tf, "WAVE"); //WAVE thingy

		printf("Done writing header into final file.\nBeginning to copy tmp data into final file.\n");

		//concatenate tmp and test.file
		fclose(tmp);
		tmp = fopen("tmp", "rb");
		unsigned char buf[bufferSize];
		size_t n;
		while ((n = fread(buf, 1, bufferSize, tmp)) > 0) {
			fwrite(buf, 1, n, tf);
		}

		printf("Done copying tmp data into final file.\nCleaning up...\n");

		fclose(tmp);
		if (remove("tmp") != 0) {
			return 500;
		}
		//	free(buf);
		free(freqs);
		return 200;
	}

	decode: {

	}
}