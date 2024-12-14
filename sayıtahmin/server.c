#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <pthread.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 10

void *handle_client(void *arg);
void generate_number(int *number);

typedef struct {
    SOCKET client_socket;
    int client_id;
} client_info;

SOCKET client_sockets[MAX_CLIENTS];
pthread_t client_threads[MAX_CLIENTS];
int target_number;
int client_count = 0;

int main() {
    WSADATA wsa;
    SOCKET server_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);
    struct timeval timeout;

    // WSA başlangıcı yapp
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code : %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    // Rastgele sayı oluştur
    srand(time(NULL));
    generate_number(&target_number);
    printf("Generated target number: %d\n", target_number); // Hedef sayıyı terminalde göster

    // Sunucu soketi oluşturma
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code : %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Binding ip baglamayı sagglıyck
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Binding failed. Error Code : %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Dinleme
    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Listening failed. Error Code : %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // İstemci bağlantısını kabul etme
        SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Client connection failed. Error Code : %d\n", WSAGetLastError());
            continue;
        }

        client_info *cinfo = (client_info *)malloc(sizeof(client_info));
        if (cinfo == NULL) {
            printf("Memory allocation failed for client_info\n");
            closesocket(client_socket);
            continue;
        }
        cinfo->client_socket = client_socket;
        cinfo->client_id = client_count + 1;

        client_sockets[client_count] = client_socket;
        pthread_create(&client_threads[client_count], NULL, handle_client, (void *)cinfo);
        printf("Client %d connected. Ready to play the guessing game.\n", cinfo->client_id);
        client_count++;
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

void *handle_client(void *arg) {
    client_info *cinfo = (client_info *)arg;
    if (cinfo == NULL) {
        printf("Invalid client_info pointer\n");
        return NULL;
    }
    SOCKET client_socket = cinfo->client_socket;
    int client_id = cinfo->client_id;
    char buffer[1024];
    int guess;
    struct timeval timeout;
    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);

        // Timeout ayarı (30 saniye)
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;

        int activity = select(0, &readfds, NULL, NULL, &timeout);
        if (activity == 0) {
            printf("Client %d timed out due to inactivity.\n", client_id);
            break;
        }
        if (activity == SOCKET_ERROR) {
            printf("Select failed. Error Code : %d\n", WSAGetLastError());
            break;
        }

        int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (n == SOCKET_ERROR) {
            printf("Recv failed. Error Code : %d\n", WSAGetLastError());
            break;
        }

        buffer[n] = '\0';
        guess = atoi(buffer);

        if (guess == target_number) {
            send(client_socket, "Congratulations! You guessed the right number.\n", 47, 0);
            printf("Client %d guessed the correct number.\n", client_id);
            generate_number(&target_number); // Yeni sayı oluştur
            printf("Generated new target number: %d\n", target_number); // Yeni hedef sayıyı göster
        } else if (guess < target_number) {
            send(client_socket, "Higher!\n", 8, 0);
            printf("Client %d guessed too low: %d\n", client_id, guess);
        } else {
            send(client_socket, "Lower!\n", 7, 0);
            printf("Client %d guessed too high: %d\n", client_id, guess);
        }
    }

    closesocket(client_socket);
    free(cinfo);
    return NULL;
}

void generate_number(int *number) {
    *number = rand() % 100 + 1; // 1-100 arasında bir sayı
}
