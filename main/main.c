#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "max31855.h"

static const char *TAG = "max31855_example";


void receiver_task(void *pvParameter) {
    ESP_LOGI(TAG, "Created: thermocouple_task");
    QueueHandle_t thermocouple_queue = (QueueHandle_t)pvParameter;
	
	  max31855_cfg_t data;
    if(thermocouple_queue == NULL){
	    ESP_LOGI(TAG, "Not Ready: thermocouple_queue");
        return;
    }
    while(1){
        xQueueReceive(thermocouple_queue, &data,(TickType_t )(2000/portTICK_PERIOD_MS)); 
        ESP_LOGI(TAG, "Struct Received on Queue: Cold Junction: %0.2f°C Temperature: %0.2f°C", data.coldjunction_c, data.thermocouple_c);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}

void thermocouple_task(void *pvParameter) {
  ESP_LOGI(TAG, "Created: receiver_task");
  QueueHandle_t thermocouple_queue = (QueueHandle_t)pvParameter;

  if(thermocouple_queue == NULL){
    ESP_LOGI(TAG, "Not Ready: thermocouple_queue");
      return;
  }
  // init
  max31855_cfg_t max31855 = max31855_init();

  // loop
  while(1){
      max31855_get_temperature(&max31855);
      xQueueSend(thermocouple_queue, &max31855,(TickType_t )0);
      vTaskDelay(200/portTICK_PERIOD_MS);
  }
}

void app_main() {

    // Queue: Thermocouple Data -> Receiver
     QueueHandle_t thermocouple_queue = xQueueCreate(5, sizeof(struct max31856_cfg_t));
    if(thermocouple_queue != NULL){
	    ESP_LOGI(TAG, "Created: thermocouple_queue");
      vTaskDelay(1000/portTICK_PERIOD_MS);
      xTaskCreate(&thermocouple_task, "thermocouple_task", 2048, (void *)thermocouple_queue, 5, NULL);
      xTaskCreate(&receiver_task, "receiver_task", 2048, (void *)thermocouple_queue, 5, NULL);
    } else{
	    ESP_LOGI(TAG, "Failed to Create: thermocouple_queue");
    }  
    
}
