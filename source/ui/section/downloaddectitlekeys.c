#include <stdio.h>

#include <3ds.h>

#include "section.h"
#include "../prompt.h"
#include "../ui.h"
#include "../../core/screen.h"
#include "utils.h"

static char dTKpath[] = "/decTitleKeys.bin";
static char url[] = "https://3ds.titlekeys.com/downloadmissingforencryption";

void download_dectitlekeys() {
    FILE *dTK = fopen(dTKpath, "wb");
    Result res;
    res = DownloadFile(url, dTK, false);
    fclose(dTK);
    
    if(R_SUCCEEDED(res)) {
        prompt_display("Success", "decTitleKeys.bin downloaded.", COLOR_TEXT, false, NULL, NULL, NULL);
    } else {
        prompt_display("Failure", "decTitleKeys.bin could not be downloaded.", COLOR_TEXT, false, NULL, NULL, NULL);
    }
}