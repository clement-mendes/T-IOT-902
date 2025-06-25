#ifndef API_H
#define API_H

/**
 * @brief Send JSON data to the remote API via HTTP POST.
 *
 * @param json_data The JSON string to send.
 */
void send_data_to_api(const char *json_data);

#endif // API_H
