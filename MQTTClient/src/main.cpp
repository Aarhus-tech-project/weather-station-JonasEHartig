// mosquitto_sub -h 192.168.106.11 -u weatheruser -P Datait2025! -t "vejrstation/data" -v

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
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
static const int   MQTT_PORT = 1883;
static const char* TOPIC    = "vejrstation/data";

static const char* DB_HOST  = "192.168.106.11";
static const int   PORT     = 3306;
static const char* DB_USER  = "mqtt_user";
static const char* DB_PASS  = "Datait2025!";
static const char* DB_NAME  = "vejrdata";
static const char* API_URL  = "http://example.com/data";

MYSQL* db_conn = nullptr;

void db_insert(double temp, double hum, double tryk) {
    if (!db_conn) {
        std::cerr << "MySQL error: db_conn is null\n";
        return;
    }
    char query[256];
    // Use %.2f is fine here; prepared statements are safer long-term.
    snprintf(query, sizeof(query),
             "INSERT INTO data (temperatur, luftfugtighed, tryk) VALUES (%.2f, %.2f, %.2f)",
             temp, hum, tryk);
    if (mysql_query(db_conn, query)) {
        std::cerr << "MySQL error: " << mysql_error(db_conn) << "\n";
    }
}

void on_message(struct mosquitto*, void*, const struct mosquitto_message* msg) {
    std::string payload(static_cast<char*>(msg->payload), msg->payloadlen);
    std::cout << "[MQTT] " << msg->topic << " => " << payload << "\n";
    try {
        auto j = json::parse(payload);
        double temp = j.value("temp", 0.0);
        double tryk = j.value("pressure", 0.0);
        double hum  = j.contains("humidity") ? j.value("humidity", 0.0) : j.value("humidity", 0.0);

        db_insert(temp, hum, tryk);
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void on_log(mosquitto*, void*, int, const char* s)
{
    std::cerr << "[MQTT-LOG] " << s << "\n";
};

int main() {
    std::cout << "build correkt" << std::endl;

    MYSQL* conn = mysql_init(nullptr);
    if (conn == nullptr) {
        std::cerr << "mysql_init() failed\n";
    } else {
        std::cout << "mysql_init() succeeded, pointer: " << conn << "\n";
    }

    unsigned int ssl_mode = SSL_MODE_DISABLED;
    std::cout << ssl_mode << "\n";

    mysql_options(conn, MYSQL_OPT_SSL_MODE, &ssl_mode);
    std::cout << ssl_mode << std::endl;

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, PORT, nullptr, 0)) {
        std::cerr << "MySQL connect error: " << mysql_error(conn) << std::endl;
        return 1;
    }
    else
    {
        std::cerr << "MYSQL connection succeeded\n" << std::endl;
        db_conn = conn;
    }

    std::cout << "guys, we are past" << std::endl;

    mosquitto_lib_init();

    mosquitto* client = mosquitto_new(nullptr, true, nullptr);
    if (!client) {
        std::cerr << "Failed to create Mosquitto client\n";
        return 1;
    } else {
        std::cerr << "Mosquitto connection succeeded\n";
    }

    mosquitto_log_callback_set(client, on_log);

    mosquitto_message_callback_set(client, on_message);

    int rc = mosquitto_username_pw_set(client, "weatheruser", "Datait2025!");
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "username_pw_set failed: " << mosquitto_strerror(rc) << "\n";
        return 1;
    }

    rc = mosquitto_connect(client, BROKER, MQTT_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT connect failed: " << mosquitto_strerror(rc) << "\n";
        return 1;
    }

    int mid = 0;
    
    rc = mosquitto_subscribe(client, &mid, TOPIC, 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "Subscribe failed: " << mosquitto_strerror(rc) << "\n";
        return 1;
    }
    std::cout << "SUBSCRIBE sent for " << TOPIC << " (mid=" << mid << ")\n";

    mosquitto_loop_forever(client, -1, 1);

}
