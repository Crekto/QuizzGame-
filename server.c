/* 	servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   	prin crearea unui thread pentru fiecare client.
   	Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   gcc server.c -o server -lsqlite3 -pthread
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
   Link: https://profs.info.uaic.ro/~computernetworks/files/NetEx/S12/ServerConcThread/servTcpConcTh2.c
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>

/* portul folosit */
#define PORT 2908

int threadIDs = 0, players = 0, started[999] = {0}, still_playing[999] = {0}, most_points = 0, winners = 0, alreadyStarted = 0;
char winners_list[100][100];

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
  //char username[100];
  //int points;
}thData;

struct{
  char username[100];
  int points;
}player[999]; //am incercat sa fac in acelasi struct thData dar am intampinat probleme

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void create_database(); // cream data de baze si inseram intrebarile in ea
int get_question(int k, char question[], char correct_answer[]); // luam intrebarile pentru fiecare client
int game_over();
void get_winners();
void reset_winners();
void reset_players();
void eliminate_player(int i);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	//int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[Server] Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pana la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=threadIDs++;
	td->cl=client;
  still_playing[players] = 1;
  create_database();
	pthread_create(&th[players], NULL, &treat, td);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[Thread #%d] Asteptam clientul...\n", tdL.idThread);
		fflush (stdout);
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
    
    sleep(1);
    //reset_players();
		/* am terminat cu acest client, inchidem conexiunea */
    //shutdown(tdL.cl, SHUT_RDWR);
    close(tdL.cl);
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
    int i = 0;
    char user[100];
	  struct thData tdL; 
	  tdL= *((struct thData*)arg);
    /////////timer intrebare
    fd_set rfds;
    struct timeval tv;
    int time_expired;
    //////////////
  while(alreadyStarted == 1)
    usleep(1);

    if(read (tdL.cl, user, 100) <= 0)
    {
      printf("[Thread #%d]\n",tdL.idThread);
      perror ("Eroare la read() de la client.\n");
      still_playing[tdL.idThread] = 0;
      players++;
      return;
    }else{
          user[strlen(user)-1] = '\0';
          strcpy(player[tdL.idThread].username,user);
          player[tdL.idThread].points = 0;
          players++;
          still_playing[tdL.idThread] = 1;
    }
  printf ("[Thread #%d] %s s-a conectat.\n", tdL.idThread, player[tdL.idThread].username);

  if(threadIDs!=players)
    {
      char aux[1000];
      strcpy(aux,"2Te-ai conectat cu succes.\nAsteptam sa se conecteze toata lumea.");
      
      if (write (tdL.cl, aux, sizeof(aux)) <= 0) 
      {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread] Eroare la write() catre client.\n");
      }
    }

  while(threadIDs!=players)
    usleep(1);
  alreadyStarted = 1;
  reset_players();

	for(int i = 0; i < 5; i++)
    {
        char question[1000], correct_answer;
        get_question(i, question, &correct_answer); //luam a i-a intrebare si raspunsul corect corespunzator

        if (write (tdL.cl, question, sizeof(question)) <= 0) //trimitem intrebarea clientului
            {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread] Eroare la write() catre client.\n");
            }
        else
            printf ("[Thread %d] Intrebarea a fost transmisa cu succes.\n",tdL.idThread);
    FD_ZERO(&rfds);
    FD_SET(tdL.cl, &rfds);

    tv.tv_sec = 20; //asteptam 20sec un raspuns
    tv.tv_usec = 0;

    time_expired = select(tdL.cl+1, &rfds, NULL, NULL, &tv);

    if(time_expired == -1)
    {
      perror("[Thread] Eroare la select().\n");
    }
    else if(time_expired)
    {
      char answer;
      int temp = read (tdL.cl, &answer, 1);
      if (temp < 0) //citim raspunul la intrebare
      {
        perror ("[Thread] Eroare la read() de la client.\n");
        return;
      }else if(temp == 0)
        {
          player[tdL.idThread].points = 0;
          still_playing[tdL.idThread] = 0;
          printf("[Thread %d] Jucatorul %s s-a deconectat.\n", tdL.idThread, player[tdL.idThread].username);
          return;
        }
      printf("[%s] Raspuns #%d: %s\n", player[tdL.idThread].username, i, &answer);
      if(answer == correct_answer)
      {
        player[tdL.idThread].points++;
      }
    }else{
      printf("[Thread %d] Nu am primit niciun raspuns in 20 de secunde de la user-ul %s.\n", tdL.idThread, player[tdL.idThread].username);
      player[tdL.idThread].points = 0;
      still_playing[tdL.idThread] = 0;
      return;
    }  
    }

    printf("[Thread %d] User-ul %s a terminat quiz-ul si are punctajul %d.\n", tdL.idThread, player[tdL.idThread].username, player[tdL.idThread].points);
    still_playing[tdL.idThread] = 0;
    fflush(stdout);

    if(game_over()!=1)
    {
      char aux[1000];
      sprintf(aux,"2Ai terminat quiz-ul si ai acumulat in total %d puncte.\nAsteptam sa termine toata lumea quiz-ul.", player[tdL.idThread].points);
      
      if (write (tdL.cl, aux, sizeof(aux)) <= 0) 
      {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread] Eroare la write() catre client.\n");
      }
    }

    while(game_over()!=1)
      usleep(1);

    get_winners();

    char aux[1000];
    memset(aux, 0, sizeof(aux));
    if(most_points == 0)
      sprintf(aux, "Imi pare rau dar nimeni nu a castigat.\n");
    else
    {
      if(winners == 1)
    {
      if(player[tdL.idThread].points == most_points)
      {
        sprintf(aux, "Felicitari, %s! Esti singurul castigator, avand punctajul %d.\n", player[tdL.idThread].username, most_points);
      }
      else sprintf(aux, "Imi pare rau, %s, ai acumulat doar %d puncte.\nCastigatorul este %s cu un punctaj de %d.\n", player[tdL.idThread].username, player[tdL.idThread].points, winners_list[1], most_points);
    }
    else
    {
      if(player[tdL.idThread].points == most_points)
        sprintf(aux, "Felicitari, %s! Ai castigat.\n", player[tdL.idThread].username);
      else sprintf(aux, "Imi pare rau, %s, ai acumulat doar %d puncte.\n", player[tdL.idThread].username, player[tdL.idThread].points);
      strcat(aux, "Castigatorii sunt: ");
      for(int i = 1; i < winners; i++)
      {
        strcat(aux, winners_list[i]);
        strcat(aux, ", ");
      }
      strcat(aux, winners_list[winners]);
      strcat(aux, " cu punctajul");
      char aux_punctaj[10];
      sprintf(aux_punctaj, " %d.", most_points);
      strcat(aux, aux_punctaj);
    }
    }

    if (write (tdL.cl, aux, sizeof(aux)) <= 0)
      perror ("[Thread] Eroare la write() catre client.\n");
    alreadyStarted = 0;
}

