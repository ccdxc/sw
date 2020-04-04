
#define WIN32_LEAN_AND_MEAN
#define _CRT_RAND_S

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int
setsock_tcp_mss( SOCKET inSock, int inMSS );

#define DEFAULT_BUFLEN 1024*100
#define DEFAULT_PORT "27015"

int 
client( char *ServerAddr) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char *sendbuf = NULL;
    char *recvbuf = NULL;
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    int remaining = 0;
    ULONG *data = NULL;
    ULONG index = 0;
    ULONG sendlen = 0;
    
    sendbuf = (char *)malloc( DEFAULT_BUFLEN);

    if (sendbuf == NULL) {
        goto cleanup;
    }

    recvbuf = (char *)malloc( DEFAULT_BUFLEN);
    if (recvbuf == NULL) {
        goto cleanup;
    }

    data = (ULONG *)sendbuf;

    for (remaining = 0,index = 0; remaining < DEFAULT_BUFLEN; remaining += sizeof(ULONG), index++) {
        *data = index;
        data++;
    }
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(ServerAddr, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    for(index = 0; index < 0xFFFFFFFF;index++) {

        sendlen = 0;
        rand_s( (unsigned int *)&sendlen);

        sendlen = sendlen % DEFAULT_BUFLEN;

        sendlen = ((sendlen/sizeof(ULONG)) + 1) * sizeof( ULONG);

        while (sendlen > DEFAULT_BUFLEN)
        {
            sendlen -= sizeof( ULONG);
        }

        *((ULONG *)sendbuf) = sendlen;

        // Send an initial buffer
        iResult = send( ConnectSocket, (const char *)sendbuf, sendlen, 0 );
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        printf("Bytes Sent: %ld\n", iResult);

        iResult = 0;

        while( iResult < (int)sendlen) 
        {
            iResult += recv(ConnectSocket, recvbuf, sendlen, 0);
            if ( iResult == 0 )
                break;
        }
    }

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
            printf("Bytes received: %d\n", iResult);
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

cleanup:

    return 0;
}

int
setsock_tcp_mss( SOCKET inSock, int inMSS ) 
{
    int rc;
    int newMSS;
    int len = sizeof( int);

    if ( inMSS > 0 ) {

        /* set */
        newMSS = inMSS;
        len = sizeof( newMSS );
        rc = setsockopt( inSock, IPPROTO_TCP, TCP_MAXSEG, (char*) &newMSS,  len );
        if ( rc != 0 ) {
            printf("Failed to set MSS Error %d\n", WSAGetLastError());
            return rc;
        }

        /* verify results */
        rc = getsockopt( inSock, IPPROTO_TCP, TCP_MAXSEG, (char*) &newMSS, &len );
        if ( rc != 0 ||
             newMSS != inMSS ) {
            printf("Failed(2) to query MSS Error %d\n", WSAGetLastError());
            return rc;
        }
    }

    return 0;
}
