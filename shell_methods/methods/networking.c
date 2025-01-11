#include "../helper.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

// Case-insensitive strstr
char* strcasestr(const char *haystack, const char *needle) {
    if (!*needle)
        return (char *)haystack;
    
    for ( ; *haystack; haystack++) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            const char *h, *n;
            for (h = haystack, n = needle; *h && *n; h++, n++) {
                if (tolower((unsigned char)*h) != tolower((unsigned char)*n))
                    break;
            }
            if (!*n)
                return (char *)haystack;
        }
    }
    return NULL;
}

// Function to remove the HTTP headers from the response
char* remove_headers(char *response, int *offset) {
    char *content = strstr(response, "\r\n\r\n");
    if (content != NULL) {
        *offset = (content - response) + 4;  // Return offset to the body after \r\n\r\n
        return response + *offset;
    }
    *offset = 0;
    return response; // If no headers found, return the entire response
}

// Function to extract and clean text content from <PRE> tags
void extract_pre_content(const char *response, FILE* OUTPUT) {
    const char *pre_start = strcasestr(response, "<pre>");
    const char *pre_end = strcasestr(response, "</pre>");

    if (pre_start != NULL && pre_end != NULL && pre_start < pre_end) {
        pre_start += 5;  // Skip over "<pre>" tag

        // Print the content between <pre> and </pre>
        while (pre_start < pre_end) {
            if (*pre_start == '<') {
                // Skip HTML tags
                while (*pre_start != '>' && pre_start < pre_end) {
                    pre_start++;
                }
                pre_start++;  // Skip '>'
            } else {
                fputc(*pre_start, OUTPUT);
                pre_start++;
            }
        }
    } else {
        fprintf(stderr, RED BOLD "ERROR: " RESET RED " No content found.\n");
    }
}

// Function to print the content for a man command
int iman(char** args, FILE* OUTPUT) {
    if (args[1] == NULL) {
        fprintf(stderr, BOLD RED "ERROR:" RESET RED " No command specified\n");
        return 1;
    }

    const char* command_name = args[1];

    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char request[DEFAULT_STRING_SIZE], response[DEFAULT_STRING_SIZE];
    int offset = 0;
    int header_found = 0;

    // Resolve the host (man.he.net)
    server = gethostbyname("man.he.net");
    if (server == NULL) {
        fprintf(stderr, BOLD RED "ERROR:" RESET RED " No such host.\n");
        return 1;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, BOLD RED "ERROR:" RESET RED " Opening socket failed.\n");
        return 1;
    }

    // Prepare server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80); // HTTP port 80
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Connecting");
        close(sockfd);
        return 1;
    }

    // Prepare the GET request with the query parameters
    snprintf(request, sizeof(request),
             "GET /?topic=%s&section=all HTTP/1.1\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n"
             "\r\n",
             command_name);  // Include the topic in the request

    // Send the GET request to the server
    if (send(sockfd, request, strlen(request), 0) < 0) {
        fprintf(stderr, BOLD RED "ERROR:" RESET RED " Sending request failed.\n");
        close(sockfd);
        return 1;
    }

    // Read the response from the server
    int total_bytes = 0;
    while ((total_bytes = recv(sockfd, response, sizeof(response) - 1, 0)) > 0) {
        response[total_bytes] = '\0'; // Null-terminate the response

        // Check if the headers are still present
        if (!header_found) {
            char *content = remove_headers(response, &offset); // Find and skip the headers
            if (offset > 0) {
                header_found = 1;
            }
            extract_pre_content(content, OUTPUT); // Extract and print content between <PRE> and </PRE>
        } else {
            // Headers already skipped, print the remaining response
            extract_pre_content(response, OUTPUT);
        }
    }

    // Close the socket
    printf("\n\n");
    close(sockfd);
    return 0;
}
