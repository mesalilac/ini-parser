#include "ini.h"
#include <ctype.h>
#include <ini.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void insert_kv(Section *section, KeyValue *kv)
{
    section->kv_list.kv_pairs[section->kv_list.index++] = kv;
}

typedef struct
{
    char *s;
    int index;
} TempString;

TempString *temp_string_init()
{
    TempString *ts = malloc(sizeof(TempString));

    ts->s = malloc(512 * sizeof(char));
    ts->index = 0;

    return ts;
}

void temp_string_push(TempString *ts, char c)
{
    ts->s[ts->index++] = c;
    ts->s[ts->index] = '\0';
}

void temp_string_free(TempString *ts)
{
    free(ts->s);
    free(ts);

    ts = NULL;
}
CommentsList comments_list_init()
{
    CommentsList comments_list;

    comments_list.comments = malloc(100 * sizeof(char));
    comments_list.index = 0;

    return comments_list;
}

void comments_list_push(CommentsList *comments_list, char *comment)
{
    comments_list->comments[comments_list->index] = malloc((strlen(comment) + 1) * sizeof(char));
    strncpy(comments_list->comments[comments_list->index], comment, strlen(comment) + 1);

    comments_list->index++;
}

void comments_list_free(CommentsList *comments_list)
{
    for (int i = 0; i < comments_list->index; ++i)
    {
        free(comments_list->comments[i]);
    }
    free(comments_list->comments);
    comments_list->comments = NULL;
    comments_list->index = 0;
}

KeyValue *kv_init()
{
    KeyValue *kv = malloc(sizeof(KeyValue));
    kv->key = malloc(512 * sizeof(char));
    // free values
    kv->value = malloc(sizeof(Value));

    kv->comments_list = comments_list_init();

    return kv;
}

void kv_free(KeyValue *kv)
{
    free(kv->key);
    free(kv->value);
    free(kv);
}

KvList kv_list_init()
{
    KvList kv_list;

    kv_list.kv_pairs = malloc(100 * sizeof(KeyValue));
    kv_list.index = 0;

    return kv_list;
}

void kv_list_free(KvList kv_list)
{
    for (int i = 0; i < kv_list.index; ++i)
    {
        kv_free(kv_list.kv_pairs[i]);
    }
    free(kv_list.kv_pairs);

    kv_list.kv_pairs = NULL;
    kv_list.index = 0;
}

Section *section_init()
{
    Section *section = malloc(sizeof(Section));

    section->name = malloc(512 * sizeof(char));
    section->kv_list = kv_list_init();
    section->comments_list = comments_list_init();

    return section;
}

void section_free(Section *section)
{
    free(section->name);
    section->name = NULL;
    kv_list_free(section->kv_list);
    comments_list_free(&section->comments_list);
    free(section);
}

typedef struct
{
    char *buffer;
    int cursor;
} Reader;

bool value_is_float(char *s)
{
    int dot_count = 0;
    int index = 0;

    while (s[index] != '\0')
    {
        if (s[index] == '.')
        {
            dot_count++;
        }
        else
        {
            if (!isdigit(s[index]))
                return false;
        }
        index++;
    }

    if (dot_count == 1)
        return true;

    return false;
}

bool value_is_int(char *s)
{
    int not_digits_count = 0;
    int index = 0;

    while (s[index] != '\0')
    {
        if (!isdigit(s[index]))
            not_digits_count++;

        index++;
    }

    return not_digits_count == 0;
}

bool value_is_string(char *s)
{
    int index = 0;
    int not_digits_count = 0;

    while (s[index] != '\0')
    {
        if (!isdigit(s[index]))
            not_digits_count++;

        index++;
    }

    return not_digits_count > 0;
}

const char *BOOL_LIST_TRUE[3] = {"true", "yes", "on"};
const char *BOOL_LIST_FALSE[3] = {"false", "no", "off"};

#define BOOL_LIST_SIZE 3

