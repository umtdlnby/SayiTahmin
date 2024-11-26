#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib") // Winsock kütüphanesi bağlantısı

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_GUESSES 5

// Global game variables
int target_number;
int attempts_left;

void start_game() {
    // Rastgele sayı üretecini ve oyun durumunu başlat
    srand(time(NULL));
    target_number = rand() % 100 + 1; // 1 ile 100 arasında rastgele sayı
    attempts_left = MAX_GUESSES;
}

void process_guess(SOCKET client_socket, int guess) {
    char response[BUFFER_SIZE];
    
    if (guess == target_number) {
        snprintf(response, sizeof(response), "Congratulations! You guessed the number %d!\n", target_number);
        send(client_socket, response, strlen(response), 0);
        start_game();  // Doğru tahminden sonra oyunu yeniden başlatın
    } else {
        attempts_left--;
        if (attempts_left > 0) {
            snprintf(response, sizeof(response), "Wrong guess. You have %d attempts left. Try again.\n", attempts_left);
            send(client_socket, response, strlen(response), 0);
        } else {
            snprintf(response, sizeof(response), "Game over! The number was %d.\n", target_number);
            send(client_socket, response, strlen(response), 0);
            start_game();  // Denemeler bittikten sonra oyunu yeniden başlatın
        }
    }
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_len = sizeof(client_address);
    char buffer[BUFFER_SIZE];
    int guess;

    // Winsock başlat
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock baslatilamadi. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    // Soket oluştur
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Soket olusturulamadi. Hata kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Sunucu adresini ayarla
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Soketi bağla
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("Bind başarisiz. Hata kodu: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Dinleme başlat
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        printf("Listen basarisiz. Hata kodu: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    start_game(); // game logic

    printf("Sunucu baslatildi. istemciler bekleniyor...\n");

    // İstemcilerden gelen bağlantıları kabul et
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len)) != INVALID_SOCKET) {
        printf("Yeni istemci baglandi.\n");

        // Her client için oyun döngüsü
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                printf("istemci baglantisi kapandi.\n");
                break;
            }

            sscanf(buffer, "%d", &guess);
            process_guess(client_socket, guess);  // Process the guess

        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
