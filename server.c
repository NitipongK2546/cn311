#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12000

#define MAX_GUESSES 6
#define WORD_LEN 5

void give_feedback(char *guess, char *word, char *feedback) {
    for (int i = 0; i < WORD_LEN; i++) {
        if (guess[i] == word[i]) {
            feedback[i] = 'G'; // Green: correct position
        } else if (strchr(word, guess[i])) {
            feedback[i] = 'Y'; // Yellow: wrong position
        } else {
            feedback[i] = 'B'; // Black: not in word
        }
    }
    feedback[WORD_LEN] = '\0';
}

int main(int argc, char *argv[]) {

    // if(argc != 2)
    // {
    //     printf("Usage: %s \"<name> <surname> <dateBE>\" \n", argv[0]);
    //     printf("Example: Nitipong Kadsritalee 31012546 \n");
    //     printf("Date: 31 January 2546 B.E. \n");
    //     printf("Warning: Don't forget to quote \"<input>\" your input. \n");
    //     return 1;
    // } 

    // Create socket to listen for port, for host, and for guesser.
    int host_socket, guess_socket, listen_socket;

    struct sockaddr_in server_address; // server address
    int addrlen = sizeof(server_address); // 

    // Create arrays to store word datas, +1 for /0 null terminator.
    char word[WORD_LEN + 1];
    char guess[WORD_LEN + 1];
    char color[WORD_LEN + 1];

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_address.sin_family = AF_INET;            // Use IPV4
    server_address.sin_addr.s_addr = INADDR_ANY;    // Accept any IP
    server_address.sin_port = htons(PORT);          // Use PORT variable -> 12000

    bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(listen_socket, 2);

    printf("Waiting for Client 1...\n");

    host_socket = accept(listen_socket, (struct sockaddr *)&server_address, (socklen_t*)&addrlen);

    send(host_socket, "Word Setter", strlen("Word Setter"), 0);

    read(host_socket, word, WORD_LEN);
    word[WORD_LEN] = '\0';
    printf("Received word from Client 1: %s\n", word);

    printf("Waiting for Client 2 (guesser)...\n");
    guess_socket = accept(listen_socket, (struct sockaddr *)&server_address, (socklen_t*)&addrlen);

    for (int i = 0; i < MAX_GUESSES; i++) {
        int valread = read(guess_socket, guess, WORD_LEN);
        guess[WORD_LEN] = '\0';
        printf("Guess %d: %s\n", i + 1, guess);

        give_feedback(guess, word, color);
        send(guess_socket, color, strlen(color), 0);

        if (strcmp(guess, word) == 0) {
            char *msg = "WIN";
            send(guess_socket, msg, strlen(msg), 0);
            break;
        } else if (i == MAX_GUESSES - 1) {
            char *msg = "LOSE";
            send(guess_socket, msg, strlen(msg), 0);
        } else {
            char *msg = "CONTINUE";
            send(guess_socket, msg, strlen(msg), 0);
        }
    }

    close(host_socket);
    close(listen_socket);
    close(guess_socket);

    return 0;

}