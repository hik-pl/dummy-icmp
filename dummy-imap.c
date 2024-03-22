#include <stdio.h>
#include <string.h>  //strlen
#include <sys/socket.h>
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>  //write
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define PORT 11143
#define MAX_BUF 1020
#define MAX_UID 254
#define MAX_CMD 254
#define MAX_USER 254

const char WELCOME[] = "* OK [CAPABILITY IMAP4rev1 SASL-IR LOGIN-REFERRALS ID ENABLE IDLE LITERAL+ STARTTLS LOGINDISABLED] Dovecot (Hikor) ready.\r\n";
const char C_CAPABILITY1[] ="* CAPABILITY IMAP4rev1 SASL-IR LOGIN-REFERRALS ID ENABLE IDLE LITERAL+ STARTTLS LOGINDISABLED\r\n";
const char C_CAPABILITY2[] =" OK Pre-login capabilities listed, post-login capabilities have more.\r\n";
const char C_ID1[] ="* ID (\"name\" \"Dovecot\")\r\n";
const char C_ID2[] =" OK ID completed.\r\n";
const char C_NOOP2[] =" OK NOOP completed.\r\n";
const char MAINTENANCE[] = " NO [UNAVAILABLE] User's backend down for maintenance\r\n";
const char GOODBYE[] = " BYE.\r\n";


pid_t pid;
//int finished=0;


void serveImap(int fd) {
	
	send(fd,WELCOME, sizeof(WELCOME),0);

	int i, n;
	char buf[MAX_BUF+1];
	char c_uid[MAX_UID];
	char c_cmd[MAX_CMD];

	while (1) {

		bzero(buf, MAX_BUF);
		int b_recv = recv(fd, buf, MAX_BUF, 0);

		if (b_recv <= 0) {
			//client closed
			close(fd);
			shutdown(fd, SHUT_RDWR); 
			break;  
		}


		if(strlen(buf) > 0 && strlen(buf) < (MAX_BUF-2) && !strcmp(buf + strlen(buf) - 1, "\n")) {
			buf[strlen(buf)+1]='\n';
		}

		
		i=0;
		n=0;		
		bzero(c_uid, MAX_UID);
		while (i < MAX_BUF && n < MAX_UID-1 && buf[i] != ' ' && buf[i] != '\0' && buf[i] != '\n' && buf[i] != '\r') { 
			c_uid[n++]=buf[i++];
		}
		if (strlen(c_uid) == 0) {
			c_uid[0]='*';
		}

		if (i+1<MAX_BUF) {
			i++;
			n=0;
			bzero(c_cmd, MAX_CMD);
			while (i < MAX_BUF && n < MAX_CMD-1 && buf[i] != ' ' && buf[i] != '\0' && buf[i] != '\n' && buf[i] != '\r') { 
				c_cmd[n++]=toupper(buf[i++]);
			}
		}

		
		if (strlen(c_cmd)>0) {
			if (strncmp("LOGOUT", c_cmd, 6) == 0) {
				printf("%d: Size: %d Msg: %s",getpid(),b_recv,buf);  
				bzero(buf, MAX_BUF);
				i=0;
				n=0;
				while (i< MAX_BUF && n<strlen(c_uid)) { buf[i++] = c_uid[n++]; }
				n=0;
				while (i < MAX_BUF && n<strlen(GOODBYE)) { buf[i++] = GOODBYE[n++]; }
				send(fd,buf,i,0);		
				close(fd);
				shutdown(fd, SHUT_RDWR); 
				break;
			} else if (strncmp("CAPABILITY", c_cmd, 10) == 0) {
				printf("%d: Size: %d Msg: %s",getpid(),b_recv,buf);  
				bzero(buf, MAX_BUF);
				i=0;
				n=0;
				while (i< MAX_BUF && n<strlen(C_CAPABILITY1)) { buf[i++] = C_CAPABILITY1[n++]; }
				n=0;
				while (i< MAX_BUF && n<strlen(c_uid)) { buf[i++] = c_uid[n++]; }
				n=0;
				while (i< MAX_BUF && n<strlen(C_CAPABILITY2)) { buf[i++] = C_CAPABILITY2[n++]; }
				send(fd,buf,i,0);
			} else if (strncmp("ID", c_cmd, 2) == 0) {
				printf("%d: Size: %d Msg: %s",getpid(),b_recv,buf);  
				bzero(buf, MAX_BUF);
				i=0;
				n=0;
				while (i< MAX_BUF && n<strlen(C_ID1)) { buf[i++] = C_ID1[n++]; }
				n=0;
				while (i< MAX_BUF && n<strlen(c_uid)) { buf[i++] = c_uid[n++]; }
				n=0;
				while (i< MAX_BUF && n<strlen(C_ID2)) { buf[i++] = C_ID2[n++]; }
				send(fd,buf,i,0);
			} else if (strncmp("NOOP", c_cmd, 4) == 0) {
				printf("%d: Size: %d Msg: %s",getpid(),b_recv,buf);  
				bzero(buf, MAX_BUF);
				i=0;
				n=0;
				while (i< MAX_BUF && n<strlen(c_uid)) { buf[i++] = c_uid[n++]; }
				n=0;
				while (i< MAX_BUF && n<strlen(C_NOOP2)) { buf[i++] = C_NOOP2[n++]; }
				send(fd,buf,i,0);
			} else if (strncmp("LOGIN", c_cmd, 5) == 0) {

				char c_user[MAX_USER];
				bzero(c_user, MAX_USER);
				//warning variable i (value from previous search)
				if (i+1<MAX_BUF) {
					i++;
					n=0;
					while (i < MAX_BUF && n < MAX_USER-1 && buf[i] != ' ' && buf[i] != '\0' && buf[i] != '\n' && buf[i] != '\r') { 
						c_user[n++]=buf[i++];
					}
				}
				printf("%d: Size: %d Msg: %s %s %s\n",getpid(),b_recv,c_uid,c_cmd,c_user);  
				
				bzero(buf, MAX_BUF);
				i=0;
				n=0;
				while (i< MAX_BUF && n<strlen(c_uid)) { buf[i++] = c_uid[n++]; }
				n=0;
				while (i< MAX_BUF && n<strlen(MAINTENANCE)) { buf[i++] = MAINTENANCE[n++]; }
				send(fd,buf,i,0);
				
			} else {
				printf("%d: Size: %d Msg: %s",getpid(),b_recv,buf);  
				bzero(buf, MAX_BUF);
				i=0;
				n=0;
				while (i< MAX_BUF && n<strlen(c_uid)) { buf[i++] = c_uid[n++]; }
				n=0;
				while (i< MAX_BUF && n<strlen(MAINTENANCE)) { buf[i++] = MAINTENANCE[n++]; }
				send(fd,buf,i,0);					
			}
		} else {
				printf("%d: Size: %d Msg: %s",getpid(),b_recv,buf);  
				bzero(buf, MAX_BUF);
				i=0;
				n=0;
				while (i< MAX_BUF && n<strlen(c_uid)) { buf[i++] = c_uid[n++]; }
				n=0;
				while (i< MAX_BUF && n<strlen(MAINTENANCE)) { buf[i++] = MAINTENANCE[n++]; }
				send(fd,buf,i,0);					
			}
		
		
		
		
		
		
	}
}


