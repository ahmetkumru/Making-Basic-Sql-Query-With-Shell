#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>

#define MSGSZ 128

// Data transferi için kullanılacak struct yapısı. Laboratuvar örneklerimizden alıntıdır.
typedef struct msgbuf
{
	long mtype;
	char mtext[MSGSZ];
} message_buf;

int main(int argc, char *argv[])
{

	int msqid;
	key_t key;
	int msgflg = IPC_CREAT | 0666;
	message_buf rbuf;
	message_buf sbuf;
	size_t buf_length;

	key = 1234;

	if ((msqid = msgget(key, msgflg)) < 0)
	{
		perror("msgget");
		exit(1);
	}

	int exitControl = 1; // Çıkışımızı kontrol etmemizi sağlayan değişken.
	char *queryLine[100]; //Kullanıcıdan aldığımız sorguyu tutan array.
	while (exitControl) // sonsuz döngü oluşturulması.
	{

		printf("Please enter the query : \n");

		gets(queryLine);
		if (strcmp(queryLine, "exit") == 0) //çıkış işleminin yapılması.
		{
			exitControl = 0;
			printf("Program shutting down. See you soon :)\n");
			exit(0);
		}
        
		//sorgumuzu sbuf.mtext e databasede kullanılmaz üzere gönderiyoruz...
		strcpy(sbuf.mtext, queryLine); 
		sbuf.mtype = 1;
		buf_length = strlen(sbuf.mtext) + 1;
		if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0)
		{

			perror("msgsnd");
			exit(1);
		}

		if (msgrcv(msqid, &rbuf, MSGSZ, 1, 0) < 0)
		{
			perror("msgrcv");
			exit(1);
		}

		if (strcmp(rbuf.mtext, "Nothing") == 0) //Sorgu sonucunda veritabanında eşleşen bir kayıt bulunmadıysa ekrana uyarı mesajı verir.
		{
			printf("No matching records found...\n");
		}
		else if(strcmp(rbuf.mtext, "error") == 0){ //Databasede kontrolünü yaptığımız sorguda parametre hatası varsa bu hatayı döndürür.
		  printf("Wrong Parametre\n");

		}
		else
		{   //Sonuç verilerinin kayıt edilip edilmeyeceğinin kullanıcıya sorulması işlemi. 
			printf("Would you like to save the results ? e/h :\n");
			char yesOrNo[1];

			gets(yesOrNo);
			if (strcmp(yesOrNo, "h") == 0) // Hayır cevabı sonucu
			{
				printf("You cancelled data save...\n");
			}
			else if (strcmp(yesOrNo, "e") == 0) // Evet cevabı sonucu
			{
				int pipefd[2];
				int temp;
				if (pipe(pipefd) < 0)
				{
					perror("Pipe couldn't create...");
					exit(1);
				}
				int pid = fork();
				if (pid == 0)
				{

					printf("Data to be written to sonuc.txt : \n%s ", rbuf.mtext);
					write(4, rbuf.mtext, MSGSZ);  //Verilerimizi unnamed pipe üzerine yazıyoruz...

					temp = execv("kaydet", NULL);

					perror("");
					close(pipefd[1]);
				}
				else
				{
					int cpid = wait(&temp);
				}
				printf("Saving completed.\n");
			}
			else // e yada h dışında bir parametre girilirse bu hatayı bastırır.
			{
				printf("You entered invalid parameters...\n");
			}
		}
	}

	exit(0);
}