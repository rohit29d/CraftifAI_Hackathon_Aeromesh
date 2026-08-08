#ifndef APP_H
#define APP_H

/* Vendor-agnostic application entry point. Called once from main/entry.c. */
#ifdef __cplusplus
extern "C" {
#endif

void app_start(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
