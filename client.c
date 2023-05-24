#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #define SERVER_IP "127.0.0.1"
// #define PORT 18000

int main(int argc, char *argv[]) {
    if (argc != 4) {
        puts("Please enter the port number and ip adress");
        exit(EXIT_FAILURE);
    }
    const int PORT = atoi(argv[1]);
    const char *SERVER_IP = argv[2];
    int sock = 0;
    int share = atoi(argv[3]);
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    } else {
        puts("Socket created");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    } else {
        puts("inet_pton done");
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    } else {
        puts("connected");
    }
   
    char buffer[1024] = {0};
    if (recv(sock, buffer, sizeof(int), 0) > 0) {
        int received = atoi(buffer);
        printf("Received share: %d\n", received);
        printf("Checking share...\n");
        sleep(1);
        if (share != received) {
            char msg[] = "Not matching!";
            puts(msg);
            send(sock, msg, strlen(msg), 0);
        } else {
            char msg[] = "Matching, sir";
            puts(msg);
            send(sock, msg, strlen(msg), 0);
        }
    }
    close(sock);
    return 0;
}
