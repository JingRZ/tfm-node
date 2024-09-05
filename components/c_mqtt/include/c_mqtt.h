#ifndef __C_MQTT
#define __C_MQTT

#include "esp_event.h"
#include "mqtt_client.h"

ESP_EVENT_DECLARE_BASE(C_MQTT_EVENT_BASE);

extern int MQTT_STATUS;
extern int MQTT_SUBSCRIPTION_COUNT;
extern char* NODE_CONTEXT;

enum{
    C_MQTT_EVENT_CONNECTED,
    C_MQTT_EVENT_DISCONNECTED,
    C_MQTT_EVENT_RECEIVED_DATA,
    C_MQTT_EVENT_SUBSCRIBED,
};

enum{
    C_MQTT_OK,
    C_MQTT_ERR
};

struct c_mqtt_data{
    char* topic;
    char* data;
    int data_len;
};

/**
 * @brief Initializes mqtt component and establishes a connection with the broker.
 *
 * @details Before calling this function, ensure the following functions have been executed:
 *   - ESP_ERROR_CHECK(nvs_flash_init()); // Initializes the NVS flash storage
 *   - ESP_ERROR_CHECK(esp_netif_init()); // Initializes the network interface
 *   - ESP_ERROR_CHECK(esp_event_loop_create_default()); // Creates the default event loop
 *   - ESP_ERROR_CHECK(example_connect()); // Establishes the initial connection
 */
void mqtt_start_client();

/**
 * @brief Stop the MQTT client
 *
 * It disconnects from the MQTT broker and cleans up any resources associated with the client.
 */
void mqtt_stop_client();

/**
 * @brief Set MQTT Last Will Testament Message
*/
int mqtt_set_lwt_msg(char *msg, int len);

/**
 * @brief Set MQTT port. 
 * Call this function before init_mqtt()
*/
int mqtt_set_port(int port);

/**
 * @brief Set MQTT username. 
 * Call this function before init_mqtt()
*/
int mqtt_set_username(char *username, int len);

/**
 * @brief If you don't want to use menuconfig to configure the mqtt broker. 
 * Call this function before init_mqtt()
*/
int mqtt_set_broker(char *broker, int len);

/**
 *  @brief  Set MQTT Quality of Service
 *  @param  qos
 *          QoS 0 = At most once (default), 
 *          QoS 1 = At least once, 
 *          QoS 2 = Exactly once
*/   
int mqtt_set_qos(int qos);

/**
 * @brief Subscribe the MQTT client to a specific topic.
 *
 * This function checks if the MQTT client is connected. If it is, it sends a subscription request for the given topic.
 * The Quality of Service (QoS) level for the subscription is determined by the `mqtt_config.qos` value.
 *
 * @param topic The topic to which the MQTT client should subscribe.
 * @return Returns C_MQTT_OK if the subscription request was sent successfully, or C_MQTT_ERR otherwise.
 */
int mqtt_subscribe_to_topic(char* topic);

/**
 * @brief Unsubscribe the MQTT client from a specific topic.
 *
 * @param topic The topic from which the MQTT client should unsubscribe.
 * @return Returns C_MQTT_OK if the unsubscription was successful, or C_MQTT_ERR otherwise.
 */
int mqtt_unsubscribe_to_topic(char* topic);

/**
 * @brief Publish data to a specific MQTT topic.
 *
 * This function checks if the MQTT client is connected. If it is, it publishes the provided data to the given topic.
 * The Quality of Service (QoS) level for the publication is determined by the `mqtt_config.qos` value.
 *
 * @param topic The topic to which the data should be published.
 * @param data The data to be published.
 * @param data_length The length of the data to be published.
 * @return Returns C_MQTT_OK if the publication was successful, or C_MQTT_ERR otherwise.
 */
int mqtt_publish_to_topic(char* topic, uint8_t* data, int data_length);

#endif