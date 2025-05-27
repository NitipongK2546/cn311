#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12000
#define WORD_LEN 5

int main() {
    int host_socket;
    struct sockaddr_in server_address;
    char word[WORD_LEN + 1];

    host_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    connect(host_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    printf("Enter a 5-letter word: ");
    fgets(word, sizeof(word), stdin);
    word[strcspn(word, "\n")] = 0;

    send(host_socket, word, WORD_LEN, 0);
    printf("Word sent to server.\n");

    close(host_socket);
    return 0;
}
