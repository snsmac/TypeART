// RUN: %scriptpath/applyAndRun.sh %s %pluginpath "-must-alloca" %rtpath 2>&1 | FileCheck %s

#include "../struct_defs.h"
#include <stdint.h>
#include <stdlib.h>
#include "util.h"


int main(int argc, char** argv) {
    // TODO

    return 0;
}
