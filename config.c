#include "config.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define CONFIG_FILE_PATH "/etc/sniffer/config"
#define MAX_LINE_SIZE 256

/* Possible keys of config file */
#define IFACE_KEY "iface"


static int first_equals_index(char *line);

void config_dispose(struct config *conf){
    free (conf->if_name);
}

struct config* get_config(){
    struct config *conf = malloc (sizeof (struct config));
    assert (conf != NULL);
    // Open config file on disk
    FILE *conf_file = fopen(CONFIG_FILE_PATH, "r");
    if (conf_file == NULL){
        fprintf(stderr, "Can open config file: CONFIG_FILE_PATH\n");
        return false;
    }
    // Read it line by line
    char line[MAX_LINE_SIZE];
    char key[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    while (fgets(line, sizeof(line), conf_file)) {
        int line_len = strnlen(line, MAX_LINE_SIZE);
        if (line_len == MAX_LINE_SIZE){
            fprintf(stderr, "[CONFIG] Line in config file is too large\n");
            continue;
        }
        line[line_len - 1] = '\0';
        line_len -= 1;  // account for newline
        // Extract key and value on particular line
        int equals_index = first_equals_index(line);
        if (equals_index == -1 || equals_index == 0){
            fprintf(stderr, "[CONFIG] Wrong format of config file, should be `token=value`\n");
            continue;
        }
        strncpy(key, line, equals_index);
        key[equals_index] = '\0'; 
        strncpy(value, line + equals_index + 1, line_len - equals_index - 1);
        value[line_len - equals_index - 1] = '\0';

        if (strcmp(IFACE_KEY, key) == 0){
            conf->if_name = strdup(value);
        } else {
            fprintf(stderr, "[CONFIG] key unrecognizable in `key=value` pair\n");
        }
        
    }
    fclose(conf_file);
    return conf;
}

/* Returns index of first '=' char in line if exists, -1 otherwise */
static int first_equals_index(char *line){
    int i = 0;
    int line_len = strlen(line);
    while (i < line_len && line[i] != '=')
        i += 1;
    if (i >= line_len - 1)
        return -1;
    return i;
}

