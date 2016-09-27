#include <stdio.h>
#include <string.h>

#include <3ds.h>

#include "section.h"
#include "task/task.h"
#include "../error.h"
#include "../list.h"
#include "../prompt.h"
#include "../ui.h"
#include "../../core/linkedlist.h"
#include "../../core/screen.h"
#include "../../json/json.h"
#include "utils.h"

#define TITLES_MAX 1024

typedef struct {
    char title_id[17];
    char enc_titlekey[33];
    char shortDescription[0x80];
} titlekey;

static char eTKpath1[] = "/files9/encTitleKeys.bin";
static char eTKpath2[] = "/encTitleKeys.bin";
static char itpath[] = "/CIAngel/input.txt";
static char jsonpath[] = "/json_dec.json";
char eTKpath[FILE_PATH_MAX];
unsigned long numTitles, t;
titlekey tkeys[TITLES_MAX];
list_item litem[TITLES_MAX];

static void titlemenu_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item* selected) {
    u32 logoWidth;
    u32 logoHeight;
    screen_get_texture_size(&logoWidth, &logoHeight, TEXTURE_LOGO);
    
    float logoX = x1 + (x2 - x1 - logoWidth) / 2;
    float logoY = y1 + (y2 - y1 - logoHeight) / 2;
    screen_draw_texture(TEXTURE_LOGO, logoX, logoY, logoWidth, logoHeight);
}

static void titlemenu_update(ui_view* view, void* data, linked_list* items, list_item* selected, bool selectedTouched) {
    if(hidKeysDown() & KEY_B) {
        ui_pop();
        list_destroy(view);
        
        return;
    }
    
    if(selected != NULL && (selectedTouched || hidKeysDown() & KEY_A)) {
        FILE *it = fopen(itpath, "w");
        if(it == NULL) {
            error_display_res(NULL, NULL, 0, "Failed to create /CIAngel/info.txt");
            ui_pop();
            list_destroy(view);
            
            return;
        }
        
        for (int i = 0; i < numTitles; i++) {
            if (strcmp(selected->name, tkeys[i].shortDescription) == 0) {
                fprintf(it, "%s\n", tkeys[i].title_id);
                fprintf(it, "%s", tkeys[i].enc_titlekey);
                break;
            }
        }
        fclose(it);
        
        ui_pop();
        list_destroy(view);
        prompt_display("Success", "Generated input.txt for chosen title.\n\nOpen it with CIAngel to install.", COLOR_TEXT, false, NULL, NULL, NULL);
        return;
    }
    
    if(linked_list_size(items) == 0) {
        for (int i = 0; i < numTitles; i++) {
            litem[i] = (list_item){"1234567890123456", COLOR_TEXT, NULL};
            sprintf(litem[i].name, "%s", tkeys[i].shortDescription);
            linked_list_add(items, &litem[i]);
        }
    }
}

void titlemenu_open() {
    list_display("Title Menu", "A: Select, B: Cancel", NULL, titlemenu_update, titlemenu_draw_top);
}

void make_input_txt() {
    if (FileExists("/CIAngel")) {
        bool ok = true;
        u32 bufSize = 0;
        char* buf;
        FILE *json_dec = fopen(jsonpath, "wb");
        Result res;
        res = DownloadFile("https://3ds.titlekeys.com/json_dec", json_dec, false);
        fclose(json_dec);
        if(R_SUCCEEDED(res)) {
            bufSize = (u32)GetFileSize_u64(jsonpath);
            FILE *json_dec = fopen(jsonpath, "rb");
            buf = (char*)ImportFile(jsonpath, 0);
            fclose(json_dec);
            if (FileExists(eTKpath1)) {
                strncpy(eTKpath, eTKpath1, strlen(eTKpath1));
            } else if (FileExists(eTKpath2)) {
                strncpy(eTKpath, eTKpath2, strlen(eTKpath2));
            } else {
                eTKpath[0] = 0;
            }
            if (eTKpath[0] != 0) {    //(FileExists(eTKpath)) {
                FILE *eTK = fopen(eTKpath, "rb");
                u8* eTKdata = ImportFile(eTKpath, 0);
                fclose(eTK);
                numTitles = (u32)eTKdata[0];
                
                json_value* json = json_parse(buf, bufSize);
                if(json != NULL) {
                    if(json->type == json_array) {
                        
                        char temp[2];
                        
                        for (t = 0; t < numTitles; t++) {
                            for (int i = 0; i < 8; i++) {
                                sprintf(temp, "%02hhx", eTKdata[(t+1)*24 + t*8 + i]);
                                tkeys[t].title_id[i*2] = temp[0];
                                tkeys[t].title_id[i*2+1] = temp[1];
                            }
                            tkeys[t].title_id[16] = 0;
                            
                            u32 j = 0;
                            bool jsondone = false;
                            while (j < json->u.array.length && !jsondone) {
                                json_value* val = json->u.array.values[j];
                                if(val->type == json_object) {
                                    if(strncmp(val->u.object.values[0].name, "titleID", val->u.object.values[0].name_length) == 0) {
                                        if(strncmp(val->u.object.values[0].value->u.string.ptr, tkeys[t].title_id, 16) == 0) {
                                            if(strncmp(val->u.object.values[3].name, "name", val->u.object.values[3].name_length) == 0) {
                                                if (val->u.object.values[3].value->u.string.length > 0) {
                                                    strncpy(tkeys[t].shortDescription, val->u.object.values[3].value->u.string.ptr, val->u.object.values[3].value->u.string.length);
                                                } else {
                                                    strncpy(tkeys[t].shortDescription, tkeys[t].title_id, 16);
                                                }
                                            }
                                            jsondone = true;
                                        }
                                    }
                                }
                                if (!jsondone) {
                                    j++;
                                }
                            }
                            
                            for (int i = 0; i < 16; i++) {
                                sprintf(temp, "%02hhx", eTKdata[(t+1)*32 + i]);
                                tkeys[t].enc_titlekey[i*2] = temp[0];
                                tkeys[t].enc_titlekey[i*2+1] = temp[1];
                            }
                            tkeys[t].enc_titlekey[32] = 0;
                        }
                    } else {
                        prompt_display("Failure", "Bad json (2).", COLOR_TEXT, false, NULL, NULL, NULL);
                        ok = false;
                    }
                } else {
                    prompt_display("Failure", "Bad json (1).", COLOR_TEXT, false, NULL, NULL, NULL);
                    ok = false;
                }
                free(eTKdata);
                free(buf);
                
                if (ok)
                    titlemenu_open();
            } else {
                prompt_display("Failure", "encTitleKeys.bin not found in SD root.\n\nDid you run Decrypt9?", COLOR_TEXT, false, NULL, NULL, NULL);
            }
            remove(jsonpath);
        } else {
            prompt_display("Failure", "Could not download json.", COLOR_TEXT, false, NULL, NULL, NULL);
        }
    } else {
        prompt_display("Failure", "/CIAngel directory not found.\nTry installing/opening CIAngel first.", COLOR_TEXT, false, NULL, NULL, NULL);
    }
}