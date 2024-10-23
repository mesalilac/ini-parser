#ifndef INI_H
#define INI_H

#include <stdbool.h>

typedef enum
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING
} ValueType;

typedef union {
    int int_value;
    float float_value;
    bool bool_value;
    char *string_value;
} ValueUnion;

typedef struct
{
    ValueType type;
    ValueUnion value;
    bool is_string_literal;
} Value;

typedef struct
{
    char **comments;
    int index;
} CommentsList;

typedef struct
{
    char *key;
    Value *value;

    CommentsList comments_list;
} KeyValue;

typedef struct
{
    KeyValue **kv_pairs;
    int index;
} KvList;

typedef struct
{
    char *name;
    KvList kv_list;
    CommentsList comments_list;
} Section;

typedef struct
{
    Section **sections;
    int index;
} Config;

Config *ini_parse(char *text);
bool ini_section_exists(Config *config, char *section_name);
bool ini_key_exists(Config *config, char *section_name, char *key);

int *ini_get_int(Config *config, char *section_name, char *key);
float *ini_get_float(Config *config, char *section_name, char *key);
bool *ini_get_bool(Config *config, char *section_name, char *key);
char *ini_get_string(Config *config, char *section_name, char *key);

char *ini_to_string(Config *config);

#endif // !INI_H
