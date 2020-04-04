
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

int
setsock_tcp_mss( SOCKET inSock, int inMSS );

#define DEFAULT_BUFLEN 1024*100
#define DEFAULT_PORT "27015"

int 
server(void) 
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char *recvbuf = NULL;
    int recvbuflen = DEFAULT_BUFLEN; 
    int remaining = 0;
    ULONG *data = (ULONG *)recvbuf;
    ULONG index = 0;
    ULONG errorcnt = 0;
    char *curr_recv = NULL;
    int recv_len = 0;
    int full_rec_len = 0;

    recvbuf = (char *)malloc( DEFAULT_BUFLEN);
    if (recvbuf == NULL) {
        goto cleanup;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    //ListenSocket = socket( AF_INET, SOCK_STREAM, 0);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {

        iResult = 0;
        curr_recv = recvbuf;
        full_rec_len = DEFAULT_BUFLEN;

        while( iResult < full_rec_len)
        {
            recv_len = recv(ClientSocket, curr_recv, recvbuflen - iResult, 0);

            if (recv_len >= sizeof(ULONG) &&
                full_rec_len == DEFAULT_BUFLEN) {
                data = (ULONG *)curr_recv;
                full_rec_len = *data;
            }
            iResult += recv_len;
            curr_recv += recv_len;
        }

        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            data = (ULONG *)recvbuf;
            data++;
            errorcnt = 0;

            for (remaining = sizeof(ULONG),index = 1; remaining < iResult; remaining += sizeof(ULONG), index++) {

                if (*data != index) {
                    printf("Bad data at offset %d have %d expected %d\n",
                                        index,
                                        *data,
                                        index);
                    if( errorcnt++ > 10)
                        break;
                }

                data++;
            }

            if( errorcnt != 0)
                return 1;

        // Echo the buffer back to the sender
            iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

cleanup:

    return 0;
}