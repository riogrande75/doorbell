#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <linux/inotify.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

//defs for filereader
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
//defs for sendudp
#define SERVER "192.168.123.148"
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to send data

void die(char *s)
{
    perror(s);
    exit(1);
}

int sendudp(void)
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];

    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    strcpy( message, "A VISITOR IS WAITING!" );
    //send the message
    if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
	{
	die("sendto()");
	}
    //receive a reply and print it
    //clear the buffer by filling null, it might have previously received data
    memset(buf,'\0', BUFLEN);
    close(s);
    return 1;
}

int main( )
{
  int length, i = 0;
  int fd;
  int wd;
  char buffer[EVENT_BUF_LEN];
  char filedir[40];
  time_t rawtime;
  struct tm * timeinfo;
  char timebuffer [80];
  time (&rawtime);
  timeinfo = localtime (&rawtime);
//  strftime (timebuffer,80,"Date is %F.",timeinfo);
  puts (timebuffer);
  strftime(filedir, sizeof(filedir), "/mnt/disc1/npc/alarm/%F", timeinfo);
  printf ("Monitored directory is: %s \n",filedir);
  /*creating the INOTIFY instance*/
  fd = inotify_init();
  /*checking for error*/
  if ( fd < 0 ) {
    perror( "inotify_init" );
  }
  /* adding the directory into watch list */
//  wd = inotify_add_watch( fd, filedir, IN_CREATE | IN_DELETE );
  wd = inotify_add_watch( fd, "/mnt/disc1/npc/alarm" , IN_CREATE | IN_DELETE );

while(1) { //do forever

  length = read( fd, buffer, EVENT_BUF_LEN );
  /*checking for error*/
  if ( length < 0 ) {
    perror( "read" );
  }
  /*actually read return the list of change events happens*/
  while ( i < length )  {
	struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
	if ( event->len ) {
      		if ( event->mask & IN_CREATE ) {
        		if ( event->mask & IN_ISDIR ) {
				printf( "** Doorbutton press was detected **\n");
                                printf( "New directory %s was created.\n", event->name );
                                sendudp();
                                printf( "UDP Packet successfuly sent to server\n");
				}
        		else {
                                printf( "** Doorbutton press was detected **\n");
  	        		printf( "New file %s was created.\n", event->name );
				sendudp();
				printf( "UDP Packet successfuly sent to server\n");
        			}
		      	}
    		}
    i += EVENT_SIZE + event->len;
  }
}
   printf( "***** Ending doorbell *****\n");
  /*removing the “/tmp” directory from the watch list.*/
   inotify_rm_watch( fd, wd );
  /*closing the INOTIFY instance*/
   close( fd );
}
