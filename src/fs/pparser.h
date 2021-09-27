#ifndef PATHPARSER_H
#define PATHPARSER_H

struct path_root{
    int drive;
    struct path_part* first;
};

struct path_part{
    const char* part;
    struct path_part* next;
};

struct path_root* parse(const char* path, const char* current_dir);
void parser_free(struct path_root* root);

#endif