/* Parameter macros for the mustache parsing engine. */
/* usage of these are optional and exist to simplify */
/* the writing and configuration of parameters. */

#ifndef MUSTACHE_MPARAM_H99
#define MUSTACHE_MPARAM_H99

#include <string.h>


#define MPARAM_CSTR(VAR, NAME, PNEXT, VAL) mustache_param_string VAR = {.pNext = (PNEXT), .type = MUSTACHE_PARAM_STRING, .name = {NAME,strlen(NAME)}, .str = {VAL, strlen(VAL)}}

#define MPARAM_STR(VAR, NAME, PNEXT, SLICE) mustache_param_string VAR = {.pNext = (PNEXT), .type = MUSTACHE_PARAM_STRING, .name = {NAME,strlen(NAME)}, .str = SLICE}

#define MPARAM_BOOL(VAR, NAME, PNEXT, VAL) mustache_param_boolean VAR = {.pNext = (PNEXT), .type = MUSTACHE_PARAM_BOOLEAN, .name = {NAME,strlen(NAME)}, .value = VAL}

#define MPARAM_NUM(VAR, NAME, PNEXT, VAL) mustache_param_number VAR = {.pNext = (PNEXT), .type = MUSTACHE_PARAM_NUMBER, .name = {NAME,strlen(NAME)}, .value = VAL, .decimals = 5, trimZeros = 1}

#define MPARAM_NUMF(VAR, NAME, PNEXT, VAL, DECIMALS, TRIMZEROS) mustache_param_number VAR = {.pNext = (PNEXT), .type = MUSTACHE_PARAM_NUMBER, .name = {NAME,strlen(NAME)}, .value = VAL, .decimals = DECIMALS, .trimZeros = TRIM_ZEROS}

#define MPARAM_LIST(VAR, NAME, PNEXT, CHILDCOUNT, CHILDREN) mustache_param_list VAR = {.pNext = (PNEXT), .type = MUSTACHE_PARAM_LIST, .name = {NAME,strlen(NAME)}, .valueCount=CHILDCOUNT, .pValues=CHILDREN}

#define MPARAM_OBJECT(VAR, NAME, PNEXT, CHILDREN) mustache_param_object VAR = {.pNext = (PNEXT), .type = MUSTACHE_PARAM_OBJECT, .name = {NAME,strlen(NAME)}, .pMembers=CHILDREN}

#endif