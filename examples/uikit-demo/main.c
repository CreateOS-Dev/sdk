#include <createos/uikit.h>

int main(void)
{
    createos_ui_t ui;
    int key;
    int status = 0;

    if (createos_ui_init(&ui) < 0)
        return 1;
    if (createos_ui_clear() < 0 ||
        createos_ui_write_centered(&ui, 2, "CreateOS UI Kit demo", 0x0F) < 0 ||
        createos_ui_fill_row(&ui, 4, '-', 0x09) < 0 ||
        createos_ui_write_at(&ui, 2, 6,
                             "The UI Kit writes colored text by cell.", 0x07) < 0 ||
        createos_ui_write_centered(&ui, ui.rows - 2,
                                   "Press Q to quit.", 0x0A) < 0)
        status = 1;
    do {
        key = createos_ui_read_key();
        if (key < 0) {
            status = 1;
            break;
        }
    } while (key != 'q' && key != 'Q');
    if (createos_ui_shutdown() < 0)
        status = 1;
    return status;
}