char *to_lowercase(char *str)
{
    char *new_str = malloc(sizeof(char) * (strlen(str) + 1));
    if (new_str == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory\n");
        return NULL;
    }

    for (int i = 0; str[i] != '\0'; ++i)
    {
        new_str[i] = tolower(str[i]);
    }

    return new_str;
}

bool value_is_bool(char *s)
{
    for (int i = 0; i < BOOL_LIST_SIZE; ++i)
    {
        if (strcmp(s, BOOL_LIST_TRUE[i]) == 0)
            return true;
        if (strcmp(s, BOOL_LIST_FALSE[i]) == 0)
            return true;
    }

    return false;
}

bool value_to_bool(char *s)
{
    for (int i = 0; i < BOOL_LIST_SIZE; ++i)
    {
        if (strcmp(s, BOOL_LIST_TRUE[i]) == 0)
            return true;
        if (strcmp(s, BOOL_LIST_FALSE[i]) == 0)
            return false;
    }

    return false;
}

Config *ini_parse(char *f_buffer)
{
    Reader r = {.buffer = f_buffer, .cursor = 0};

    Config *sections_list = malloc(sizeof(Config));
    sections_list->sections = malloc(100 * sizeof(Section));
    sections_list->index = 0;

    Section *root_section = section_init();
    root_section->name = "root";
    sections_list->sections[sections_list->index++] = root_section;

    CommentsList temp_comments_list = comments_list_init();

    int current_section_index = 0;

    while (r.buffer[r.cursor] != '\0')
    {
        if (r.buffer[r.cursor] == ';')
        {
            r.cursor++; // skip ;

            if (r.buffer[r.cursor] == ' ')
            {
                while (r.buffer[r.cursor] == ' ')
                    r.cursor++; // skip spaces
            }

            TempString *ts = temp_string_init();

            while (r.buffer[r.cursor] != '\n')
                temp_string_push(ts, r.buffer[r.cursor++]);

            char *comment_string = malloc(ts->index * sizeof(char));
            strncpy(comment_string, ts->s, strlen(ts->s) + 1);

            comments_list_push(&temp_comments_list, comment_string);

            temp_string_free(ts);
        }
        else if (r.buffer[r.cursor] == '[')
        {
            r.cursor++; // skip [
            TempString *ts = temp_string_init();
            while (r.buffer[r.cursor] != ']' && r.buffer[r.cursor] != '\0')
                temp_string_push(ts, r.buffer[r.cursor++]);

            if (r.buffer[r.cursor] == ']')
                r.cursor++; // skip ]

            Section *section = section_init();

            strncpy(section->name, ts->s, strlen(ts->s) + 1);

            for (int i = 0; i < temp_comments_list.index; ++i)
                comments_list_push(&section->comments_list, temp_comments_list.comments[i]);
            temp_comments_list.index = 0;

            sections_list->sections[sections_list->index++] = section;
            current_section_index = sections_list->index - 1;

            temp_string_free(ts);
        }
        else if (r.buffer[r.cursor] != '\n' && r.buffer[r.cursor] != ' ' && r.buffer[r.cursor] != '\0')
        {
            TempString *key = temp_string_init();

            while (r.buffer[r.cursor] != '=' && r.buffer[r.cursor] != ' ' && r.buffer[r.cursor] != '\n' &&
                   r.buffer[r.cursor] != '\0')
                temp_string_push(key, r.buffer[r.cursor++]);

            if (r.buffer[r.cursor] == ' ')
            {
                while (r.buffer[r.cursor] == ' ')
                    r.cursor++;
            }

            if (r.buffer[r.cursor] == '=')
                r.cursor++; // skip =

            if (r.buffer[r.cursor] == ' ')
            {
                while (r.buffer[r.cursor] == ' ')
                    r.cursor++;
            }

            TempString *value = temp_string_init();
            bool string_literal = false;

            while (r.buffer[r.cursor] != '\n' && r.buffer[r.cursor] != '\0')
            {
                if (r.buffer[r.cursor] == '"')
                {
                    string_literal = true;
                    r.cursor++;
                }
                else
                {
                    temp_string_push(value, r.buffer[r.cursor++]);
                }
            }

            KeyValue *kv = kv_init();

            strncpy(kv->key, key->s, strlen(key->s) + 1);

            kv->value->is_string_literal = string_literal;

            if (string_literal)
            {
                kv->value->type = TYPE_STRING;
                kv->value->value.string_value = malloc((strlen(value->s) + 1) * sizeof(char));
                strncpy(kv->value->value.string_value, value->s, strlen(value->s) + 1);
            }
            else
            {
                char *lowercase_value = to_lowercase(value->s);

                if (value_is_float(value->s))
                {
                    kv->value->type = TYPE_FLOAT;
                    kv->value->value.float_value = atof(value->s);
                }
                else if (value_is_int(value->s))
                {
                    kv->value->type = TYPE_INT;
                    kv->value->value.int_value = atoi(value->s);
                }
                else if (value_is_bool(lowercase_value))
                {
                    kv->value->type = TYPE_BOOL;
                    kv->value->value.bool_value = value_to_bool(lowercase_value);
                }
                else if (value_is_string(value->s))
                {
                    kv->value->type = TYPE_STRING;
                    kv->value->value.string_value = malloc((strlen(value->s) + 1) * sizeof(char));
                    strncpy(kv->value->value.string_value, value->s, strlen(value->s) + 1);
                }

                free(lowercase_value);
            }

            for (int i = 0; i < temp_comments_list.index; ++i)
                comments_list_push(&kv->comments_list, temp_comments_list.comments[i]);
            temp_comments_list.index = 0;

            sections_list->sections[current_section_index]
                ->kv_list.kv_pairs[sections_list->sections[current_section_index]->kv_list.index++] = kv;

            temp_string_free(key);
            temp_string_free(value);
        }

        r.cursor++;
    }

    if (temp_comments_list.comments != NULL)
        comments_list_free(&temp_comments_list);

    return sections_list;
}

