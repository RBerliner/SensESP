#include "app.h"

#include "components/component.h"
#include "devices/device.h"
#include "devices/system_info.h"
#include "net/ota.h"
#include "net/wifi.h"
#include "net/ws_client.h"
#include "system/spiffs_storage.h"


// FIXME: Setting up the system is too verbose and repetitive

SensESPApp::SensESPApp() {

  // initialize filesystem

  setup_spiffs_storage();

  // connect systemhz

  SystemHz* syshz = new SystemHz();
  Passthrough<float>* syshz_value = new Passthrough<float>("sensors.unknown.system_hz");
  syshz->attach([syshz, syshz_value](){ syshz_value->set_input(syshz->get()); });
  devices.push_back(syshz);
  components.push_back(syshz_value);

  // connect freemem

  FreeMem* freemem = new FreeMem();
  Passthrough<float>* freemem_value = new Passthrough<float>("sensors.unknown.freemem");
  freemem->attach([freemem, freemem_value](){ freemem_value->set_input(freemem->get()); });
  devices.push_back(freemem);
  components.push_back(freemem_value);

  // connect uptime (exaggerate the value!)

  Uptime* uptime = new Uptime();
  Linear* uptime_value = new Linear("sensors.unknown.uptime", 1.2, 3600.);
  uptime->attach([uptime, uptime_value](){ uptime_value->set_input(uptime->get()); });
  devices.push_back(uptime);
  components.push_back(uptime_value);

  // connect all components to the Signal K delta output

  sk_delta = new SKDelta("unknown");
  for (auto const& comp : components) {
    comp->attach([comp, this](){ this->sk_delta->append(comp->as_json()); });
  }

  // create the websocket client

  auto ws_connected_cb = [this](){
    this->led_blinker.set_server_connected();
  };
  auto ws_disconnected_cb = [this](){
    this->led_blinker.set_wifi_connected();
  };
  auto ws_delta_cb = [this](){
    this->led_blinker.flip();
  };
  this->ws_client = new WSClient(
    sk_delta, ws_connected_cb, ws_disconnected_cb, ws_delta_cb);

}

void SensESPApp::enable() {
  this->led_blinker.set_wifi_disconnected();
  Serial.println("Enabling subsystems");
  setup_wifi(led_blinker);
  setup_OTA();

  this->ws_client->enable();

  Serial.println("WS client enabled");

  for (auto const& dev : devices) {
    dev->enable();
  }

  Serial.println("All devices enabled");
}