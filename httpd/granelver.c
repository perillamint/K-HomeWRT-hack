//Ultra simple http server based on
//Echoserv by Paul Griffiths, 1999

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/*  Global constants  */

#define HTTP_PORT          (31337)
#define MAX_LINE           (1000)
#define LISTENQ            (1024)
#define MARGIN             (512)

const char http200[] = "HTTP/1.0 200 OK\r\n"
  "Content-Type: text/html\r\n"
  "Content-Length: ";

const char endline[] = {0x0D, 0x0A, 0x0D, 0x0A};
const int endline_len = 4;

//Embedded HTML in ELF
extern int _binary_pwned_html_start;
extern int _binary_pwned_html_size;

void reaper(int sig);

int main(int argc, char *argv[])
{
  int       list_s;                /*  listening socket          */
  int       conn_s;                /*  connection socket         */
  struct    sockaddr_in servaddr;  /*  socket address structure  */

  char http200len = strlen(http200);
  char httphdr[http200len + MARGIN];
  int httphdr_size;

  //Daemonize.

  if(fork() != 0)
    {
      exit(0);
    }
  setsid();
  signal(SIGHUP, SIG_IGN);

  snprintf(httphdr, http200len + MARGIN, "%s%d\r\n\r\n",
           http200, (int)(intptr_t)&_binary_pwned_html_size);
  httphdr_size = strlen(httphdr);
  
  /*  Create the listening socket  */

  if ( (list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
      fprintf(stderr, "Granelver: Error creating listening socket.\n");
      exit(EXIT_FAILURE);
    }


  /*  Set all bytes in socket address structure to
      zero, and fill in the relevant data members   */

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(HTTP_PORT);


  /*  Bind our socket addresss to the
      listening socket, and call listen()  */

  if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
    {
      fprintf(stderr, "Granelver: Error calling bind()\n");
      exit(EXIT_FAILURE);
    }

  if ( listen(list_s, LISTENQ) < 0 )
    {
      fprintf(stderr, "Granelver: Error calling listen()\n");
      exit(EXIT_FAILURE);
    }

  //Corpse reaper
  signal(SIGCHLD, reaper);

  /*  Enter an infinite loop to respond
      to client requests and echo input  */

  for(;;)
    {

      /*  Wait for a connection, then accept() it  */

      if ( (conn_s = accept(list_s, NULL, NULL) ) < 0 )
        {
          fprintf(stderr, "Granelver: Error calling accept()\n");
          exit(EXIT_FAILURE);
        }

      if(0 != fork())
        {
          close(conn_s);
          continue;
        }

      /*  Retrieve an input line from the connected socket
          then simply write it back to the same socket.     */

      char c;
      int cnt = 0;

      //Ignore HTTP verbs.
      //Just serve single page.
      for(;;)
        {
          read(conn_s, &c, 1);

          if(c == endline[cnt])
            cnt++;

          if(cnt == endline_len)
            break;
        }

      write(conn_s, httphdr, httphdr_size);
      write(conn_s, (char*)&_binary_pwned_html_start,
            (int)(intptr_t)&_binary_pwned_html_size);

      /*  Close the connected socket  */

      if ( close(conn_s) < 0 )
        {
          fprintf(stderr, "Granelver: Error calling close()\n");
          exit(EXIT_FAILURE);
        }
      else
        {
          exit(0);
        }
    }
}

void reaper(int sig)
{
  while (waitpid(-1, 0, WNOHANG) >= 0);
}
