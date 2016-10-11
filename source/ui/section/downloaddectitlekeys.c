#include <stdio.h>
#include <string.h>

#include <3ds.h>

#include "section.h"
#include "task/task.h"
#include "../prompt.h"
#include "../ui.h"
#include "../../core/screen.h"
#include "utils.h"

static char dTKpath1[] = "/files9/decTitleKeys.bin";
static char dTKpath2[] = "/decTitleKeys.bin";
char dTKpath[FILE_PATH_MAX];
static char url[] = "https://3ds.titlekeys.com/downloadmissingforencryption";

u32 numTitles = 0;

void download_dectitlekeys() {
    if (FileExists("/files9")) {
        strncpy(dTKpath, dTKpath1, strlen(dTKpath1));
    } else {
        strncpy(dTKpath, dTKpath2, strlen(dTKpath2));
    }
    FILE *dTK = fopen(dTKpath, "wb");
    Result res;
    res = DownloadFile(url, dTK, false);
    fclose(dTK);
    
    if(R_SUCCEEDED(res)) {
        FILE *dTK = fopen(dTKpath, "rb");
        if (dTK != NULL && GetFileSize_u64(dTKpath) > 0) {
            fread(&numTitles, 4, 1, dTK);
            if (numTitles == 0) {
                prompt_display("Info", "Downloaded decTitleKeys.bin has 0 entries.\n\nEncrypted keys should be up to date on\ntitlekey site, so just install with CIAngel etc.", COLOR_TEXT, false, NULL, NULL, NULL);
            } else {
                prompt_display("Success", "decTitleKeys.bin downloaded.", COLOR_TEXT, false, NULL, NULL, NULL);
            }
        } else {
            prompt_display("Failure", "decTitleKeys.bin could not be opened.", COLOR_TEXT, false, NULL, NULL, NULL);
        }
    } else {
        prompt_display("Failure", "decTitleKeys.bin could not be downloaded.", COLOR_TEXT, false, NULL, NULL, NULL);
    }
}