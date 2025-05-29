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

    int client_socket;
    struct sockaddr_in server_address;

    char guess[WORD_LEN + 99], word[WORD_LEN + 1], feedback[WORD_LEN + 1], result[8];

    char buffer[BUFFER_SIZE + 1];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    // client_guess = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    // Get the role you have: You joined first -> Word Setter.
    memset(buffer, 0, sizeof(buffer));
    read(client_socket, buffer, BUFFER_SIZE);
    buffer[BUFFER_SIZE] = '\0';
    printf("Your role is: %s\n", buffer);

    if (strcmp(buffer, "Word Setter") == 0) {

        // Send word to the server to get the other client to guess.
        // Only accept 5 letters or it will keep looping.
        while (1) {
            printf("Enter a 5-letter word: ");
            fgets(word, sizeof(word), stdin);
            word[strcspn(word, "\n")] = 0;  

            if (strlen(word) == 5) {
                break;  
            } else {
                printf("Invalid input. Please enter exactly 5 letters.\n");
            }
        }

        //Sending
        send(client_socket, word, WORD_LEN, 0);
        printf("Word sent to server.\n");

        for (int i = 0; i < MAX_GUESSES; i++) {
            if (strcmp(buffer, "The other player won.") == 0) {
                break;
            } else {
                memset(buffer, 0, sizeof(buffer));
                read(client_socket, buffer, BUFFER_SIZE);
                buffer[BUFFER_SIZE] = '\0';
                printf("%s\n", buffer);
            }
        }


    } else {

        // Waiting.

        memset(buffer, 0, sizeof(buffer));
        read(client_socket, buffer, BUFFER_SIZE);
        buffer[BUFFER_SIZE] = '\0';
        printf("%s\n", buffer);   

        if(strcmp(buffer, "Ready") == 0) {

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

                send(client_socket, guess, WORD_LEN, 0);

                memset(word, 0, sizeof(word));
                read(client_socket, word, WORD_LEN);
                word[WORD_LEN] = '\0';
                printf("Feedback: %s\n", word);
                memset(feedback, 0, sizeof(feedback));
                read(client_socket, feedback, WORD_LEN);
                feedback[WORD_LEN] = '\0';
                printf("Feedback: %s (G=correct, Y=wrong pos, B=wrong letter)\n", feedback);

                memset(result, 0, sizeof(result));
                read(client_socket, result, sizeof(result));
                result[sizeof(result)] = '\0';

                if (strcmp(result, "WIN") == 0 || strcmp(result, "LOSE") == 0) {
                    printf("Game Result: %s\n", result);
                    break;
                } else {
                    printf("%s\n", result);
                }
            }
        }

    }

    close(client_socket);

    return 0;
}
