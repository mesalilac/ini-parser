#include <stdio.h>
#include <stdlib.h>

#include "include/ini.h"

void print_config(const Config *config)
{
    for (int i = 0; i < config->index; ++i)
    {
        for (int j = 0; j < config->sections[i]->comments_list.index; ++j)
        {
            printf("comments: '%s'\n", config->sections[i]->comments_list.comments[j]);
        }
        printf("section: '%s'\n", config->sections[i]->name);

        printf("section kv pairs:\n");

        for (int j = 0; j < config->sections[i]->kv_list.index; ++j)
        {
            for (int k = 0; k < config->sections[i]->kv_list.kv_pairs[j]->comments_list.index; ++k)
            {
                printf("\tkv comments: '%s'\n", config->sections[i]->kv_list.kv_pairs[j]->comments_list.comments[k]);
            }
            printf("\tkey: '%s'\n", config->sections[i]->kv_list.kv_pairs[j]->key);
            switch (config->sections[i]->kv_list.kv_pairs[j]->value->type)
            {
            case TYPE_INT:
                printf("\tvalue (int): '%i'\n", config->sections[i]->kv_list.kv_pairs[j]->value->value.int_value);
                break;
            case TYPE_BOOL:
                printf("\tvalue (bool): '%i'\n", config->sections[i]->kv_list.kv_pairs[j]->value->value.bool_value);
                break;
            case TYPE_FLOAT:
                printf("\tvalue (float): '%f'\n", config->sections[i]->kv_list.kv_pairs[j]->value->value.float_value);
                break;
            case TYPE_STRING:
                printf("\tvalue (string): '%s'\n", config->sections[i]->kv_list.kv_pairs[j]->value->value.string_value);
                break;
            }
        }
    }
}

int main()
{
    char *filename = "test.ini";

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    char *buffer = malloc(1024 * sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for buffer\n");
        return 1;
    }

    fread(buffer, 1, file_size, fp);
    buffer[file_size] = '\0';

    fclose(fp);

    Config *config = ini_parse(buffer);

    print_config(config);

    printf("########################################################\n");
    printf("\n");

    printf("player name: '%s'\n", ini_get_string(config, "player", "name"));
    printf("player hp: %f\n", *ini_get_float(config, "player", "hp"));
    printf("player last_used_item: '%s'\n", ini_get_string(config, "player", "last_used_item"));

    printf("\n");

    printf("player.inventory size: %i\n", *ini_get_int(config, "player.inventory", "size"));
    printf("player.inventory items_count: %i\n", *ini_get_int(config, "player.inventory", "items_count"));

    printf("\n");

    printf("enemy name: '%s'\n", ini_get_string(config, "enemy", "name"));
    printf("enemy hp: %f\n", *ini_get_float(config, "enemy", "hp"));
    printf("enemy alive: %i\n", *ini_get_bool(config, "enemy", "alive"));

    printf("########################################################\n");

    char *config_as_string = ini_to_string(config);

    printf("config as string\n");
    printf("'%s'", config_as_string);

    return 0;
}
