#ifndef ZOS_FILE_SYSTEM_H
#define ZOS_FILE_SYSTEM_H

#include <string>
#include "zmk.h"

#define EXIT_OK 0
#define EXIT_NOT_MOUTED 1

class FileSystem {
public:

    //static const int EXIT_OK = 0; // jeste uvidime

    FileSystem();
    int mount (const std::string &fat_file);
    void sync ();
    int umount ();
    int create(const std::string &fat_file, int cluster_count, int16_t cluster_size, int8_t fat_count,
               const std::string &volume_descriptor, const std::string &signature);
private:



    FILE *fs;
    bool mounted = false;
    boot_record *br; //tady se ulozi u mountnute veci
    int32_t fat[];
    int32_t clusters_offset;
    int32_t max_files_in_dir;

    int32_t first_unused(int start=0);
    int32_t cd(const std::string &filename);


    std::vector<directory> * get_dir(int32_t addr);

    void write_dir(int32_t addr, const std::vector<directory> *dir);
};

#endif //ZOS_FILE_SYSTEM_H
