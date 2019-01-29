#pragma once
#include "jeruvm.h"
#include <stdbool.h>

bool run_next_token(JeruVM *vm, JeruBlock *scope, bool nopop);
