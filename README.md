# Nug Doug's Home Humidifier
My little ESP8266-based humidifier.

It has an attached DHT22 temperature/humidity sensor and a 5V relay.

The principal variable is a struct, `settings`, containing the following:
- `humidityLowerBound` (float) - Humidity level at which the relay should be turned ON.
- `humidityUpperBound` (float) - Humidity level at which the relay should be turned OFF.

It has the following HTTP API.

- GET `/` - JSON output of all current data.
- GET `/humidity` - humidity (float)
- GET `/temperature` - temperature (Fahrenheit) (float)
- GET `/settings/humidityLowerBound` - humidity lower bound (float)
- GET `/settings/humidityUpperBound` - humidity upper bound (float)
- POST `/settings/humidityLowerBound` - set humidity lower bound (float), using the `value` parameter
- POST `/settings/humidityUpperBound` - set humidity upper bound (float), using the `value` parameter

It allows OTA updates.