void create_database()
{
  sqlite3* database;
  char* sql = "DROP TABLE IF EXISTS intrebari;"
              "CREATE TABLE intrebari("
              "ID INT PRIMARY KEY NOT NULL, "
              "INTREBARE TEXT NOT NULL, "
              "R1 TEXT NOT NULL, "
              "R2 TEXT NOT NULL, "
              "R3 TEXT NOT NULL, "
              "R4 TEXT NOT NULL, "
              "R_CORRECT CHAR(1) NOT NULL);"
              "INSERT INTO intrebari VALUES(0, '1+1= ?', '1', '2', '3', '4', '2');"
              "INSERT INTO intrebari VALUES(1, 'What is a correct syntax to output \"Hello World\" in C++?', 'print (\"Hello World\");', 'cout << \"Hello World\";', 'Console.WriteLine(\"Hello World\");', 'System.out.println(\"Hello World\");', '2');"
              "INSERT INTO intrebari VALUES(2, 'How do you insert COMMENTS in C++ code?', '/* This is a comment', '# This is a comment', '<!-- This is a comment', '// This is a comment', '4');"
              "INSERT INTO intrebari VALUES(3, 'How do you create a variable with the floating number 5.2?', 'int x = 5.2;', 'x = 5.2;', 'double x = 5.2;', 'byte x = 5.2', '3');"
              "INSERT INTO intrebari VALUES(4, 'Which method can be used to find the length of a string?', 'length()', 'getSize()', 'len()', 'getLength()', '1');";

  int db = sqlite3_open("intrebari.db", &database); 
  if(db)
    perror("[create_database()] Eroare la deschiderea bazei de date.");
  else printf("[create_database()] Baza de date a fost deschisa cu succes.\n");
  char* error;
  db = sqlite3_exec(database, sql, NULL, 0, &error);
  if(db != SQLITE_OK)
  {
    perror("[create_database()] Eroare la baza de date.\n");
    sqlite3_free(error);
  }
  else printf("[create_database()] Baza de date a fost creata cu succes.\n");
  sqlite3_close(database);
}

int get_question(int k, char question[], char correct_answer[])
{
  sqlite3* database;
  sqlite3_stmt* statement;

  int db = sqlite3_open("intrebari.db", &database); 
  if(db)
    perror("[get_question()] Eroare la deschiderea bazei de date.\n");
  else printf("[get_question()] Baza de date a fost deschisa cu succes.\n");
  
  char *sql = "select * from intrebari where id = ?";
  db = sqlite3_prepare_v2(database, sql, -1, &statement, NULL);
  if(db != SQLITE_OK)
  {
    perror("[get_question()] Eroare la sqlite3_prepare_v2.\n");
    return 0;
  }
  
  sqlite3_bind_int(statement, 1, k);
  int step = sqlite3_step(statement);
  if(step == SQLITE_ROW)
  {
    sprintf(question, "%s\n[1] %s\n[2] %s\n[3] %s\n[4] %s\n--------------------\nRaspunsul tau: ", sqlite3_column_text(statement, 1),sqlite3_column_text(statement, 2),sqlite3_column_text(statement, 3),sqlite3_column_text(statement, 4),sqlite3_column_text(statement, 5));
    sprintf(correct_answer, "%s", sqlite3_column_text(statement, 6));
  }
  sqlite3_finalize(statement);
  sqlite3_close(database);

  return 1;
}


int game_over()
{
  for(int i = 0; i < players; i++)
    if(still_playing[i] == 1) return 0;
  return 1;
}

void reset_winners()
{
  winners = 0;
  most_points = 0;
  memset(winners_list, 0, sizeof(winners_list));
}

void reset_players()
{
  for(int i = 0; i < players; i++)
  {
    player[i].points = 0;
  }
}

void get_winners()
{
  reset_winners();
  for(int i = 0; i < players; i++)
  {
    if(player[i].points > most_points)
    {
      most_points = player[i].points;
      winners = 1;
      memset(winners_list, 0, sizeof(winners_list));
      strcpy(winners_list[1],player[i].username); 
    }
    else if(player[i].points == most_points)
    {
      winners++;
      strcpy(winners_list[winners], player[i].username);
    }
  }
    
}