void zombie_hunter(int sig)
    {
    int status;
    waitpid(pid, &status, 0);
//    printf("Got status %d from child\n",status);
//    finished=1;
    }


int main(int argc , char *argv[]) {

  signal(SIGCHLD,zombie_hunter);
  int port=PORT;
  char ip[16]="127.0.0.1";

 if (argc==2) {
  if ((strcmp(argv[1], "--help") == 0)||(strcmp(argv[1], "-help") == 0)||(strcmp(argv[1], "help") == 0)||(strcmp(argv[1], "-h") == 0)||(strcmp(argv[1], "-?") == 0)) {
     printf("Usage: %s [ip{default:%s}] [port{default:%d}]\n",argv[0],ip,port);
    return 0;
  }
 }

  if (argc>1) {
	  if (strlen(argv[1]) <16 ) strcpy(ip, argv[1]);
  }
  if (argc>2) {
	  port=atoi(argv[2]);
  }


  // Create the socket

  int server_socket = socket(AF_INET , SOCK_STREAM , 0);
  if (server_socket == -1) {
    printf("Could not create socket.\n");
    return 1;
  }

  // Set the 'reuse address' socket option
  int on = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  //server.sin_addr.s_addr = INADDR_ANY;
  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_port = htons( port );

  // Bind to the port we want to use
  if(bind(server_socket,(struct sockaddr *)&server , sizeof(server)) < 0) {
    printf("Bind failed\n");
    printf("Already in use ip %s on port %d...\n",ip,port);
    return 1;
  }
  // Mark the socket as a passive socket
  listen(server_socket , 3);

  // Accept incoming connections
  printf("Server dummy IMAP(Hikor) PID:%d ready. \n",getpid());
  printf("Waiting for incoming connections ip %s on port %d...\n",ip,port);

  while(1) {

    struct sockaddr_in client;
    int new_socket , c = sizeof(struct sockaddr_in);
    new_socket = accept(server_socket, (struct sockaddr *) &client, (socklen_t*)&c);
	int status;
    if(new_socket != -1) {
	  int r=random();
	  pid = fork();
      if (pid == -1) {
        exit(1);
      } else if (pid == 0) {
		printf("%d: Client_connected: %s:%d\n", getpid(), inet_ntoa(client.sin_addr),(int) ntohs(client.sin_port));
        serveImap(new_socket);
		close(new_socket);
		printf("%d: Dissconected\n", getpid());
        exit(0);
      } else {
       // printf("Spawned process %d\n",pid);
        close(new_socket);
		while(waitpid(-1,&status,WNOHANG) > 0);
      }
    }
  }
  return 0;
}
