#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12000
#define WORD_LEN 5

#define MAX_GUESSES 6

#define BUFFER_SIZE 999

int main() {
    int guess_socket;
    struct sockaddr_in server_address;
    char guess[WORD_LEN + 99], word[WORD_LEN + 1], feedback[WORD_LEN + 1], result[8];

    char buffer[BUFFER_SIZE + 1];

    guess_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    connect(guess_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    
    // Waiting.

    memset(buffer, 0, sizeof(buffer));
    read(guess_socket, buffer, BUFFER_SIZE);
    buffer[BUFFER_SIZE] = '\0';
    printf("Your role is: %s\n", buffer);

    memset(buffer, 0, sizeof(buffer));
    read(guess_socket, buffer, BUFFER_SIZE);
    buffer[BUFFER_SIZE] = '\0';
    printf("%s\n", buffer);

    // memset(buffer, 0, sizeof(buffer));
    // read(guess_socket, buffer, BUFFER_SIZE);
    // buffer[BUFFER_SIZE] = '\0';
    // printf("%s\n", buffer);

    for (int i = 0; i < MAX_GUESSES; i++) {

        // Actual guessing part.

        printf("Enter your guess (%d/%d): ", i + 1, MAX_GUESSES);
        
        fgets(guess, sizeof(guess), stdin);
        guess[strcspn(guess, "\n")] = 0;

        if (strlen(guess) != WORD_LEN) {
            printf("Guess must be exactly %d characters.\n", WORD_LEN);
            memset(guess, 0, sizeof(guess));
            i--;
            continue;
        }

        send(guess_socket, guess, WORD_LEN, 0);

        memset(word, 0, sizeof(word));
        read(guess_socket, word, WORD_LEN);
        word[WORD_LEN] = '\0';
        printf("Feedback: %s\n", word);
        memset(feedback, 0, sizeof(feedback));
        read(guess_socket, feedback, WORD_LEN);
        feedback[WORD_LEN] = '\0';
        printf("Feedback: %s (G=correct, Y=wrong pos, B=wrong letter)\n", feedback);

        memset(result, 0, sizeof(result));
        read(guess_socket, result, sizeof(result));
        result[sizeof(result)] = '\0';

        if (strcmp(result, "WIN") == 0 || strcmp(result, "LOSE") == 0) {
            printf("Game Result: %s\n", result);
            break;
        } else {
            printf("%s\n", result);
        }
    }

    close(guess_socket);
    return 0;
}
