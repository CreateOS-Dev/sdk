#include <createos/uikit.h>

int main(void)
{
    createos_ui_t ui;
    int status = 0;

    if (createos_ui_init(&ui) < 0)
        return 1;
    if (createos_ui_clear() < 0 ||
        createos_ui_write_centered(&ui, ui.rows / 2,
                                  "Hello from CreateOS!", 0x0F) < 0 ||
        createos_ui_write_centered(&ui, ui.rows / 2 + 2,
                                  "Press any key to return.", 0x07) < 0)
        status = 1;
    createos_ui_read_key();
    if (createos_ui_shutdown() < 0)
        status = 1;
    return status;
}
