#define device "smart-meter"

// WiFi credentials.
#define wifi_hostname device
#define wifi_ssid "SSID"
#define wifi_password "PASSPHRASE"

// Sensor configuration
#define optical D1
#define ppwh 1/3.2 //3200 pulses/kWh = 1/3.2 pulse per Wh
#define ct A0
#define samples 1000 // heuristically no improvement above 1000

// Reporting
#define mqtt_server "0.0.0.0"
#define power_topic device "/power/optical"
#define power_ct_topic device "/power/ct"
