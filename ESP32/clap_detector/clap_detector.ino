#include "pffft.h"
#include "pector_config.h"
#include "pector_util.h"
#include "pector_stream_processor.h"

// Include I2S driver
#include <driver/i2s.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>

// a gain factor to amplify the sound: samples come out of the
// microphone rather quietly.
uint16_t gain_factor = 40;

const char * ssid = "your_ssid";
const char * password = "your_password";

// Connections to INMP441 I2S microphone
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32

// Use I2S Processor 0
#define I2S_PORT I2S_NUM_0

// Define input buffer length
#define bufferLen 64
uint8_t sBuffer[bufferLen*2];

size_t audio_block_index;
float * audio_block;
size_t audio_sample_index;

Pector_Config *config;
Pector_Stream_Processor* processor;


void http_get(String url){
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(url.c_str());

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

bool ventilation_state = false;
long ventilation_state_switch_time = 0;

//20 minutes
long ventilation_auto_off_time_delta = 25l * 60l * 1000l;

void auto_shutdown_ventilation(){
  if(ventilation_state){
    long current_time = millis();
    long delta = current_time - ventilation_state_switch_time;
    if(delta > ventilation_auto_off_time_delta){
      ventilation_state_switch_time = current_time;
      ventilation_state = false;
      http_get("http://192.168.88.111/1");
      Serial.println("Auto shutdown of ventilation");  
    }
  }
}

//callback called on match
void pector_onset_callback(float onset_time_0,float onset_time_1,float onset_time_2,float onset_time_3, float loudness, float onset_value) {
  
  //calculate if it a double triple or quad onset
	float max_time_between_onsets = 0.45;//seconds
	float max_diff_between_onsets = 0.1;//seconds

	float delta_0_1 = onset_time_0 - onset_time_1;
	float delta_1_2 = onset_time_1 - onset_time_2;
	float delta_2_3 = onset_time_2 - onset_time_3;

	bool double_onset = delta_0_1 < max_time_between_onsets;
	printf("%.3f\n",delta_0_1);
	bool triple_onset = delta_0_1 < max_time_between_onsets && 
		delta_1_2 < max_time_between_onsets && 
		fabs(delta_0_1 - delta_1_2) < max_diff_between_onsets;
	
	bool quad_onset = delta_0_1 < max_time_between_onsets && 
		delta_1_2 < max_time_between_onsets && 
		delta_2_3 < max_time_between_onsets && 
		fabs(delta_0_1 - delta_1_2) < max_diff_between_onsets &&
		fabs(delta_0_1 - delta_2_3) < max_diff_between_onsets &&
		fabs(delta_1_2 - delta_2_3) < max_diff_between_onsets;
  

  if(double_onset){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    if(ventilation_state){
      http_get("http://192.168.88.111/1");
      ventilation_state = false;
    }else{
      http_get("http://192.168.88.111/3");
      ventilation_state = true;
    }    
    ventilation_state_switch_time = millis();
    Serial.printf("%.3f %.3f %.3f %.3f| %.3f %.3f %d \n", onset_time_0, onset_time_1, onset_time_2, onset_time_3, loudness, onset_value,ventilation_state);    
  }

}

//Setup the pector structs
void setup_pector(){
  config = pector_config_ESP32();

  processor = pector_stream_processor_new(config,NULL,&pector_onset_callback);

  audio_block = (float *) calloc(sizeof(float),config->audio_block_size);

  size_t step_size = config->audio_step_size;
  size_t block_size = config->audio_block_size;
  size_t overlap_size = block_size - step_size;
  //initialize the first sample to be read at overlap_size =
  audio_sample_index = overlap_size;
}

void connect_wifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("WiFi Failed");
      while(1) {
          delay(500);
      }
  }
  // you are now connected:
  Serial.print("You are connected to the network ");
  Serial.println(ssid);
}

void i2s_install() {
  // Set up I2S Processor configuration
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  // Set I2S pin configuration
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  connect_wifi();

  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);

  Serial.println("[pector] Free memory before init: " + String(esp_get_free_heap_size()) + " bytes");
   //print everything to stdout (serial);
  stderr = stdout;
  setup_pector();
  Serial.println("[pector] Free after init: " + String(esp_get_free_heap_size()) + " bytes");

  for(size_t i = 0 ; i < 10 ; i++){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(100);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(120);   
  }
}

void loop(){
  
  auto_shutdown_ventilation();

  size_t step_size = config->audio_step_size;
  size_t block_size = config->audio_block_size;
  size_t overlap_size = block_size - step_size;

  // Get I2S data and place in data buffer
  size_t bytesIn = 0;
  esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);
  
  if (result == ESP_OK && bytesIn > 0){

    int16_t * sample_buffer = (int16_t *) sBuffer;

    // Read I2S data buffer
    // I2S reads stereo data and one channel is zero the other contains
    // data, both have 2 bytes per sample
    int16_t samples_read = bytesIn / 2;
    
    for(size_t i = 0 ; i < samples_read ; i++){
      sample_buffer[i] = gain_factor * sample_buffer[i];
      //Max for signed int16_t is 2^15
      audio_block[audio_sample_index] = sample_buffer[i] / 32768.f;
      audio_sample_index++;

      if(audio_sample_index==block_size){
        pector_stream_process_audio_block(processor,audio_block);
        //printf("[OLAF] processed audio block index: %zu\n",audio_block_index);
        audio_block_index++;

        //shift samples to make room for new samples
        for(size_t j = 0; j < overlap_size ; j++){
          audio_block[j] = audio_block[j+step_size];
        }
        audio_sample_index=overlap_size;
      }
    }
  }
}




