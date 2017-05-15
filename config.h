#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <malloc.h>

/* Holds values from config file */
struct config {
    char *if_name;
};

// Populates provided struct with config values
// read from configuration file on disk
bool get_config(struct config*);

// Frees memomy allocates for keys in config struct
void config_dispose(struct config*);

#endif
