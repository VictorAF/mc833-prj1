#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 4444

// OBS:
// Default Buffer sizes: Buffer size only directly affects buffered mode. The default size like the kernel is based on the page size (4096 bytes on my system) if stdin/stdout are connected to a terminal then default size = 1024; else size = 4096.

// Problem: When this client disconnects, the server gets an anormal behavior.

int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[4096];
  char * pch;
	struct timeval tv1, tv2, elapsed_timeval;

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	while(1){
		printf("Client: \t");
		fgets(buffer, sizeof(buffer), stdin);

    // Cleans \n character in the end of buffer
    buffer[strcspn(buffer, "\n")] = 0;

		send(clientSocket, buffer, sizeof(buffer), 0);
		gettimeofday(&tv1, NULL);

		if(strcmp(buffer, ":exit") == 0){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}

    // Clean buffer before receiving
    bzero(buffer, sizeof(buffer));

		if(recv(clientSocket, buffer, 4096, 0) < 0){
			printf("[-]Error in receiving data.\n");
		}else{
			printf("Server: \t%s\n", buffer);
		}
		gettimeofday(&tv2, NULL);
		timersub(&tv1, &tv2, &elapsed_timeval);

		if(elapsed_timeval.tv_sec < 0){
		  elapsed_timeval.tv_sec = 0;
		}

		printf("Elapsed time for request/response: %ld:%d (sec:usec)\n", elapsed_timeval.tv_sec, elapsed_timeval.tv_usec);
	}

	return 0;
}
