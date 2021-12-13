/* udp_client.c */ 
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2021 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

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

   int sock_client;  /* Socket used by client */ 

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned short client_port;  /* Port number used by client (local port) */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */

   r_q * rq = malloc(sizeof(r_q));
   rq->request_ID=htons(0);
   int intsWanted;
   res_pkt *rpkt = malloc(sizeof(res_pkt));

   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Client: can't open datagram socket\n");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound. 
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

   /* initialize client address information */

   client_port = 0;   /* This allows choice of any available local port */

   /* Uncomment the lines below if you want to specify a particular 
             local port: */
   /*
   printf("Enter port number for client: ");
   scanf("%hu", &client_port);
   */

   /* clear client address structure and initialize with client address */
   memset(&client_addr, 0, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one 
                                        are present */
   client_addr.sin_port = htons(client_port);

   /* bind the socket to the local client port */

   if (bind(sock_client, (struct sockaddr *) &client_addr,
                                    sizeof (client_addr)) < 0) {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
   }

   /* end of local address initialization and binding */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname\n");
      close(sock_client);
      exit(1);
   }
   printf("Enter port number for server: ");
   scanf("%hu", &server_port);

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

   /* user interface */

   while(1){
	    printf("Please input the number of integers you would like to receive:\n");
	    scanf("%d", &intsWanted);
	    while(intsWanted > 65535 || intsWanted < 1 ){
	    	printf("Please input the number of integers you would like so receive:\n");
	   	scanf("%d", &intsWanted);
	    }
   	    /* Request Packet Handling */
  	    rq->count =htons((unsigned short)intsWanted);
   	    int id = ntohs(rq->request_ID);
  	    id++;
   	    rq->request_ID = htons(id);

   	    /* send message */
 	     printf("Request Packet:\n");
	     printf("Request ID: %d\n", ntohs(rq->request_ID));
	     printf("Count: %d\n\n", ntohs(rq->count));
  	     bytes_sent = sendto(sock_client, rq ,4 , 0,
            	(struct sockaddr *) &server_addr, sizeof (server_addr));

   	    /* get response from server */
  
   	    printf("Waiting for response from server...\n\n");
	    /* Clearing out variables for new data */
	    int bytesRcv = 0;
   	    int pktRcv = 0;
   	    int checksum = 0;
   	    int index = 0;
   	    int stop = 0;
   	    long int sequenceSum = 0;
   	    unsigned int intSum = 0;

	    
	    
	    while(stop == 0){
	   	 bytes_recd = recvfrom(sock_client, rpkt,108 , 0,
            	    (struct sockaddr *) 0, (int *) 0);
		 if(ntohs(rpkt->request_ID) != ntohs(rq->request_ID)){
			 printf("Wrong Response Number\n");
	   	 }
            bytesRcv += bytes_recd;
	    pktRcv++;
            int sum = ntohs(rpkt->seq_number);
	    sequenceSum += sum;
	    stop = ntohs(rpkt->last);
	    unsigned int temporary_sum = 0;
	    for(int i=0; i<ntohs(rpkt->count);i++){
		 temporary_sum = temporary_sum + ntohl(rpkt->payload[i]);
	    }
	    intSum = intSum + temporary_sum;
	    index++;
	    }

	    /*Response Packet Printing*/
	    printf("\nThe response from server is:\n");
	    printf("Request ID: %d\n", ntohs(rq->request_ID));
	    printf("Count: %d\n", ntohs(rq->count));
	    printf("Number of Response Packets Received: %d\n", pktRcv);
	    printf("Number of Bytes Received: %d\n",bytesRcv);
	    printf("Sequence Number sum: %lu\n", sequenceSum);
	    printf("Checksum: %u\n\n", intSum);
	    printf("Do you want to continue? Type 1 to continue or 0 to exit\n");
	    int continueOn = 0;
	    scanf("%d", &continueOn);
	    if(continueOn==1){
		    printf("Continuing\n\n");
	    }
	    else{
		    printf("Exiting\n");
		    exit(1);
	    }
   }
   /* close the socket */
   close (sock_client);
   free(rq);
   free(rpkt);
}
