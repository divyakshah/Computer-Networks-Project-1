/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2021 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 45321  
typedef struct request_packet{
	unsigned short request_ID;
	unsigned short count;
}r_q;

typedef struct response_packet{
	unsigned short request_ID;
	unsigned short seq_number;
	unsigned short last;
	unsigned short count;
	int payload[25];
}res_pkt;

int main(void) {

   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   //char sentence[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */

   r_q * rq = malloc(sizeof(r_q));
   res_pkt *rpkt = malloc(sizeof(res_pkt));

   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   
   server_port = SERV_UDP_PORT; /* Server will listen on this port */

   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n", 
                           server_port);

   client_addr_len = sizeof (client_addr);

   for (;;) {
      bytes_sent = 0;
      bytes_recd = recvfrom(sock_server, rq, 4, 0,
                     (struct sockaddr *) &client_addr, &client_addr_len);
      printf("Received Request Packet: \nRequest ID: %d\n Count:  %d\n\n",
                        ntohs(rq->request_ID), 
			ntohs(rq->count));

      /* prepare the message to send */
      
      rpkt->request_ID = ntohs(rq->request_ID);
      rpkt->request_ID = htons(rpkt->request_ID);
      rpkt->seq_number = htons((unsigned short int)1);
      unsigned short int dataToSend = htons(rq->count);
      int pkt_sent = 0;
      unsigned long int sequence_sum = 0;
      unsigned int intSum = 0;
      unsigned int data[25];
      unsigned int temporary_sum;
      int temporary_seqSum;
      while(dataToSend > 0){ /* Loop to keep track of how many packets need to be sent */
	      if(dataToSend <= 25){ //Last Packet
		      rpkt->count = htons(dataToSend);
		      rpkt->last = htons(1);
		      dataToSend = 0;
	      }
	      else{ //Not the last Packet
		      dataToSend -= 25;
		      rpkt->last = htons(0);
		      rpkt->count = htons(25); //Since it is not the last packet, the payload is maxed out at 25 integers
	      }
	      for(int i = 0; i < ntohs(rpkt->count); i++){
			      data[i] = htonl((unsigned int)(rand()));
			      temporary_sum = ntohl(data[i]);
			      intSum +=temporary_sum;
	      }
	      memcpy(rpkt->payload, data, 4*(ntohs(rpkt->count)));

      /* send message */
     
      bytes_sent += sendto(sock_server, rpkt,108, 0,
               (struct sockaddr*) &client_addr, client_addr_len);
      pkt_sent++;
      sequence_sum += ntohs(rpkt->seq_number);
      temporary_seqSum =  ntohs(rpkt->seq_number);
      temporary_seqSum++;
      rpkt->seq_number = htons(temporary_seqSum);
      }
      printf("Request ID: %d\n", ntohs(rpkt->request_ID));
      printf("Count: %d\n", ntohs(rq->count));
      printf("Number of Packets Sent: %d\n", pkt_sent);
      printf("Number of Bytes Sent: %d\n",bytes_sent);
      printf("Sequence Number sum: %lu\n", sequence_sum);
      printf("Checksum: %u\n\n", (unsigned int)intSum);

      
   }
}
