// receiver.c - UDP Receiver with support for large data buffers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define UDP_PORT 54321
#define MAX_BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Set up server_addr structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address and port
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Receiver is waiting for data...\n");

    while (1) {
        char received_data[MAX_BUFFER_SIZE];
        int bytes_received = recvfrom(sockfd, received_data, MAX_BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_addr, &client_len);

        if (bytes_received <= 0) {
            perror("Receive error");
            break;
        }

        received_data[bytes_received] = '\0';
        printf("Received data: %s\n", received_data);
    }

    // Close the socket
    close(sockfd);
    printf("Receiver program terminated.\n");
    return 0;
}

