/* cliTCPIt.c - Exemplu de client TCP
   Trimite un nume la server; primeste de la server "Hello nume".
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
   Link: https://profs.info.uaic.ro/~computernetworks/files/NetEx/S5/cliTcpIt.c
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[1000];		// mesajul trimis catre server
  char username[100];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET; 
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[Client]Eroare la connect().\n");
      return errno;
    }

  
  bzero (msg, 100);
  system("clear"); 
  printf("----------------------------------------| QuizzGame |---------------------------------------- \n|                Vei primi 5 intrebari la care trebuie sa raspunzi obligatoriu.             |\n|                  Pentru fiecare intrebare ai la dispozitie 20 de secunde.                 |\n|                          Daca timpul expira, vei fi descalificat.                         |\n|      Pentru a raspunde, introduce cifra corespunzatoare raspunsului si apasa \"Enter\".     |\n--------------------------------------------------------------------------------------------- \n[Client] Introduce un username: ");
  fflush (stdout);
  /* citim username */
  if (read(0, msg, sizeof(msg)) < 0)
  {
    perror("[Client] Eroare la citirea username-ului.\n");
    return errno;
  }
  
  /* trimiterea username-ului la server */
  if (write (sd, msg, 100) <= 0)
    {
      perror ("[Client] Eroare la trimiterea username-ului spre server.\n");
      return errno;
    }

    /* prima intrebare, am pus-o separat pentru a nu face o verificare in plus la fiecare
    pentru a vedea daca s-a conectat toata lumea sau nu [if(msg[0] == '2')]*/
    system("clear");
    int temp = read (sd, msg, sizeof(msg));
    if (temp < 0)
    {
      perror ("[Client] Eroare la read() de la server.\n");
      return errno;
    }else if(temp == 0) {
              printf("Timpul a expirat, imi pare rau. \n");
              exit(0);
              } //a expirat timpul si inchidem clientul
    if(msg[0] == '2')
    {
      printf("%s\n", msg+1);
      if (read(sd, msg, sizeof(msg)) < 0)
      {
        perror("[Client] Eroare la citirea castigatorului.\n");
        return errno;
      }
      
    }
    system("clear");
    /* afisam mesajul primit */
    printf ("[Client] Intrebarea #1:\n---> %s\n", msg);
    //citim raspunsul de la user
    if (read(0, msg, sizeof(msg)) < 0)
    {
      perror("[Client] Eroare la citirea raspunsului.\n");
      return errno;
    }
      
      /* trimiterea raspunsului la server */
      if (write (sd, msg, 1) <= 0)
      {
        perror ("[Client] Eroare la trimiterea raspunsului spre server.\n");
        return errno;
      }


  /* urmatoarele intrebari */ 
    for(int i = 2; i <= 5; i++)
    {
      system("clear");
      int temp = read (sd, msg, sizeof(msg));
      if (temp < 0)
      {
        perror ("[Client] Eroare la read() de la server.\n");
        return errno;
      }else if(temp == 0) {
                printf("Timpul a expirat, imi pare rau. \n");
                exit(0);
                } //a expirat timpul si inchidem clientul

      /* afisam mesajul primit */
      printf ("[Client] Intrebarea #%d:\n---> %s\n", i, msg);
      //citim raspunsul de la user
      if (read(0, msg, sizeof(msg)) < 0)
      {
        perror("[Client] Eroare la citirea raspunsului.\n");
        return errno;
      }
      
      /* trimiterea raspunsului la server */
      if (write (sd, msg, 1) <= 0)
      {
        perror ("[Client] Eroare la trimiterea raspunsului spre server.\n");
        return errno;
      }
  }

  if (read(sd, msg, sizeof(msg)) < 0)
  {
    perror("[Client] Eroare la citirea castigatorului.\n");
    return errno;
  }
  system("clear");
  if(msg[0] == '2')
    {
      printf("%s\n", msg+1);
      if (read(sd, msg, sizeof(msg)) < 0)
      {
        perror("[Client] Eroare la citirea castigatorului.\n");
        return errno;
      }
      system("clear");
      printf("%s\n", msg);
    }
    else printf("%s\n", msg);
  //printf("%s\n", msg);
 



  /* inchidem conexiunea, am terminat */
  close (sd);
}