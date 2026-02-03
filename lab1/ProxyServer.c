#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_CLIENTS 10
#define MAX_BYTES 4096

int main(int argc, char *argv[])
{

    // Check command line arguments
    if (argc <= 1)
    {
        printf("Usage: %s server_ip\n", argv[0]);
        printf("[server_ip: It is the IP Address Of Proxy Server]\n");
        printf("Example: %s 127.0.0.1\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = 8888;

    printf("Starting proxy server on %s:%d\n", server_ip, server_port);

    // Create a server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    // Allow socket reuse
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Set up server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_aton(server_ip, &server_addr.sin_addr);

    // Fill in start.
    // TODO: Bind the socket to (server_ip, server_port)
    // HINT: bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))

    // Fill in end.

    // Fill in start.
    // TODO: Listen for incoming connections (backlog of MAX_CLIENTS)
    // HINT: listen(server_socket, MAX_CLIENTS)

    // Fill in end.

    printf("Proxy server is listening on port %d...\n", server_port);

    while (1)
    {
        printf("\n");
        for (int i = 0; i < 50; i++)
            printf("=");
        printf("\nReady to serve...\n");

        // Accept a client connection
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Error accepting connection");
            continue;
        }

        printf("Received a connection from: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Fill in start.
        // TODO: Receive message from client (use recv with buffer size MAX_BYTES)
        // HINT: int recv_len = recv(client_socket, message, MAX_BYTES - 1, 0);
        char message[MAX_BYTES] = {0};
        int recv_len =

            // Fill in end.

            if (recv_len <= 0)
        {
            printf("Error receiving or client closed connection\n");
            close(client_socket);
            continue;
        }

        message[recv_len] = '\0';
        printf("Request:\n%s\n", message);

        // Extract the filename from the HTTP request
        char request_line[MAX_BYTES] = {0};
        char url[MAX_BYTES] = {0};
        char filename[MAX_BYTES] = {0};
        char filetouse[MAX_BYTES] = {0};

        // Parse the request line (e.g., "GET http://www.example.com/index.html HTTP/1.1")
        char *newline = strchr(message, '\n');
        if (newline == NULL)
        {
            printf("Error parsing request\n");
            close(client_socket);
            continue;
        }

        strncpy(request_line, message, newline - message);
        printf("Request line: %s\n", request_line);

        // Extract URL (second field after GET)
        char *token = strtok(request_line, " ");
        token = strtok(NULL, " "); // Get second token (URL)

        if (token == NULL)
        {
            printf("Error parsing URL from request\n");
            close(client_socket);
            continue;
        }

        strcpy(url, token);
        printf("URL: %s\n", url);

        // Extract filename (remove "http://")
        char *start = strstr(url, "//");
        if (start != NULL)
        {
            strcpy(filename, start + 2);
        }
        else
        {
            strcpy(filename, url);
        }
        printf("Filename: %s\n", filename);

        // Remove 'www.' for cleaner filenames
        strcpy(filetouse, filename);
        char *www_pos = strstr(filetouse, "www.");
        if (www_pos != NULL)
        {
            memmove(www_pos, www_pos + 4, strlen(www_pos + 4) + 1);
        }
        printf("Cache file: %s\n", filetouse);

        // Create cache file path
        char cache_path[MAX_BYTES] = {0};
        snprintf(cache_path, sizeof(cache_path), "cache/%s", filetouse);

        // Try to open the file from cache
        FILE *cache_file = fopen(cache_path, "r");

        if (cache_file != NULL)
        {
            // CACHE HIT: File found in cache
            printf("==================================================\n");
            printf("CACHE HIT! File found in cache\n");
            printf("==================================================\n");

            // Send HTTP response headers
            char response_headers[] = "HTTP/1.0 200 OK\r\nContent-Type:text/html\r\n\r\n";
            send(client_socket, response_headers, strlen(response_headers), 0);

            // Fill in start.
            // TODO: Read and send each line from the cached file
            // HINT: Use fgets() to read a line, then send() to send it
            // HINT: Loop until fgets returns NULL
            char buffer[MAX_BYTES] = {0};

            // Fill in end.

            fclose(cache_file);
            printf("Successfully sent cached file to client\n");
        }
        else
        {
            // CACHE MISS: File not in cache, fetch from web server
            printf("==================================================\n");
            printf("CACHE MISS! Fetching from web server...\n");
            printf("==================================================\n");

            // Extract hostname (remove any path)
            strcpy(request_line, filename);
            char *slash_pos = strchr(request_line, '/');
            if (slash_pos != NULL)
            {
                request_line[slash_pos - request_line] = '\0';
            }

            char hostname[MAX_BYTES] = {0};
            strcpy(hostname, request_line);

            // Remove 'www.' from hostname
            www_pos = strstr(hostname, "www.");
            if (www_pos != NULL)
            {
                memmove(www_pos, www_pos + 4, strlen(www_pos + 4) + 1);
            }

            printf("Connecting to host: %s\n", hostname);

            // Fill in start.
            // TODO: Create a socket for proxy-to-webserver connection
            // HINT: int web_socket = socket(AF_INET, SOCK_STREAM, 0);

            // Fill in end.

            if (web_socket < 0)
            {
                perror("Error creating web socket");
                send(client_socket, "HTTP/1.0 500 Internal Server Error\r\n\r\n", 40, 0);
                close(client_socket);
                continue;
            }

            // Resolve hostname
            struct hostent *hostinfo = gethostbyname(hostname);
            if (hostinfo == NULL)
            {
                printf("Error: Could not resolve hostname %s\n", hostname);
                send(client_socket, "HTTP/1.0 404 Not Found\r\n\r\n", 26, 0);
                close(web_socket);
                close(client_socket);
                continue;
            }

            // Set up web server address
            struct sockaddr_in web_server_addr;
            memset(&web_server_addr, 0, sizeof(web_server_addr));
            web_server_addr.sin_family = AF_INET;
            web_server_addr.sin_port = htons(80); // HTTP port
            memcpy(&web_server_addr.sin_addr, hostinfo->h_addr, hostinfo->h_length);

            // Fill in start.
            // TODO: Connect to the web server on port 80
            // HINT: int connect_result = connect(web_socket, (struct sockaddr *)&web_server_addr, sizeof(web_server_addr));
            int connect_result =

                // Fill in end.

                if (connect_result < 0)
            {
                perror("Error connecting to web server");
                send(client_socket, "HTTP/1.0 404 Not Found\r\n\r\n", 26, 0);
                close(web_socket);
                close(client_socket);
                continue;
            }

            printf("Connected to %s:80\n", hostname);

            // Create HTTP GET request
            char http_request[MAX_BYTES] = {0};
            snprintf(http_request, sizeof(http_request),
                     "GET http://%s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
                     filename, hostname);

            printf("Sending request:\n%s\n", http_request);

            // Send request to web server
            send(web_socket, http_request, strlen(http_request), 0);

            // Fill in start.
            // TODO: Read the response from the web server
            // HINT: Convert socket to FILE* with fdopen(web_socket, "r")
            // HINT: Use fgets() to read lines in a loop until NULL
            FILE *web_file =

                // Fill in end.

                if (web_file == NULL)
            {
                perror("Error opening web socket for reading");
                send(client_socket, "HTTP/1.0 500 Internal Server Error\r\n\r\n", 40, 0);
                close(web_socket);
                close(client_socket);
                continue;
            }

            // Save to cache and send to client
            FILE *save_file = fopen(cache_path, "wb");
            if (save_file == NULL)
            {
                printf("Error: Could not create cache file at %s\n", cache_path);
                send(client_socket, "HTTP/1.0 500 Internal Server Error\r\n\r\n", 40, 0);
                fclose(web_file);
                close(web_socket);
                close(client_socket);
                continue;
            }

            // Fill in start.
            // TODO: Read from web server and forward to client, while saving to cache
            // HINT: Use fgets() to read from web_file
            // HINT: Use fwrite() to save to cache
            // HINT: Use send() to send to client
            // HINT: Loop until fgets returns NULL
            char read_buffer[MAX_BYTES] = {0};

            // Fill in end.

            fclose(save_file);
            fclose(web_file);
            close(web_socket);

            printf("Successfully cached and forwarded response to client\n");
        }

        // Close client connection
        printf("Closing client connection\n");
        close(client_socket);
    }

    // Fill in start.
    // TODO: Close the server socket
    // HINT: close(server_socket)

    // Fill in end.

    return 0;
}
