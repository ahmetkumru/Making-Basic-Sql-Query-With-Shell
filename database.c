#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSGSZ 128

// Queryimizi boşluğa göre parse etmemizi sağlayan fonksiyonumuz. İlk ödevde kullandığım fonksiyondan yararlandım.
void parseQueryLine(char *query, char **parsedQueryArray)
{

	for (int i = 0; i < 100; i++)
	{
		parsedQueryArray[i] = strsep(&query, " ");

		if (parsedQueryArray[i] == NULL)
			break;
		if (strlen(parsedQueryArray[i]) == 0)
			i--;
	}
}

// Data transferi için kullanılacak struct yapısı. Laboratuvar örneklerimizden alıntıdır.
typedef struct msgbuf
{
	long mtype;
	char mtext[MSGSZ];
} message_buf;

main()
{

	int msqid;
	int msgflg = IPC_CREAT | 0666;
	key_t key;
	message_buf sbuf;
	message_buf rbuf;
	size_t buf_length;

	key = 1234;

	if ((msqid = msgget(key, msgflg)) < 0)
	{
		perror("msgget");
		exit(1);
	}

	do
	{

		if (msgrcv(msqid, &rbuf, MSGSZ, 1, 0) < 0)
		{
			perror("msgrcv");
			exit(1);
		}

		char *myParsedQuery[100]; //Querymizin boşluğa göre parçalandıktan sonra atıldığı array.
		parseQueryLine(rbuf.mtext, myParsedQuery);

		char data[150]; // Sorgulanan verilerin dosyadan sorgulanıp sonuç değerlerinin tutulacığı dizi.
		strcpy(data, "");

		//Parametre girişlerinin doğruluğunun kontrolü.
		if (strcmp(myParsedQuery[0], "select") == 0 && strcmp(myParsedQuery[2], "from") == 0)
		{
			char lastPart[2][10];
			/* Sorgunun boşluğa göre parse edilmiş halinin tutulduğu
			dizinin son elemanının "=" karakterine göre parçalanması ve bir arraya atılması işlemi */
			char *tempOfPointer1 = strtok(myParsedQuery[5], "=");
			strcpy(lastPart[0], tempOfPointer1);
			tempOfPointer1 = strtok(NULL, "=");
			strcpy(lastPart[1], tempOfPointer1);

			//lastPart[0]' ın kopyalanması işlemi
			char firstElementOfLastPart[20];
			strcpy(firstElementOfLastPart, lastPart[0]);

			//Dosyadan verilerin okunup sorgu sonuçlarının aranması işlemleri.
			FILE *filePointer = fopen(myParsedQuery[3], "r");

			char fileText[128];		  //Dosyadan okunan satırın tutulduğu array.
			char copiedFileText[128]; //fileText arrayinin yedekği olan array.

			if (filePointer == NULL)
			{
				perror("Unable to open file!");
				exit(1);
			}

			while (fgets(copiedFileText, sizeof(copiedFileText), filePointer) != NULL)
			{

				char personData[2][10];
				strcpy(fileText, copiedFileText);
				//Dosya satırının yedeği alınıp ikiye bölünüyor ve personDAta 2 boyutlu arrayinde isim ve numara bilgileri saklanıyor.
				char *tempOfPointer2 = strtok(copiedFileText, " ");
				strcpy(personData[0], tempOfPointer2);
				tempOfPointer2 = strtok(NULL, "");
				strcpy(personData[1], tempOfPointer2);

				//sorgumuzun numara ya da ad olarak işlenmesi.
				//Sorgumuzun numara olarak işlenmesi: Örneğin (select * from veri2.txt where number=20)
				if (strcmp(firstElementOfLastPart, "number") == 0)
				{
					int realLengthOfPersonNumberData = strlen(personData[1]) - 2;
					//Dosyadan okunan numaradaki son 2 çöp verisinin eksiltilerek, numara verisinin uzunluğunun doğru şekilde belirlenmesi.

					char realNumberData[realLengthOfPersonNumberData];
					int counter = 0;
					while (counter < realLengthOfPersonNumberData)
					{
						realNumberData[counter] = personData[1][counter];
						counter++;
					}
					realNumberData[realLengthOfPersonNumberData] = NULL;
//Aşağıdaki 3 if kontrolü number sorgusu ve istenen verilerin *, ad ve number için 3 farklı şekilde datanın içine strcat vasıtasıyla eklenmesi.

					if (strcmp(realNumberData, lastPart[1]) == 0 && strcmp(myParsedQuery[1], "*") == 0)
						strcat(data, fileText);  // * ise tüm satır ekleniyor.
                     
					if (strcmp(realNumberData, lastPart[1]) == 0 && strcmp(myParsedQuery[1], "ad") == 0)
					{
						strcat(personData[0], "\n");  // ad ise sadece isim verisi ekleniyor.
						strcat(data, personData[0]);
					}
					if (strcmp(realNumberData, lastPart[1]) == 0 && strcmp(myParsedQuery[1], "number") == 0)
						strcat(data, personData[1]);  // number ise sadece numara verisi ekleniyor.
				}

				//Sorgumuzun ad olarak işlenmesi: Örneğin (select * from veri2.txt where ad=talat)
				else if (strcmp(firstElementOfLastPart, "ad") == 0)
				{
					if (strcmp(lastPart[1], personData[0]) == 0 && strcmp(myParsedQuery[1], "*") == 0)
						strcat(data, fileText);  // * ise tüm satır ekleniyor.

					if (strcmp(lastPart[1], personData[0]) == 0 && strcmp(myParsedQuery[1], "ad") == 0)
					{
						strcat(personData[0], "\n");  // ad ise sadece isim verisi ekleniyor.
						strcat(data, personData[0]); 
					}
					if (strcmp(lastPart[1], personData[0]) == 0 && strcmp(myParsedQuery[1], "number") == 0)
						strcat(data, personData[1]);  // number ise sadece numara verisi ekleniyor.
				}
			}

			fclose(filePointer);
		}
		else //Parametre girdilerinin yazmında bir hata varsa pipe üzerine yazılacak dataya error mesajı yazılır..
		{
			strcpy(data, "error");
		}
//Yukarıdaki koşullar gerçekleşmemişse sorgu başarılı olmuş ama veri1.txt ve veri2.txt dosyalarında eşkeleşen veriler bulunamamıştır demektir.
//Bu sebeple pipe a gidecek dataya nothing mesajı yazılır.
		if (strcmp(data, "") == 0) 
			strcpy(data, "Nothing");
		strcpy(sbuf.mtext, data); // İçi doldurulmuş datanın pipe üzerine yazılması işlemi...

		sbuf.mtype = 1;
		buf_length = strlen(sbuf.mtext) + 1;

		if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0)
		{
			perror("msgsnd");
			exit(1);
		}

	} while (1);

	exit(0);
}