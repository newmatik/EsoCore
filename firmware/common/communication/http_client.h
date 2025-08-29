/**
 * @file http_client.h
 * @brief HTTP Client for EsoCore API Communication
 *
 * This file provides HTTP client functionality for the EsoCore Edge device
 * to communicate with the Django REST API backend, including sensor data
 * upload, configuration retrieval, and firmware updates.
 *
 * Features:
 * - HTTP/HTTPS support with TLS
 * - RESTful API communication
 * - JSON data serialization/deserialization
 * - Authentication (API key, JWT)
 * - Error handling and retry logic
 * - Connection pooling and keep-alive
 * - Firmware update downloads
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HTTP_CLIENT_H
#define ESOCORE_HTTP_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * HTTP Client Configuration
 * ============================================================================ */

#define HTTP_MAX_URL_LENGTH        256
#define HTTP_MAX_HEADER_COUNT      16
#define HTTP_MAX_HEADER_LENGTH     128
#define HTTP_MAX_BODY_SIZE         2048
#define HTTP_DEFAULT_TIMEOUT_MS    30000
#define HTTP_MAX_REDIRECTS         5

typedef enum {
    HTTP_METHOD_GET      = 0,    /* HTTP GET method */
    HTTP_METHOD_POST     = 1,    /* HTTP POST method */
    HTTP_METHOD_PUT      = 2,    /* HTTP PUT method */
    HTTP_METHOD_DELETE   = 3,    /* HTTP DELETE method */
    HTTP_METHOD_PATCH    = 4,    /* HTTP PATCH method */
} http_method_t;

typedef enum {
    HTTP_STATUS_CONTINUE           = 100,
    HTTP_STATUS_OK                 = 200,
    HTTP_STATUS_CREATED            = 201,
    HTTP_STATUS_NO_CONTENT         = 204,
    HTTP_STATUS_BAD_REQUEST        = 400,
    HTTP_STATUS_UNAUTHORIZED       = 401,
    HTTP_STATUS_FORBIDDEN          = 403,
    HTTP_STATUS_NOT_FOUND          = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    HTTP_STATUS_CONFLICT           = 409,
    HTTP_STATUS_INTERNAL_ERROR     = 500,
    HTTP_STATUS_BAD_GATEWAY        = 502,
    HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
} http_status_code_t;

typedef struct {
    char server_url[HTTP_MAX_URL_LENGTH];       /* Base server URL */
    uint16_t server_port;                        /* Server port (default 80/443) */
    bool use_https;                              /* Enable HTTPS/TLS */
    char api_key[128];                           /* API key for authentication */
    uint32_t connection_timeout_ms;              /* Connection timeout */
    uint32_t response_timeout_ms;                /* Response timeout */
    uint8_t max_retries;                         /* Maximum retry attempts */
    uint32_t retry_delay_ms;                     /* Delay between retries */
    bool keep_alive;                             /* Enable HTTP keep-alive */
    uint16_t buffer_size;                        /* HTTP buffer size */
} http_client_config_t;

/* ============================================================================
 * HTTP Request/Response Structures
 * ============================================================================ */

typedef struct {
    char name[HTTP_MAX_HEADER_LENGTH];           /* Header name */
    char value[HTTP_MAX_HEADER_LENGTH];          /* Header value */
} http_header_t;

typedef struct {
    http_method_t method;                        /* HTTP method */
    char url[HTTP_MAX_URL_LENGTH];               /* Request URL */
    http_header_t headers[HTTP_MAX_HEADER_COUNT]; /* Request headers */
    uint8_t num_headers;                         /* Number of headers */
    uint8_t *body;                               /* Request body */
    uint16_t body_length;                        /* Request body length */
    char content_type[64];                       /* Content-Type header */
} http_request_t;

typedef struct {
    http_status_code_t status_code;              /* HTTP status code */
    char status_message[128];                    /* Status message */
    http_header_t headers[HTTP_MAX_HEADER_COUNT]; /* Response headers */
    uint8_t num_headers;                         /* Number of headers */
    uint8_t *body;                               /* Response body */
    uint16_t body_length;                        /* Response body length */
    uint32_t response_time_ms;                   /* Response time */
} http_response_t;

/* ============================================================================
 * EsoCore API Endpoints
 * ============================================================================ */

#define API_ENDPOINT_AUTH_HANDSHAKE    "/api/iot/v1/auth/handshake"
#define API_ENDPOINT_TELEMETRY_BATCH   "/api/iot/v1/telemetry/batch"
#define API_ENDPOINT_CONFIG            "/api/iot/v1/config"
#define API_ENDPOINT_OTA_CHECK         "/api/iot/v1/ota/check"
#define API_ENDPOINT_OTA_REPORT        "/api/iot/v1/ota/report"
#define API_ENDPOINT_HEARTBEAT         "/api/iot/v1/heartbeat"

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize HTTP client
 *
 * @param config Pointer to HTTP client configuration
 * @return true if initialization successful, false otherwise
 */
bool http_client_init(const http_client_config_t *config);

/**
 * @brief Deinitialize HTTP client
 *
 * @return true if deinitialization successful, false otherwise
 */
bool http_client_deinit(void);

/**
 * @brief Send HTTP request
 *
 * @param request Pointer to HTTP request structure
 * @param response Pointer to HTTP response structure to fill
 * @return true if request successful, false otherwise
 */
bool http_client_send_request(const http_request_t *request, http_response_t *response);

/**
 * @brief Send GET request
 *
 * @param url Request URL (relative to base URL)
 * @param response Pointer to HTTP response structure to fill
 * @return true if request successful, false otherwise
 */
bool http_client_get(const char *url, http_response_t *response);

