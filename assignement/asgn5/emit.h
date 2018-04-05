#ifndef __EMIT_H
#define __EMIT_H
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "lyutils.h"
#include "auxlib.h"
#include "astree.h"

int emit(FILE *outfile, astree *root);

#endif
