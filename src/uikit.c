#include "createos/uikit.h"
#include "user_syscall.h"

int createos_ui_refresh_dimensions(createos_ui_t* ui)
{
    tty_info_t info;
    char active_tty[16] = {0};

    if (!ui || sys_tty_name(active_tty) < 0)
        return -1;

    ui->columns = 80;
    ui->rows = 25;
    for (int index = 0; index < 5; index++) {
        if (sys_tty_info(index, &info) == 0 &&
            strcmp(info.name, active_tty) == 0 &&
            info.width > 0 && info.height > 0) {
            ui->columns = info.width;
            ui->rows = info.height;
            break;
        }
    }

    return 0;
}

int createos_ui_init(createos_ui_t* ui)
{
    if (createos_ui_refresh_dimensions(ui) < 0)
        return -1;
    return sys_tty_fullscreen_begin();
}

int createos_ui_clear(void)
{
    return sys_clear();
}

int createos_ui_write_at(const createos_ui_t* ui, uint32_t column,
                         uint32_t row, const char* text, int color)
{
    if (!ui || !text || column >= ui->columns || row >= ui->rows)
        return -1;
    if (sys_set_cursor((int)column, (int)row) < 0)
        return -1;
    return sys_puts(text, color);
}

int createos_ui_write_centered(const createos_ui_t* ui, uint32_t row,
                               const char* text, int color)
{
    size_t length;
    uint32_t column;

    if (!ui || !text || row >= ui->rows)
        return -1;
    length = strlen(text);
    if (length > ui->columns)
        length = ui->columns;
    column = (ui->columns - (uint32_t)length) / 2;
    if (sys_set_cursor((int)column, (int)row) < 0)
        return -1;
    for (size_t index = 0; index < length; index++)
        if (sys_putc(text[index], color) < 0)
            return -1;
    return 0;
}

int createos_ui_fill_row(const createos_ui_t* ui, uint32_t row,
                         char character, int color)
{
    if (!ui || row >= ui->rows || ui->columns > 4096)
        return -1;
    if (sys_set_cursor(0, (int)row) < 0)
        return -1;
    for (uint32_t column = 0; column < ui->columns; column++)
        if (sys_putc(character, color) < 0)
            return -1;
    return 0;
}

int createos_ui_read_key(void)
{
    return (unsigned char)sys_getc();
}

int createos_ui_mouse_position(void)
{
    return sys_tty_mouse_position();
}

int createos_ui_shutdown(void)
{
    return sys_tty_fullscreen_end();
}
