/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/


#include "unet.h"

#if !defined(SOCKET_ERROR)
	/** error in socket operation */
	#define SOCKET_ERROR -1
#endif


#define INVALID_SOCKET SOCKET_ERROR

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define SHUT_RD    		0
#define	SHUT_WR				1
#define SHUT_RDWR 		2




/**
This simple low-level implementation assumes a single connection for a single thread. Thus, a static
variable is used for that connection.
On other scenarios, the user must solve this by taking into account that the current implementation of
MQTTPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
*/
static int mysock = INVALID_SOCKET;

int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen)
{
	int rc = 0;
	rc = send(sock, (char *)buf, buflen,0);
	return rc;
}


int transport_getdata(unsigned char* buf, int count)
{
	int rc = recv(mysock, (char *)buf, count, 0);
	//printf("received %d bytes count %d\n", rc, (int)count);
	return rc;
}

int transport_getdatanb(void *sck, unsigned char* buf, int count)
{
	int sock = *((int *)sck); 	/* sck: pointer to whatever the system may use to identify the transport */
	/* this call will return after the timeout set on initialization if no bytes;
	   in your system you will use whatever you use to get whichever outstanding
	   bytes your socket equivalent has ready to be extracted right now, if any,
	   or return immediately */
	int rc = recv(sock, (char *)buf, count, 0);	
	if (rc == -1) {
		/* check error conditions from your system here, and return -1 */
		return 0;
	}
	return rc;
}

/**
return >=0 for a socket descriptor, <0 for an error code
@todo Basically moved from the sample without changes, should accomodate same usage for 'sock' for clarity,
removing indirections
*/


int transport_open(char *hostname, int port)
{
	int rc = -1;
	int sockf;
	int mode = 1;
	struct sockaddr_in address;
	
	YIELD();
	sockf = socket(AF_INET, SOCK_STREAM, 0);
	
	char serverip[20];
	struct hostent *hp;
	
	if ((hp = gethostbyname(hostname)) == NULL)
	{
		return -1;
	}
	
	strcpy(serverip, inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]));
	
	//remote
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = inet_addr(serverip);//inet_addr("192.168.1.26");
	
	ioctlsocket(sockf,FIONBIO,&mode);
	rc = connect(sockf, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	
	if (mysock == INVALID_SOCKET)
		return rc;

	mysock = sockf;
	
	return mysock;
}


int transport_close(int sock)
{
	int rc;
	
	YIELD();
	
	rc = shutdown(sock, SHUT_WR);
	rc = recv(sock, NULL, (size_t)0, 0);
	//rc = close(sock);
	rc = closesocket(sock);
	
	return rc;
}
