#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <vector>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

std::vector<SOCKET> Clients;

// Sending message for everyone except sender
void sendEveryone(char *message, int len, SOCKET SenderSocket) 
{
   for (auto const& client : Clients) 
   {
      if (client != SenderSocket) 
      {
         send(client, message, len, 0);
      }
   }
}

int handleClient(SOCKET ClientSocket)
{

   int iResult = 0;
   char recvbuf[DEFAULT_BUFLEN];
   int recvbuflen = DEFAULT_BUFLEN;

   do {

      iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
      /*sendEveryone(recvbuf, iResult, ClientSocket);*/
      if (iResult > 0) {
         sendEveryone(recvbuf, iResult, ClientSocket);
      }
      else if (iResult == 0)
      {
         printf("Connection closing...\n");
      }
      else {
         printf("recv failed with error: %d\n", WSAGetLastError());
         closesocket(ClientSocket);
         //WSACleanup();
         return 1;
      }


   } while (iResult > 0);


   //for (auto const& client : Clients) 
   //{
   //   do {

   //      iSendResult = send(client, "Connected new client", 21, 0);
   //      if (iSendResult == SOCKET_ERROR) {
   //         printf("send failed with error: %d\n", WSAGetLastError());
   //         closesocket(ClientSocket);
   //         WSACleanup();
   //         return 1;
   //      }

   //      iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
   //      if (iResult > 0) {
   //         printf("Bytes received: %d\n", iResult);

   //         // Echo the buffer back to the sender
   //         iSendResult = send(ClientSocket, recvbuf, iResult, 0);
   //         if (iSendResult == SOCKET_ERROR) {
   //            printf("send failed with error: %d\n", WSAGetLastError());
   //            closesocket(ClientSocket);
   //            WSACleanup();
   //            return 1;
   //         }
   //         printf("Bytes sent: %d\n", iSendResult);
   //      }
   //      else if (iResult == 0)
   //         printf("Connection closing...\n");
   //      else {
   //         printf("recv failed with error: %d\n", WSAGetLastError());
   //         closesocket(ClientSocket);
   //         WSACleanup();
   //         return 1;
   //      }

   //   } while (iResult > 0);
   //}

   //do {

   //   iSendResult = send(ClientSocket, "Connected 22", 13, 0);
   //   if (iSendResult == SOCKET_ERROR) {
   //      printf("send failed with error: %d\n", WSAGetLastError());
   //      closesocket(ClientSocket);
   //      WSACleanup();
   //      return 1;
   //   }

   //   //iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
   //   //if (iResult > 0) {
   //   //   printf("Bytes received: %d\n", iResult);

   //   //   // Echo the buffer back to the sender
   //   //   iSendResult = send(ClientSocket, recvbuf, iResult, 0);
   //   //   if (iSendResult == SOCKET_ERROR) {
   //   //      printf("send failed with error: %d\n", WSAGetLastError());
   //   //      closesocket(ClientSocket);
   //   //      WSACleanup();
   //   //      return 1;
   //   //   }
   //   //   printf("Bytes sent: %d\n", iSendResult);
   //   //}
   //   //else if (iResult == 0)
   //   //   printf("Connection closing...\n");
   //   //else {
   //   //   printf("recv failed with error: %d\n", WSAGetLastError());
   //   //   closesocket(ClientSocket);
   //   //   WSACleanup();
   //   //   return 1;
   //   //}

   //} while (iResult > 0);

   ////// cleanup
   ////closesocket(ClientSocket);

}


int main()
{
   WSADATA wsaData;
   int iResult;

   SOCKET ListenSocket = INVALID_SOCKET;
   SOCKET ClientSocket = INVALID_SOCKET;

   struct addrinfo* result = NULL;
   struct addrinfo hints;

   int iSendResult;
   char recvbuf[DEFAULT_BUFLEN];
   int recvbuflen = DEFAULT_BUFLEN;

   // Initialize Winsock
   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
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
   if (iResult != 0) {
      printf("getaddrinfo failed with error: %d\n", iResult);
      WSACleanup();
      return 1;
   }

   // Create a SOCKET for the server to listen for client connections.
   ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
   if (ListenSocket == INVALID_SOCKET) {
      printf("socket failed with error: %ld\n", WSAGetLastError());
      freeaddrinfo(result);
      WSACleanup();
      return 1;
   }

   // Setup the TCP listening socket
   iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
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

   while (1) {
      // Accept a client socket
      ClientSocket = accept(ListenSocket, NULL, NULL);
      if (ClientSocket == INVALID_SOCKET) {
         printf("accept failed with error: %d\n", WSAGetLastError());
         closesocket(ListenSocket);
         WSACleanup();
         return 1;
      }

      Clients.push_back(ClientSocket);

      std::thread th(handleClient, ClientSocket);
      th.detach();

   }


   // cleanup
   closesocket(ListenSocket);
   WSACleanup();

   return 0;
}