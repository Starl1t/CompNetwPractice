#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int sendMessage(SOCKET ConnectSocket)
{
   int iResult;
   std::string message;

   while(1) {

      std::getline(std::cin, message);

      // Send an initial buffer
      iResult = send(ConnectSocket, message.c_str(), message.length(), 0);
      if (iResult == SOCKET_ERROR) {
         printf("send failed with error: %d\n", WSAGetLastError());
         closesocket(ConnectSocket);
         WSACleanup();
         return 1;
      }

   }
}


int main()
{
   WSADATA wsaData;
   SOCKET ConnectSocket = INVALID_SOCKET;
   struct addrinfo* result = NULL,
      * ptr = NULL,
      hints;
   const char* sendbuf = "this is a tester";
   char recvbuf[DEFAULT_BUFLEN];
   int iResult;
   int recvbuflen = DEFAULT_BUFLEN;

   // Initialize Winsock
   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (iResult != 0) {
      printf("WSAStartup failed with error: %d\n", iResult);
      return 1;
   }

   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;

   // Resolve the server address and port
   iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &result);
   if (iResult != 0) {
      printf("getaddrinfo failed with error: %d\n", iResult);
      WSACleanup();
      return 1;
   }

   // Attempt to connect to an address until one succeeds
   for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

      // Create a SOCKET for connecting to server
      ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
         ptr->ai_protocol);
      if (ConnectSocket == INVALID_SOCKET) {
         printf("socket failed with error: %ld\n", WSAGetLastError());
         WSACleanup();
         return 1;
      }

      // Connect to server.
      iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
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

   printf("Connected\n");

   //// Send an initial buffer
   //iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
   //if (iResult == SOCKET_ERROR) {
   //   printf("send failed with error: %d\n", WSAGetLastError());
   //   closesocket(ConnectSocket);
   //   WSACleanup();
   //   return 1;
   //}

   //printf("Bytes Sent: %ld\n", iResult);


   //// shutdown the connection since no more data will be sent
   //iResult = shutdown(ConnectSocket, SD_SEND);
   //if (iResult == SOCKET_ERROR) {
   //   printf("shutdown failed with error: %d\n", WSAGetLastError());
   //   closesocket(ConnectSocket);
   //   WSACleanup();
   //   return 1;
   //}

   // Receive until the peer closes the connection\

   std::thread inputThread(sendMessage, ConnectSocket);


   do {

      iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
      if (iResult > 0)
      {
         recvbuf[iResult] = '\0';
         printf("%s\n", recvbuf);
      }
      else if (iResult == 0)
         printf("Connection closed\n");
      else
         printf("recv failed with error: %d\n", WSAGetLastError());

   } while (iResult > 0);

   // cleanup
   closesocket(ConnectSocket);
   WSACleanup();

   return 0;
}