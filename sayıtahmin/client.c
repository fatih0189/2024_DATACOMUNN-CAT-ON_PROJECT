#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];
    struct timeval timeout;

    // WSA başlangıcı
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code : %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    // İstemci soketi oluşturma
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code : %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    // Timeout ayarı (30 saniye)
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Sunucuya bağlanma
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Connection failed. Error Code : %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Start guessing the number (1-100):\n");

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        send(client_socket, buffer, strlen(buffer), 0);

        int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (n == SOCKET_ERROR) {
            printf("Recv failed. Error Code : %d\n", WSAGetLastError());
            break;
        }
        buffer[n] = '\0';
        printf("Server: %s\n", buffer);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
