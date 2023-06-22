#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#define BUFFER_SIZE 1024
#define DTLS_PORT 9443

void handle_error(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
    ERR_print_errors_fp(stderr);
    exit(1);
}

void init_openssl() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX* create_context() {
    const SSL_METHOD* method = DTLS_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        handle_error("Failed to create SSL context");
    }
    return ctx;
}

void configure_context(SSL_CTX* ctx) {
    SSL_CTX_set_ecdh_auto(ctx, 1);
    SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
}

void run_server() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        handle_error("Failed to create socket");
    }
    std::cout << "start dtls server ..."  << std::endl;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(DTLS_PORT);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        handle_error("Failed to bind socket");
    }

    SSL_CTX* ctx = create_context();
    configure_context(ctx);

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        handle_error("Failed to create SSL");
    }

    BIO* bio = BIO_new_dgram(sockfd, BIO_NOCLOSE);
    if (!bio) {
        handle_error("Failed to create BIO");
    }

    SSL_set_bio(ssl, bio, bio);

    if (SSL_accept(ssl) <= 0) {
        handle_error("Failed to perform SSL_accept");
    }

    char buffer[BUFFER_SIZE];
    int bytes_read = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        handle_error("Failed to read from SSL connection");
    }
    buffer[bytes_read] = '\0';
    std::cout << "Received message from client: " << buffer << std::endl;

    int bytes_written = SSL_write(ssl, buffer, bytes_read);
    if (bytes_written <= 0) {
        handle_error("Failed to write to SSL connection");
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
}

void run_client() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        handle_error("Failed to create socket");
    }
    std::cout << "start dtls client ..."  << std::endl;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DTLS_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr)) <= 0) {
        handle_error("Invalid address");
    }

    SSL_CTX* ctx = create_context();
    configure_context(ctx);

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        handle_error("Failed to create SSL");
    }

    BIO* bio = BIO_new_dgram(sockfd, BIO_NOCLOSE);
    if (!bio) {
        handle_error("Failed to create BIO");
    }

    SSL_set_bio(ssl, bio, bio);

    if (SSL_connect(ssl) <= 0) {
        handle_error("Failed to perform SSL_connect");
    }

    if (SSL_do_handshake(ssl) <= 0) {
        handle_error("Failed to perform SSL handshake");
    }

    const char* message = "Hello, server!";
    int message_len = strlen(message);

    int bytes_written = SSL_write(ssl, message, message_len);
    if (bytes_written <= 0) {
        handle_error("Failed to write to SSL connection");
    }

    char buffer[BUFFER_SIZE];
    int bytes_read = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        handle_error("Failed to read from SSL connection");
    }
    buffer[bytes_read] = '\0';
    std::cout << "Received message from server: " << buffer << std::endl;

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
}

int main() {
    init_openssl();

    // Run the server in a separate process or thread
    pid_t pid = fork();
    if (pid == 0) {
        run_server();
        return 0;
    }

    // Wait for a brief moment before starting the client
    sleep(1);

    // Run the client
    run_client();

    return 0;
}
