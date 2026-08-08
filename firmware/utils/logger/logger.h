#ifndef LOGGER_H
#define LOGGER_H

/* Logging: use the standard ESP-IDF macros — ESP_LOGI / ESP_LOGW / ESP_LOGE / ESP_LOGD(TAG, ...).
   This header simply re-exports esp_log.h so any file that includes "logger.h" can log. */
#include "esp_log.h"

#endif /* LOGGER_H */
