#ifndef CREATEOS_UIKIT_H
#define CREATEOS_UIKIT_H

#include <stdint.h>

typedef struct {
    uint32_t columns;
    uint32_t rows;
} createos_ui_t;

int createos_ui_init(createos_ui_t* ui);
int createos_ui_refresh_dimensions(createos_ui_t* ui);
int createos_ui_clear(void);
int createos_ui_write_at(const createos_ui_t* ui, uint32_t column,
                         uint32_t row, const char* text, int color);
int createos_ui_write_centered(const createos_ui_t* ui, uint32_t row,
                               const char* text, int color);
int createos_ui_fill_row(const createos_ui_t* ui, uint32_t row,
                         char character, int color);
int createos_ui_read_key(void);
int createos_ui_mouse_position(void);
int createos_ui_shutdown(void);

#endif
