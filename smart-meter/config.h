#define device "device"

// WiFi credentials.
#define wifi_hostname device
#define wifi_ssid "SSID"
#define wifi_password "PASSWORD"

// Sensor configuration
#define optical 13
#define ppkWh 3200.0 // pulses per kWh from smart meter
#define resolution 50.0 // required resolution of sampling, in W

// Reporting
#define mqtt_server "0.0.0.0"
#define power_topic device "/power"

// Variable calculations
#define Jpp 3600.0 / (ppkWh/1000.0) // Joules per pulse
#define sampling_window 1000.0 * (Jpp / resolution) // ms
