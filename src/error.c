#include "error.h"

void reportError(ErrorCode code, const char *message) {
    fprintf(stderr, "Error [%d]: %s\n", code, message);
}

void handleFatalError(ErrorCode code, const char *message) {
    reportError(code, message);
    exit(code);
}
