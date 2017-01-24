#ifndef ZOS_FILE_SYSTEM_H
#define ZOS_FILE_SYSTEM_H

#include <string>
#include <vector>
#include "zmk.h"

class FileSystem {
public:

    static const int EXIT_OK = 0; // jeste uvidime
    static const int EXIT_PATH_NOT_FOUND = 1;
    static const int EXIT_PATH_NOT_EMPTY = 2;
    static const int EXIT_EMPTY = 3;
    static const int EXIT_DIR_FULL = 4;
    static const int EXIT_NOT_MOUTED = 5;

    static const char *CODES_STR[];
    //iniciace pod tridou

    FileSystem();

    ~FileSystem();

    virtual int mount(const std::string &fat_file);

    virtual int umount();

    void sync();

    int create(const std::string &fat_file, int cluster_count, int16_t cluster_size, int8_t fat_count,
               const std::string &volume_descriptor, const std::string &signature);

    int put_file(const std::string &file, const std::string &path);

    int del_file(std::string &path);

    std::string list_of_clusters_of_file(const std::string &path);

    void print_file(const std::string &path);

    static void print_code(int code);

    int
    mk_dir(const std::string &dir_name, const std::string &path);

    void print_tree();

    int rm_dir(const std::string &path);


protected:
    FILE *fs;
    bool mounted = false;
    boot_record *br; //tady se ulozi u mountnute veci
    int32_t *fat;
    int32_t clusters_offset;
    int32_t max_files_in_dir;

    int32_t first_unused(int start);

    int32_t cd(const std::string &filename);


    /// buffer pro cteni a zapis clusteru
    char *buffer_cluster;
    /// urceny k nulovani clusteru, nenmenim mu hodnotu
    char *zero_cluster;

    std::vector<directory> *get_dir(int32_t cluster);

    void write_dir(int32_t addr, std::vector<directory> *dir);

    int32_t cluster_to_addr(int32_t cluster);

    void flush_fat();

    void print_dir(const std::string &dir_name, int32_t cluster, int32_t depth);

};


#endif //ZOS_FILE_SYSTEM_H
