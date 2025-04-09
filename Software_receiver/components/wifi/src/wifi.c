#include "wifi.h"

static EventGroupHandle_t s_wifi_event_group;// utilisé pour la synchronisation des événements ( connecté ou non)
static const int WIFI_CONNECTED_BIT = BIT0; //bit de l’événement pour indiquer une connexion réussie
static int s_retry_num = 0;
static const char *TAG = "wifi_station"; //utilisé pour les logs ESP_LOG

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) { //le Wi-Fi démarre, on tente de se connecter
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying to connect to the AP...");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT); //active le bit d’événement WIFI_CONNECTED_BIT pour notifier les autres tâches que la connexion a échoué
        }
        ESP_LOGI(TAG, "Failed to connect to SSID: %s", WIFI_SSID);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) { //DHCP du routeur attribue une adresse IP
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate(); //permet de synchroniser plusieurs tâches 

    ESP_ERROR_CHECK(esp_netif_init()); //Initialise le composant esp-netif, nécessaire pour la gestion réseau
    ESP_ERROR_CHECK(esp_event_loop_create_default());//Crée une boucle d'événements par défaut pour gérer les événements système
    esp_netif_create_default_wifi_sta(); //Crée une interface réseau configurée pour mode station coté client

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();//Prépare la configuration initiale par défaut pour le Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // applique la configuration

    esp_event_handler_instance_t instance_any_id; //gestionnaires d’événements
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id)); //Enregistre event_handler() pour TOUS les événements Wi-Fi (comme START, DISCONNECTED...)
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));//Enregistre event_handler() pour l'événement IP quand le Wi-Fi obtient une adresse IP

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,//contient le nom du réseau  et le mot de passe
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, //threshold.authmode force l'ESP32 à se connecter uniquement à des réseaux au minimum WPA2 (plus sécurisé).

        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));//Configure le mode Station
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));//Applique la configuration (SSID + mot de passe) à l'interface station
    ESP_ERROR_CHECK(esp_wifi_start());//Lance le Wi-Fi

    ESP_LOGI(TAG, "WiFi initialized in station mode.");//Affiche dans les logs
}