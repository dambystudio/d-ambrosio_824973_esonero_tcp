/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#define NO_ERROR 0

void errorhandler(char *errorMessage) {
    printf("%s", errorMessage);
}

void clearwinsock() {
	#if defined WIN32
		WSACleanup();
	#endif
}

void istruzioni() {
    printf("Istruzioni d'uso: client-project [-s server] [-p port] -r \"type city\"\n");
    printf("  -s server : Server IP (default: 127.0.0.1)\n");
    printf("  -p port  : Server port (default: 56700)\n");
    printf("  -r request: Weather request \"t|h|w|p city\" (OBBLIGATORIO)\n");
    printf("\nEsempio: client-project -r \"t bari\"\n");
}

void print_result(char *server_ip, weather_response_t response, char *city) {
    printf("Ricevuto risultato dal server ip %s. ", server_ip);

	if (response.status == STATUS_SUCCESS) {
        city[0] = toupper(city[0]);
        
        switch(response.type) {
            case TYPE_TEMPERATURE:
                printf("%s: Temperatura = %.1f°C.\n", city, response.value);
                break;
            case TYPE_HUMIDITY:
                printf("%s: Umidità = %.1f%%\n", city, response.value);
                break;
            case TYPE_WIND:
                printf("%s: Vento = %.1f km/h.\n", city, response.value);
                break;
            case TYPE_PRESSURE:
                printf("%s: Pressione = %.1f hPa.\n", city, response.value);
                break;
        }
	}
	
	else if (response.status == STATUS_CITY_NOT_FOUND) {
    	printf("Città non disponibile.\n");
    }

    else if (response.status == STATUS_INVALID_REQUEST) {
        printf("Richiesta non valida.\n");
    }
}

int main(int argc, char *argv[]) {

    char server_ip[16] = "127.0.0.1";
    int serverPort = SERVER_PORT;
    char request_string[128] = "";
    int has_request = 0;

    // PARSING ARGOMENTI
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strcpy(server_ip, argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
        	serverPort = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            strcpy(request_string, argv[i + 1]);
            has_request = 1;
            i++;
        }
    }

    if (!has_request) {
        printf("Errore: argomento -r obbligatorio\n");
        istruzioni();
        return -1;
    }

	#if defined WIN32
		// Initialize Winsock
		WSADATA wsa_data;
		int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
		if (result != NO_ERROR) {
			printf("Error at WSAStartup()\n");
			return 0;
		}
	#endif

	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket <0){
		errorhandler("socket creation failed.\n");
		clearwinsock();
		return -1;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	if (connect(my_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        errorhandler("Connessione al server fallita.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

	weather_request_t request;
    memset(&request, 0, sizeof(request));
	request.type = request_string[0];

	if (strlen(request_string) > 2) {
        strcpy(request.city, &request_string[2]);  
    } else {
        printf("Errore: formato richiesta non valido\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

	for (int i = 0; request.city[i]; i++) {
        request.city[i] = tolower((unsigned char)request.city[i]);
    }

	if (send(my_socket, (char*)&request, sizeof(request), 0) < 0) {
        errorhandler("Invio richiesta fallito.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

	weather_response_t response;
    int bytes_rcvd = recv(my_socket, (char*)&response, sizeof(response), 0);
    if (bytes_rcvd <= 0) {
        errorhandler("Ricezione risposta fallita.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

	print_result(server_ip, response, request.city);

	closesocket(my_socket);
	printf("Client terminated.\n");
	clearwinsock();
	return 0;
} // main end
