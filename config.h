#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <malloc.h>

/* Holds values from config file */
struct config {
    char *if_name;
};

// Returns dynamically allocated struct with config values
// read from configuration file on disk
struct config* get_config();

// Frees memomy allocates for keys in config struct
void config_dispose(struct config*);

#endif
