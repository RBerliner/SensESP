#include "frequency.h"

// Frequency

Frequency::Frequency(float k, String config_path)
    : Transform<int, float>(config_path), k{k} {
  load_configuration();
}

void Frequency::enable() { last_update = millis(); }

void Frequency::set_input(int input, uint8_t inputChannel) {
  unsigned long cur_millis = millis();
  unsigned long elapsed_millis = cur_millis - last_update;
  last_update = cur_millis;
  this->emit(k * input / (elapsed_millis / 1000.));
}

void Frequency::get_configuration(JsonObject& root) {
  root["k"] = k;
  root["value"] = output;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "k": { "title": "Multiplier", "type": "number" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

String Frequency::get_config_schema() { return FPSTR(SCHEMA); }

bool Frequency::set_configuration(const JsonObject& config) {
  String expected[] = {"k"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k = config["k"];
  return true;
}
