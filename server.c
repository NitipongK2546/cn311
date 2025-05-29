#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12000

#define MAX_GUESSES 6
#define WORD_LEN 5

typedef struct
{
    int host_sock;
    int guess_sock;
} game_session_args;

// Function to determine feedback color
void color_answer(char *guess, char *word, char *color)
{
    for (int i = 0; i < WORD_LEN; i++)
    {
        if (guess[i] == word[i])
        {
            color[i] = 'G'; // Green: correct position
        }
        else if (strchr(word, guess[i]))
        {
            color[i] = 'Y'; // Yellow: wrong position
        }
        else
        {
            color[i] = 'B'; // Black: not in word
        }
    }
    color[WORD_LEN] = '\0';
}

// This function will be executed by each new game (thread)
void *handle_game(void *arg)
{
    game_session_args *args = (game_session_args *)arg;
    int client_host = args->host_sock;
    int client_guess = args->guess_sock;

    char word[WORD_LEN + 1];
    char guess[WORD_LEN + 1];
    char color[WORD_LEN + 1];

    // Role assignment
    send(client_host, "Word Setter", strlen("Word Setter"), 0);
    send(client_guess, "Word Guesser\n", strlen("Word Guesser\n"), 0);
    send(client_guess, "Waiting for word...", strlen("Waiting for word..."), 0);

    // Receive word from host client
    read(client_host, word, WORD_LEN);
    word[WORD_LEN] = '\0';
    printf("Received word from Client (host_sock %d): %s\n", client_host, word);

    send(client_guess, "Ready", strlen("Ready"), 0);

    // Game loop
    for (int i = 0; i < MAX_GUESSES; i++)
    {
        int valread = read(client_guess, guess, WORD_LEN);
        if (valread <= 0)
        {
            printf("Client (guess_sock %d) disconnected or error.\n", client_guess);
            break; // Exit loop if client disconnects
        }
        guess[WORD_LEN] = '\0';
        printf("Guess %d from (guess_sock %d): %s\n", i + 1, client_guess, guess);

        color_answer(guess, word, color);
        send(client_guess, guess, strlen(guess), 0);
        send(client_guess, color, strlen(color), 0);

        if (strcmp(guess, word) == 0)
        {
            char *msg = "WIN";
            send(client_guess, msg, strlen(msg), 0);
            send(client_host, "The other player won.", strlen("The other player won."), 0);
            printf("Game finished: WIN for guesser (guess_sock %d).\n", client_guess);
            break;
        }
        else if (i == MAX_GUESSES - 1)
        {
            char *msg = "LOSE";
            send(client_guess, msg, strlen(msg), 0);
            send(client_host, "The other player lost.", strlen("The other player lost."), 0);
            printf("Game finished: LOSE for guesser (guess_sock %d).\n", client_guess);
        }
        else
        {
            char *msg = "CONTINUE";
            send(client_guess, msg, strlen(msg), 0);
            send(client_host, "The other player guessed incorrectly.", strlen("The other player guessed incorrectly."), 0);
        }
    }

    close(client_host);
    close(client_guess);
    free(args);
    printf("Game session with host_sock %d and guess_sock %d ended.\n", client_host, client_guess);
    pthread_exit(NULL); // Terminate the thread
}

int main()
{
    int listen_socket;
    struct sockaddr_in server_address;
    int addrlen = sizeof(server_address);

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1;
    listen(listen_socket, 5);

    printf("Server listening on port %d...\n", PORT);

    while (1)
    { // Main server loop to accept new connections
        int client_host_sock, client_guess_sock;

        printf("Waiting for Client 1 (Word Setter)...\n");
        client_host_sock = accept(listen_socket, (struct sockaddr *)&server_address, (socklen_t *)&addrlen);
        printf("Client 1 connected (socket %d).\n", client_host_sock);

        printf("Waiting for Client 2 (Word Guesser)...\n");
        client_guess_sock = accept(listen_socket, (struct sockaddr *)&server_address, (socklen_t *)&addrlen);
        printf("Client 2 connected (socket %d).\n", client_guess_sock);

        // Prepare arguments for the new thread
        game_session_args *args = (game_session_args *)malloc(sizeof(game_session_args));

        args->host_sock = client_host_sock;
        args->guess_sock = client_guess_sock;

        pthread_t game_thread;

        if (pthread_create(&game_thread, NULL, handle_game, (void *)args) != 0) {
            perror("Failed to create thread");
            close(client_host_sock);
            close(client_guess_sock);
            free(args);
            continue;
        }

        pthread_detach(game_thread);
        printf("New game thread created for host_sock %d and guess_sock %d.\n", client_host_sock, client_guess_sock);
    }

    close(listen_socket); // This line is technically unreachable in the infinite loop
    return 0;
}