bool ini_section_exists(Config *config, char *section_name)
{
    for (int i = 0; i < config->index; ++i)
    {
        if (strcmp(config->sections[i]->name, section_name) == 0)
            return true;
    }

    return false;
}

bool ini_key_exists(Config *config, char *section_name, char *key)
{
    for (int i = 0; i < config->index; ++i)
    {
        if (strcmp(config->sections[i]->name, section_name) == 0)
        {
            for (int j = 0; j < config->sections[i]->kv_list.index; ++j)
            {
                if (strcmp(config->sections[i]->kv_list.kv_pairs[j]->key, key) == 0)
                    return true;
            }
        }
    }

    return false;
}

int *ini_get_int(Config *config, char *section_name, char *key)
{
    for (int i = 0; i < config->index; ++i)
    {
        if (strcmp(config->sections[i]->name, section_name) == 0)
        {
            for (int j = 0; j < config->sections[i]->kv_list.index; ++j)
            {
                if (strcmp(config->sections[i]->kv_list.kv_pairs[j]->key, key) == 0)
                {
                    return &config->sections[i]->kv_list.kv_pairs[j]->value->value.int_value;
                }
            }
        }
    }

    return NULL;
}

float *ini_get_float(Config *config, char *section_name, char *key)
{
    for (int i = 0; i < config->index; ++i)
    {
        if (strcmp(config->sections[i]->name, section_name) == 0)
        {
            for (int j = 0; j < config->sections[i]->kv_list.index; ++j)
            {
                if (strcmp(config->sections[i]->kv_list.kv_pairs[j]->key, key) == 0)
                {
                    return &config->sections[i]->kv_list.kv_pairs[j]->value->value.float_value;
                }
            }
        }
    }

    return NULL;
}

bool *ini_get_bool(Config *config, char *section_name, char *key)
{
    for (int i = 0; i < config->index; ++i)
    {
        if (strcmp(config->sections[i]->name, section_name) == 0)
        {
            for (int j = 0; j < config->sections[i]->kv_list.index; ++j)
            {
                if (strcmp(config->sections[i]->kv_list.kv_pairs[j]->key, key) == 0)
                {
                    return &config->sections[i]->kv_list.kv_pairs[j]->value->value.bool_value;
                }
            }
        }
    }

    return NULL;
}

