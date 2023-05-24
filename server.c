#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

// #define PORT 18000
#define NUM_SONS 8

int shares[NUM_SONS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *client_handler(void *arg)
{
    int client_socket = *((int *)arg);
    int n = *(((int *)arg) + 1);
    // Отправляем сообщение о случайной смерти цветка каждую секунду
    char msg[1024] = {0};
    pthread_mutex_lock(&mutex);
    shares[n] = rand() % 1000;
    pthread_mutex_unlock(&mutex);
    sprintf(msg, "%d", shares[n]);
    send(client_socket, msg, sizeof(int), 0);
    printf("%d's share is %d\n", n, shares[n]);
    char buffer[1024] = {0};
    if (recv(client_socket, buffer, 14, 0) > 0)
    {
        if (strncmp(buffer, "Matching, sir", 14) == 0) {
            printf("Of course it's matching!\n");
        } else if (strncmp(buffer, "Not matching!", 14) == 0) {
            printf("Oops! My mistake, Mr. %d.\n", n);
        } else {
            puts("I can't quite understand you, sir...");
        }
    } else {
        puts("recv error");
    }
    close(client_socket);
    free(arg);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        puts("Please enter the port number");
        exit(EXIT_FAILURE);
    }
    const int PORT = atoi(argv[1]);
    int server_fd, new_socket[NUM_SONS];
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t thread_id[NUM_SONS];
    pthread_t sender_id;

    for (int i = 0; i < NUM_SONS; i++)
    {
        shares[i] = 0;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        puts("Socket created");
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        puts("Bind successful");
    }
    if (listen(server_fd, 2) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    else
    {
        puts("Listening");
    }

    for (int i = 0; i < NUM_SONS; ++i)
    {
        if ((new_socket[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Accepting client %d\n", i);
        }

        int *arg = malloc(2 * sizeof(int));
        arg[0] = new_socket[i];
        arg[1] = i;
        if (pthread_create(&thread_id[i], NULL, client_handler, (void *)arg) < 0)
        {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < NUM_SONS; ++i) {
        pthread_join(thread_id[i], NULL);
    }
    return 0;
}
