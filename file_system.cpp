
#include <vector>
#include "file_system.h"
#include "string_functions.h"


FileSystem::FileSystem() {

}

int FileSystem::mount(const std::string &fat_file) {
    fs = fopen(fat_file.c_str(), "rb+");
    br = new boot_record;
    fread(br, sizeof(br), 1, fs);
    fat = new int32_t[br->usable_cluster_count];
    fread(fat, sizeof(fat), 1, fs);
    if (br->fat_copies > 1) {
        fseek(fs, (br->fat_copies - 1) * br->usable_cluster_count, SEEK_CUR);
    }
    clusters_offset = (int32_t) ftell(fs);
    max_files_in_dir = br->cluster_size / sizeof(directory);

    mounted = true;
    return EXIT_OK;
}

int FileSystem::umount() {
    if (!mounted)
        return EXIT_NOT_MOUTED;
    sync();
    fclose(fs);

    fs = NULL;
    delete br;
    delete fat;
    return EXIT_OK;
}

int FileSystem::create(const std::string &fat_file, int cluster_count, int16_t cluster_size, int8_t fat_count,
                       const std::string &volume_descriptor, const std::string &signature) {
    FILE *file = fopen(fat_file.c_str(), "wb+");


    // boot record
    boot_record *br = new boot_record();
    memset(br->volume_descriptor, '\0', sizeof(br->volume_descriptor));
    strcpy(br->volume_descriptor, volume_descriptor.c_str());
    memset(br->signature, '\0', sizeof(br->signature));
    strcpy(br->signature, signature.c_str());
    br->cluster_size = cluster_size;
    br->usable_cluster_count = cluster_count;
    br->fat_type = 8;
    br->fat_copies = fat_count;

    char cluster[cluster_size]; // = new char[cluster_size];
    memset(cluster, '\0', cluster_size);

    int32_t fat[cluster_count]; // = new new int32_t[cluster_count];
    for (int32_t &code : fat) {
        code = FAT_UNUSED;
    }


    fat[0] = FAT_DIRECTORY;

    fwrite(br, sizeof(br), 1, file);
    for (int i = 0; i < fat_count; ++i) {
        fwrite(fat, sizeof(fat), 1, file);
    }
    for (int j = 0; j < cluster_count; ++j) {
        fwrite(cluster, cluster_size, 1, file);
    }

    fclose(file);

    delete (br);


    return EXIT_OK;
}

void FileSystem::sync() {
    if (mounted && fs) fflush(fs);

}

int32_t FileSystem::first_unused(int start=0) {
    int32_t i = start;
    while (i > br->usable_cluster_count) {
        if (fat[i]==FAT_UNUSED) {
            return i;
        }
        i++;
    }
    return -1;
}

int32_t FileSystem::cd(const std::string &filename) {
    int32_t current_dir = 0;
    int depth = 0;
    std::vector<std::string> path = split(filename,'/');
    deeper:
    while (depth + 2 < path.size()) {
        std::vector<directory> *dir = get_dir(current_dir);
        for (directory x : *dir) {
            if (!x.isFile) {
                if (path[depth].compare(x.name)==0) {
                    current_dir = x.start_cluster;
                    depth++;
                    goto depth;
                }
            }
        }
        return -1; // pokud nelze traverzovat
    }
    return current_dir;
}

std::vector<directory> *FileSystem::get_dir(int32_t addr) {
    directory tmp;
    std::vector<directory> *r = new std::vector<directory>();
    fseek(fs, addr*br->cluster_size + clusters_offset);
    fread(&tmp,sizeof(directory),1,fs);
    while (r->size()<= max_files_in_dir && tmp.name[0]!='\0') {
        r->push_back(tmp);
        fread(&tmp,sizeof(directory),1,fs);
    }
    return r;
}

void FileSystem::write_dir(int32_t addr, const std::vector<directory> *dir) {


    fwrite(&dir,sizeof(directory),dir->size(),fs);
    if (dir->size()<max_files_in_dir) {
        char n = '\0';
        fwrite(n,1,1,fs);
    }


}

