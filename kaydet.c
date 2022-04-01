#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSGSZ 128

int main(int argc, char *argv[])
{

	char dataFromProgramC[MSGSZ];	  //Program.c den gelen verilerin tutulduğu array.
	strcpy(dataFromProgramC, "");	  // array ın temizlenmesi.
	read(3, dataFromProgramC, MSGSZ); //pipe üzerinden arraya veri okunması işlemi.

	//Sonuc.txt ye verilerin yazılması işlemleri.
	FILE *filePointer;
	filePointer = fopen("sonuc.txt", "a+");
	if (filePointer == NULL) //Dosyaya ulaşılma kontrolü...............
	{
		printf("File Error!");
		exit(1);
	}
	fprintf(filePointer, "%s\n", dataFromProgramC); //Sonucun sonuc.txt ye yazılması işlemi.
	fclose(filePointer);

	return 0;
}
