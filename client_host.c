#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12000
#define WORD_LEN 5

#define BUFFER_SIZE 999

int main() {
    int host_socket;
    struct sockaddr_in server_address;
    char word[WORD_LEN + 1];

    char buffer[BUFFER_SIZE + 1];

    host_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    connect(host_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    // Get the role you have: You joined first -> Word Setter.
    read(host_socket, buffer, BUFFER_SIZE);
    buffer[BUFFER_SIZE] = '\0';
    printf("Your role is: %s\n", word);

    // Send word to the server to get the other client to guess.
    printf("Enter a 5-letter word: ");
    fgets(word, sizeof(word), stdin);
    word[strcspn(word, "\n")] = 0;
    //Sending
    send(host_socket, word, WORD_LEN, 0);
    printf("Word sent to server.\n");

    close(host_socket);
    return 0;
}
