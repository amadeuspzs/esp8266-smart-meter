#define device "smart-meter"

// WiFi credentials.
#define wifi_hostname device
#define wifi_ssid "SSID"
#define wifi_password "PASSPHRASE"

// Sensor configuration
#define optical D1
#define pp_kwh 3200 // 3200 pulses/kWh
#define jp_impulse (3600 * 1000/pp_kwh) // Joules per impulse
#define sample_ms 5000 // how often to sample power

// Reporting
#define mqtt_server "0.0.0.0"
#define power_topic device "/power"
