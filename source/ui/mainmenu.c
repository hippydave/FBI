#include <stdio.h>

#include <3ds.h>

#include "list.h"
#include "mainmenu.h"
#include "ui.h"
#include "section/section.h"
#include "../core/linkedlist.h"
#include "../core/screen.h"

static list_item download_unencrypted = {"1 Download decTitleKeys.bin for unencrypted titles", COLOR_TEXT, download_dectitlekeys};
static list_item dummy2 = {"2 Encrypt it to encTitleKeys.bin with Decrypt9", COLOR_DS_TITLE, NULL};
static list_item make_input = {"3 Make Input.txt for selected title", COLOR_TEXT, make_input_txt};
static list_item dummy4 = {"4 Install using CIAngel", COLOR_DS_TITLE, NULL};
static list_item dummy5 = {"5 ????", COLOR_DS_TITLE, NULL};
static list_item dummy6 = {"6 Profit!", COLOR_DS_TITLE, NULL};

static void mainmenu_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item* selected) {
    u32 logoWidth;
    u32 logoHeight;
    screen_get_texture_size(&logoWidth, &logoHeight, TEXTURE_LOGO);

    float logoX = x1 + (x2 - x1 - logoWidth) / 2;
    float logoY = y1 + (y2 - y1 - logoHeight) / 2;
    screen_draw_texture(TEXTURE_LOGO, logoX, logoY, logoWidth, logoHeight);
}

static void mainmenu_update(ui_view* view, void* data, linked_list* items, list_item* selected, bool selectedTouched) {
    if(hidKeysDown() & KEY_START) {
        ui_pop();
        list_destroy(view);

        return;
    }

    if(selected != NULL && (selectedTouched || hidKeysDown() & KEY_A) && selected->data != NULL) {
        ((void(*)()) selected->data)();
        return;
    }

    if(linked_list_size(items) == 0) {
        linked_list_add(items, &download_unencrypted);
        linked_list_add(items, &dummy2);
        linked_list_add(items, &make_input);
        linked_list_add(items, &dummy4);
        linked_list_add(items, &dummy5);
        linked_list_add(items, &dummy6);
    }
}

void mainmenu_open() {
    list_display("Main Menu", "A: Select, START: Exit", NULL, mainmenu_update, mainmenu_draw_top);
}
