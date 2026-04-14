#ifndef HTTP_TLS_CLIENT_H
#define HTTP_TLS_CLIENT_H

#include <stddef.h>
#include <stdint.h>

/// @brief Request object for tls_client.
typedef struct {
    /// @brief The hostname for the request. Must be null-terminated.
    const char *hostname;
    /// @brief The request; for HTTP this should be a raw request with headers, URI, and body. Must
    /// be null-terminated.
    const char *request;
    /// @brief The public certificate for the hostname.
    const uint8_t *cert;
    /// @brief The length of the certificate for the hostname.
    size_t cert_len;
} TlsClientRequest;

/// @brief Send a HTTPS GET request.
/// @param request Request details.
/// @param buffer Buffer to write the response to. This can be the same buffer used for
/// request.request.
/// @param buffer_len The length of \p buffer.
/// @return 0 if successful.
/// Otherwise, one of the following error codes:
///   * -1: Client timeout.
///   * -2: Failed to resolve DNS & open connection, or the tls_client_err callback was hit.
///   * -3: Failed to allocate client state.
///   * -4: Request was apparently successful, but no bytes were written to the buffer.
int32_t tls_request(TlsClientRequest request, char *buffer, uint16_t buffer_len);

#endif // HTTP_TLS_CLIENT_H