char *ini_get_string(Config *config, char *section_name, char *key)
{
    for (int i = 0; i < config->index; ++i)
    {
        if (strcmp(config->sections[i]->name, section_name) == 0)
        {
            for (int j = 0; j < config->sections[i]->kv_list.index; ++j)
            {
                if (strcmp(config->sections[i]->kv_list.kv_pairs[j]->key, key) == 0)
                {
                    return config->sections[i]->kv_list.kv_pairs[j]->value->value.string_value;
                }
            }
        }
    }

    return NULL;
}

char *ini_to_string(Config *config)
{
    int index = 0;
    char *buffer = malloc(1024 * sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for buffer\n");
        return NULL;
    }

    for (int i = 0; i < config->index; ++i)
    {
        Section *section = config->sections[i];
        bool is_root_section = false;

        // push comments
        for (int j = 0; j < section->comments_list.index; ++j)
        {
            buffer[index++] = ';';
            buffer[index++] = ' ';

            for (int k = 0; k < strlen(section->comments_list.comments[j]); ++k)
            {
                buffer[index++] = section->comments_list.comments[j][k];
            }

            buffer[index++] = '\n';
        }

        if (strcmp(section->name, "root") != 0)
        {
            buffer[index++] = '[';

            for (int j = 0; j < strlen(section->name); ++j)
            {
                buffer[index++] = section->name[j];
            }

            buffer[index++] = ']';

            buffer[index++] = '\n';
        }

        for (int j = 0; j < section->kv_list.index; ++j)
        {
            KeyValue *kv = section->kv_list.kv_pairs[j];

            for (int k = 0; k < kv->comments_list.index; ++k)
            {
                buffer[index++] = ';';
                buffer[index++] = ' ';
                for (int l = 0; l < strlen(kv->comments_list.comments[k]); ++l)
                {
                    buffer[index++] = kv->comments_list.comments[k][l];
                }

                buffer[index++] = '\n';
            }

            for (int k = 0; k < strlen(kv->key); ++k)
            {
                buffer[index++] = kv->key[k];
            }

            buffer[index++] = ' ';
            buffer[index++] = '=';
            buffer[index++] = ' ';

            if (kv->value->is_string_literal)
                buffer[index++] = '"';

            // loop over value

            Value *value = kv->value;

            switch (value->type)
            {
            case TYPE_INT: {
                char *new_str = malloc(128 * sizeof(char));
                sprintf(new_str, "%i", value->value.int_value);

                for (int k = 0; k < strlen(new_str); ++k)
                {
                    buffer[index++] = new_str[k];
                }

                free(new_str);
            }
            break;

            case TYPE_FLOAT: {
                char *new_str = malloc(128 * sizeof(char));
                sprintf(new_str, "%f", value->value.float_value);

                for (int k = 0; k < strlen(new_str); ++k)
                {
                    buffer[index++] = new_str[k];
                }

                free(new_str);
            }
            break;

            case TYPE_BOOL:
                if (value->value.bool_value == 0)
                {
                    buffer[index++] = 'f';
                    buffer[index++] = 'a';
                    buffer[index++] = 'l';
                    buffer[index++] = 's';
                    buffer[index++] = 'e';
                }
                else
                {
                    buffer[index++] = 't';
                    buffer[index++] = 'r';
                    buffer[index++] = 'u';
                    buffer[index++] = 'e';
                }
                break;

            case TYPE_STRING:
                for (int k = 0; k < strlen(value->value.string_value); ++k)
                {
                    buffer[index++] = value->value.string_value[k];
                }
                break;
            }

            if (kv->value->is_string_literal)
                buffer[index++] = '"';

            buffer[index++] = '\n';
        }
        buffer[index++] = '\n';
    }

    buffer[index] = '\0';
    return buffer;
}
