#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <mariadb/errmsg.h>
extern "C" {
#include <mosquitto.h>
}
#include <mariadb/mysql.h>
#define MYSQL_OPT_SSL_MODE ((mysql_option)129)
#define SSL_MODE_DISABLED 0

using json = nlohmann::json;

// Config
static const char* BROKER   = "192.168.106.11"; // change to broker IP
static const int   PORT     = 1883;
static const char* TOPIC    = "sensors/#";

static const char* DB_HOST  = "192.168.106.11";
static const char* DB_USER  = "mqtt_user";
static const char* DB_PASS  = "Datait2025!";
static const char* DB_NAME  = "vejrdata";
static const char* API_URL  = "http://example.com/data";

MYSQL* db_conn = nullptr;

// Insert into MySQL
void db_insert(double temp, double hum) {
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO sensor_data (temperature, humidity) VALUES (%.2f, %.2f)",
             temp, hum);
    if (mysql_query(db_conn, query)) {
        std::cerr << "MySQL error: " << mysql_error(db_conn) << "\n";
    }
}

// Send HTTP POST
void http_post_json(const json& j) {
    CURL* curl = curl_easy_init();
    if (!curl) return;

    std::string payload = j.dump();
    struct curl_slist* hdr = nullptr;
    hdr = curl_slist_append(hdr, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, API_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_perform(curl);

    curl_slist_free_all(hdr);
    curl_easy_cleanup(curl);
}

// MQTT message callback
void on_message(struct mosquitto*, void*, const struct mosquitto_message* msg) {
    std::string payload(static_cast<char*>(msg->payload), msg->payloadlen);
    std::cout << "[MQTT] " << msg->topic << " => " << payload << "\n";

    try {
        auto j = json::parse(payload);
        double temp = j.value("temp", 0.0);
        double hum  = j.value("hum",  0.0);

        db_insert(temp, hum);
        http_post_json(j);

    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "build correkt" << std::endl;

    MYSQL* conn = mysql_init(nullptr);

    // Then use 'db_conn' below as well:
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(conn, MYSQL_OPT_SSL_MODE, &ssl_mode);

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, nullptr, 0)) {
        std::cerr << "MySQL connect error: " << mysql_error(conn) << std::endl;
        return 1;
    }
    else
    {
        std::cerr << "MYSQL connection succeeded\n";
    }

    // Init libs
    mosquitto_lib_init();
    curl_global_init(CURL_GLOBAL_DEFAULT);

    mosquitto* client = mosquitto_new(nullptr, true, nullptr);
    if (!client) {
        std::cerr << "Failed to create Mosquitto client\n";
        return 1;
    }

    mosquitto_message_callback_set(client, on_message);

    if (mosquitto_connect(client, BROKER, PORT, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT connection failed\n";
        return 1;
    } else {
        std::cerr << "MQTT connection succeeded\n";
    }

    mosquitto_subscribe(client, nullptr, TOPIC, 0);
    std::cout << "Subscribed to " << TOPIC << "\n";

    mosquitto_loop_forever(client, -1, 1);

    mosquitto_destroy(client);
    curl_global_cleanup();
    mosquitto_lib_cleanup();
    mysql_close(db_conn);
    return 0;
}
