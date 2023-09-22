// sender.c - UDP Sender with data throughput control via .conf file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>  // Include for clock_gettime

#define UDP_PORT 54321
#define MAX_BUFFER_SIZE 1024
#define CONF_FILENAME "sender.conf"

unsigned int readDataRateFromConf() {
    unsigned int data_rate = 0;
    FILE *conf_file = fopen(CONF_FILENAME, "r");
    if (conf_file != NULL) {
        char line[100]; // Assuming lines are not longer than 100 characters
        while (fgets(line, sizeof(line), conf_file)) {
            if (sscanf(line, "data_rate = %u", &data_rate) == 1) {
                break; // Found and read the data_rate, exit the loop
            }
        }
        fclose(conf_file);
    }
    return data_rate;
}

// Function to limit the sending rate based on the specified data rate
void limitSendingRate(int sockfd, unsigned int data_rate, struct sockaddr_in server_addr) {
    unsigned int bytes_sent = 0;
    struct timespec start_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (1) {
        char input[MAX_BUFFER_SIZE];
        printf("> ");
        fgets(input, MAX_BUFFER_SIZE, stdin);

        // Check if the user wants to quit
        if (input[0] == 'q' && (input[1] == '\n' || input[1] == '\0')) {
            break;
        }

        // Calculate elapsed time
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1e9;


        // Calculate the allowed number of bytes to send based on data_rate and elapsed_time
        unsigned int allowed_bytes = (unsigned int)(data_rate * elapsed_time);
        
        printf("allowed bytes: %d", allowed_bytes);

        if (bytes_sent < allowed_bytes) {
            // Send the data to the receiver using UDP
            if (sendto(sockfd, input, strlen(input), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
                perror("Send error");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            bytes_sent += strlen(input);
            
            // change receiver address
            
            char ip4[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(server_addr.sin_addr), ip4, INET_ADDRSTRLEN);
            
            printf("send to ip: %s\n", ip4);
            
            if (strcmp(ip4,"134.93.182.78") == 0) {
                inet_pton(AF_INET, "134.93.182.163", &server_addr.sin_addr);
            }
            else {
                inet_pton(AF_INET, "134.93.182.78", &server_addr.sin_addr);
            }
            
            
        } else {
            // Rate limit exceeded; sleep briefly to control the sending rate
            printf("rate over run sleeping");
            usleep(1000); // Sleep for 1 millisecond
        }
    }
}

int main() {
    int sockfd;
    
    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Set up server_addr structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, "134.93.182.78", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Enter data to send (type 'q' to quit):\n");

    // Read the data rate from the .conf file
    unsigned int data_rate = readDataRateFromConf();
    if (data_rate == 0) {
        printf("Error: Invalid data rate in the configuration file.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Perform rate-limited sending
    limitSendingRate(sockfd, data_rate, server_addr);

    // Close the socket
    close(sockfd);
    printf("Sender program terminated.\n");
    return 0;
}
