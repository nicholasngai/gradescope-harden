#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <yaml.h>
#include "config.h"
#include "defs.h"

static int read_bool(const yaml_event_t *event, bool *val) {
    int ret;

    if (strcmp((char *) event->data.scalar.value, "true") == 0) {
        *val = true;
    } else if (strcmp((char *) event->data.scalar.value, "false") == 0) {
        *val = false;
    } else {
        ret = -1;
        goto exit;
    }

    ret = 0;

exit:
    return ret;
}

int config_read(const char *config_path, struct config *config) {
    int ret;

    /* Init parser. */
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Error initializing YAML parser\n");
        ret = -1;
        goto exit;
    }

    /* Open input file. */
    FILE *f = fopen(config_path, "r");
    if (!f) {
        perror("fopen");
        ret = -1;
        goto exit_delete_parser;
    }

    /* Set file input. */
    yaml_parser_set_input_file(&parser, f);

    /* Initialize default config. */
    memset(config, '\0', sizeof(*config));

    /* Parse events. We only have config variables at the root, so we only track
     * the level. */
    bool done;
    enum state {
        START,
        READ_CONFIG,
        READ_DISABLE_NETWORKING,
        END,
    };
    enum state state = START;
    do {
        yaml_event_t event;
        if (!yaml_parser_parse(&parser, &event)) {
            goto exit_free_event;
        }

        switch (event.type) {
        case YAML_MAPPING_START_EVENT:
            switch (state) {
            case START:
                state = READ_CONFIG;
                break;
            default:
                fprintf(stderr, "Config does not match schema!\n");
                ret = -1;
                goto exit_free_event;
            }
            break;
        case YAML_MAPPING_END_EVENT:
            switch (state) {
            case READ_CONFIG:
                state = END;
                break;
            case READ_DISABLE_NETWORKING:
                state = READ_CONFIG;
                break;
            default:
                fprintf(stderr, "Config does not match schema!\n");
                ret = -1;
                goto exit_free_event;
            }
            break;
        case YAML_SCALAR_EVENT:
            switch (state) {
            case READ_CONFIG:
                if (strcmp((char *) event.data.scalar.value,
                            "disable_networking")
                        == 0) {
                    state = READ_DISABLE_NETWORKING;
                } else {
                    fprintf(stderr, "Config does not match schema!\n");
                    ret = -1;
                    goto exit_free_event;
                }
                break;
            case READ_DISABLE_NETWORKING:
                if (read_bool(&event, &config->disable_networking)) {
                    fprintf(stderr, "Config does not match schema!\n");
                    ret = -1;
                    goto exit_free_event;
                }
                state = READ_CONFIG;
                break;
            default:
                fprintf(stderr, "Config does not match schema!\n");
                ret = -1;
                goto exit_free_event;
            }
            break;
        case YAML_STREAM_START_EVENT:
        case YAML_STREAM_END_EVENT:
        case YAML_DOCUMENT_START_EVENT:
        case YAML_DOCUMENT_END_EVENT:
            break;
        default:
            fprintf(stderr, "Config does not match schema!\n");
            ret = -1;
            goto exit_free_event;
        }

        done = event.type == YAML_STREAM_END_EVENT;

        ret = 0;

exit_free_event:
        yaml_event_delete(&event);
        if (ret) {
            goto exit_close_file;
        }
    } while (!done);

exit_close_file:
    fclose(f);
exit_delete_parser:
    yaml_parser_delete(&parser);
exit:
    return ret;
}

void config_free(struct config *config UNUSED) {}
