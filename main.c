#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	if (argc < 5) goto ret400;
	goto skip400;
	ret400:
		printf("Only %i arguments given when 4 are required!\n", argc-1);
		if (argc <= 1) printf("Error! No from filename given!\n");
		if (argc <= 2) printf("Error! No to filename given!\n");
		if (argc <= 3) printf("Error! No base frequency given!\n");
		if (argc <= 4) printf("Error! No length in seconds given!\n");
		printf("Usage: qbbbe-enc fromfile tofile basefreq seconds\n");
		return 400;
	skip400: {}

	const int sampleRate = 44100;


	FILE *ff = fopen(argv[1], "rb"); //for from file
	if (!ff) {
		printf("404, from file not found!");
		return 404;
	}

	int *bytes = malloc(sizeof(int));
	bytes[0] = atoi(argv[3]);
	int numBytes = 1;
	int b; //for byte
	while ((b = fgetc(ff)) != EOF) {
		bytes = realloc(bytes, sizeof(int)*(numBytes+1));
		bytes[numBytes] = b+atoi(argv[3]);
		numBytes++;
	}

	fclose(ff);

	printf("Done reading source file and outputting frequencies into temp file.\n");

	FILE *tf = fopen(argv[2], "wb");
	FILE *tmp1 = fopen("tmp1", "rb");
	FILE *tmp2 = fopen("tmp2", "wb");

	//start writing file
	//"fmt " chunk
	fprintf(tmp2, "fmt "); //id
	uint32_t sixteen32 = 16; fwrite(&sixteen32, sizeof(uint32_t), 1, tmp2); //remaining size
	uint16_t one16 = 1; fwrite(&one16, sizeof(uint16_t), 1, tmp2); //type(PCM)
	fwrite(&one16, sizeof(uint16_t), 1, tmp2); //#channels
	uint32_t samplerate32 = 44100; fwrite(&samplerate32, sizeof(uint32_t), 1, tmp2); //samplerate
	uint32_t byterate32 = (uint32_t)(samplerate32*one16*16/8); fwrite(&byterate32, sizeof(uint32_t), 1, tmp2); //byterate
	uint16_t blockAlign16 = (uint16_t)(one16*16/8); fwrite(&blockAlign16, sizeof(uint16_t), 1, tmp2); //block align
	uint16_t sixteen16 = 16; fwrite(&sixteen16, sizeof(uint16_t), 1, tmp2); //bits per sample

	//"data" chunk
	fprintf(tmp2, "data"); //id

	fprintf(tf, "RIFF");
	uint32_t zero32 = 0x00000000; fwrite(&zero32, 1, 4, tf);
	fprintf(tf, "WAVE");

	fclose(tmp1);
	fclose(tmp2);
	if (remove("tmp1") != 0) {
		return 500;
	} //else if (remove("tmp2") != 0) {
//		return 500;
//	}
	return 200;
}