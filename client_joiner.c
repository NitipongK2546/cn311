#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12000
#define WORD_LEN 5

int main() {
    int guess_socket;
    struct sockaddr_in server_address;
    char guess[WORD_LEN + 99], feedback[WORD_LEN + 1], result[8];

    guess_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    connect(guess_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    for (int i = 0; i < 6; i++) {
        printf("Enter your guess (%d/6): ", i + 1);
        
        fgets(guess, sizeof(guess), stdin);
        guess[strcspn(guess, "\n")] = 0;

        if (strlen(guess) != WORD_LEN) {
            printf("Guess must be exactly %d characters.\n", WORD_LEN);
            memset(guess, 0, sizeof(guess));
            i--;
            continue;
        }

        send(guess_socket, guess, WORD_LEN, 0);

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
