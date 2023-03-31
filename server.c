#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

#define PORT 33333

void handle_client(int client_socket);

void send_error_404(int client_socket);

void send_file_content(int client_socket, char *filename);

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    printf("Servidor Iniciado\n");

    while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("accept()");
            exit(EXIT_FAILURE);
        }

        handle_client(new_socket);

        close(new_socket);
    }

    return 0;
}

void handle_client(int client_socket) {
    char buffer[30000] = {0};
    long valread = read(client_socket, buffer, sizeof(buffer));

    if (valread < 0) {
        perror("read()");
        exit(EXIT_FAILURE);
    }

    printf("Mensagem do cliente: %s\n", buffer);

    char *method = strtok(buffer, " ");
    char *url = strtok(NULL, " ");
    char *filename = url + 1;

    if (access(filename, F_OK) != 0) {
        send_error_404(client_socket);
        printf("Arquivo não encontrado: %s\n", filename);
    } else {
        send_file_content(client_socket, filename);
        printf("Arquivo enviado: %s\n", filename);
    }
}

void send_error_404(int client_socket) {
    char *response_header = "HTTP/1.0 404 Not Found\nContent-Type: text/plain\n\n";
    write(client_socket, response_header, strlen(response_header));
    char *response_body = "Arquivo não encontrado\n";
    write(client_socket, response_body, strlen(response_body));
}

void send_file_content(int client_socket, char *filename) {
    char *response_header = "HTTP/1.0 200 OK\nContent-Type: text/plain\n\n";
    write(client_socket, response_header, strlen(response_header));

    int fd = open(filename, O_RDONLY);
    char file_content[30000] = {0};
    int nbytes = read(fd, file_content, sizeof(file_content));
    close(fd);

    write(client_socket, file_content, nbytes);
}