/**
 * @brief Send POST request
 *
 * @param url Request URL (relative to base URL)
 * @param body Request body data
 * @param body_length Request body length
 * @param content_type Content-Type header
 * @param response Pointer to HTTP response structure to fill
 * @return true if request successful, false otherwise
 */
bool http_client_post(const char *url, const uint8_t *body, uint16_t body_length,
                     const char *content_type, http_response_t *response);

/**
 * @brief Send PUT request
 *
 * @param url Request URL (relative to base URL)
 * @param body Request body data
 * @param body_length Request body length
 * @param content_type Content-Type header
 * @param response Pointer to HTTP response structure to fill
 * @return true if request successful, false otherwise
 */
bool http_client_put(const char *url, const uint8_t *body, uint16_t body_length,
                    const char *content_type, http_response_t *response);

/**
 * @brief Add custom header to request
 *
 * @param request Pointer to HTTP request structure
 * @param name Header name
 * @param value Header value
 * @return true if header added successfully, false otherwise
 */
bool http_client_add_header(http_request_t *request, const char *name, const char *value);

/**
 * @brief Set authorization header (API key)
 *
 * @param request Pointer to HTTP request structure
 * @return true if authorization set successfully, false otherwise
 */
bool http_client_set_authorization(http_request_t *request);

/**
 * @brief Update HTTP client configuration
 *
 * @param config Pointer to new configuration
 * @return true if configuration updated successfully, false otherwise
 */
bool http_client_update_config(const http_client_config_t *config);

/**
 * @brief Get current HTTP client configuration
 *
 * @param config Pointer to configuration structure to fill
 * @return true if configuration retrieved successfully, false otherwise
 */
bool http_client_get_config(http_client_config_t *config);

/* ============================================================================
 * EsoCore API Functions
 * ============================================================================ */

/**
 * @brief Send authentication handshake to server
 *
 * @param device_id Device identifier
 * @param response Pointer to HTTP response structure to fill
 * @return true if handshake successful, false otherwise
 */
bool esocore_api_auth_handshake(const char *device_id, http_response_t *response);

/**
 * @brief Send telemetry data batch to server
 *
 * @param telemetry_data Pointer to telemetry data
 * @param data_length Length of telemetry data
 * @param response Pointer to HTTP response structure to fill
 * @return true if upload successful, false otherwise
 */
bool esocore_api_send_telemetry(const uint8_t *telemetry_data, uint16_t data_length,
                               http_response_t *response);

/**
 * @brief Retrieve device configuration from server
 *
 * @param config_data Pointer to buffer for configuration data
 * @param max_length Maximum buffer length
 * @param response Pointer to HTTP response structure to fill
 * @return true if configuration retrieved successfully, false otherwise
 */
bool esocore_api_get_config(uint8_t *config_data, uint16_t max_length,
                           http_response_t *response);

/**
 * @brief Check for firmware updates
 *
 * @param current_version Current firmware version
 * @param response Pointer to HTTP response structure to fill
 * @return true if update check successful, false otherwise
 */
bool esocore_api_check_ota(const char *current_version, http_response_t *response);

/**
 * @brief Report OTA update status
 *
 * @param status Update status (success/failure)
 * @param error_message Error message (if applicable)
 * @param response Pointer to HTTP response structure to fill
 * @return true if report sent successfully, false otherwise
 */
bool esocore_api_report_ota(const char *status, const char *error_message,
                           http_response_t *response);

/**
 * @brief Send heartbeat to server
 *
 * @param status Device status information
 * @param response Pointer to HTTP response structure to fill
 * @return true if heartbeat sent successfully, false otherwise
 */
bool esocore_api_send_heartbeat(const char *status, http_response_t *response);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Create JSON payload for sensor data
 *
 * @param sensor_data Pointer to sensor data
 * @param data_length Length of sensor data
 * @param json_buffer Pointer to JSON buffer
 * @param max_length Maximum JSON buffer length
 * @return Length of JSON payload, 0 on error
 */
uint16_t http_create_sensor_json(const uint8_t *sensor_data, uint16_t data_length,
                                char *json_buffer, uint16_t max_length);

/**
 * @brief Parse JSON response from server
 *
 * @param json_data Pointer to JSON data
 * @param data_length Length of JSON data
 * @param key Key to search for
 * @param value_buffer Pointer to buffer for value
 * @param max_length Maximum value buffer length
 * @return true if key found and parsed successfully, false otherwise
 */
bool http_parse_json_value(const char *json_data, uint16_t data_length,
                          const char *key, char *value_buffer, uint16_t max_length);

/**
 * @brief URL encode a string
 *
 * @param input Input string
 * @param output Pointer to output buffer
 * @param max_length Maximum output buffer length
 * @return Length of encoded string, 0 on error
 */
uint16_t http_url_encode(const char *input, char *output, uint16_t max_length);

/**
 * @brief Get HTTP status code description
 *
 * @param status_code HTTP status code
 * @return Status code description string
 */
const char *http_get_status_description(http_status_code_t status_code);

/**
 * @brief Check if HTTP status indicates success
 *
 * @param status_code HTTP status code
 * @return true if status indicates success, false otherwise
 */
bool http_is_success_status(http_status_code_t status_code);

/**
 * @brief Get HTTP client statistics
 *
 * @param requests_sent Pointer to store number of requests sent
 * @param responses_received Pointer to store number of responses received
 * @param errors_count Pointer to store number of errors
 * @return true if statistics retrieved successfully, false otherwise
 */
bool http_client_get_statistics(uint32_t *requests_sent, uint32_t *responses_received,
                               uint32_t *errors_count);

/**
 * @brief Clear HTTP client statistics
 *
 * @return true if statistics cleared successfully, false otherwise
 */
bool http_client_clear_statistics(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HTTP_CLIENT_H */
