
#include <cstdio>
#include <sstream>
#include "file_system.h"
#include "string_functions.h"


const char *FileSystem::CODES_STR[] = {
        "OK",
        "PATH NOT FOUND",
        "PATH NOT EMPTY",
        "EMPTY",
        "DIR FULL",
        "NOT MOUNTED"
};

FileSystem::FileSystem() {

}

FileSystem::~FileSystem() {

}


int FileSystem::mount(const std::string &fat_file) {
    fs = fopen(fat_file.c_str(), "rb+");
    br = new boot_record;
    fread(br, sizeof(boot_record), 1, fs);
    fat = new int32_t[br->usable_cluster_count];

    fread(fat, br->usable_cluster_count * sizeof(int32_t), 1, fs);

    clusters_offset = sizeof(boot_record) + sizeof(int32_t) * br->usable_cluster_count * br->fat_copies;
    max_files_in_dir = br->cluster_size / sizeof(directory);
    buffer_cluster = new char[br->cluster_size];
    zero_cluster = new char[br->cluster_size];
    memset(zero_cluster, '\0', (size_t) br->cluster_size); // znulujem
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
    free(fat);
    delete buffer_cluster;
    delete zero_cluster;
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

    fwrite(br, sizeof(boot_record), 1, file);
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

int32_t FileSystem::first_unused(int start = 0) {
    int32_t i = start;
    while (i <= br->usable_cluster_count) {
        if (fat[i] == FAT_UNUSED) {
            return i;
        }
        i++;
    }
    //pokad dojedem do konce tak bude chtit zacit od zacatku
    i = 0;
    while (i < start) {
        if (fat[i] == FAT_UNUSED) {
            return i;
        }
        i++;
    }
    return -1;
}

int32_t FileSystem::cd(const std::string &filename) {
    int32_t current_dir_cluster = 0;
    int depth = 0;
    std::string new_path = filename;

    // aby jsme mohli psat cesty zacinajici lomitkem
    // ktere ale budeme ignorovat, jen potrebujeme
    // aby byli akceptovany jako argument
    if (new_path.front() == '/') {
        new_path = new_path.substr(1);
    };
    std::vector<std::string> path = split(new_path, '/');
    if (path.size()<2)
        return 0; //root


    // nyni zkusime traverzovat zkrz cestu
    // to co je za poslednim lomitkem je ignorovano
    deeper:
    while (depth + 1 < path.size()) {
        std::vector<directory> *dir = get_dir(current_dir_cluster);
        for (directory x : *dir) {
            if (!x.isFile) {
                if (path[depth].compare(x.name) == 0) {
                    current_dir_cluster = x.start_cluster;
                    depth++;
                    delete dir;
                    goto deeper;
                }
            }
        }
        return -1; // pokud nelze traverzovat
    }
    return current_dir_cluster;
}

std::vector<directory> *FileSystem::get_dir(int32_t cluster) {
    directory tmp;
    std::vector<directory> *r = new std::vector<directory>();
    fseek(fs, cluster_to_addr(cluster), SEEK_SET);
    fread(&tmp, sizeof(directory), 1, fs);
    while (r->size() <= max_files_in_dir && tmp.name[0] != '\0') {
        r->push_back(tmp);
        fread(&tmp, sizeof(directory), 1, fs);
    }
    return r;
}

void FileSystem::write_dir(int32_t addr, std::vector<directory> *dir) {

    fseek(fs, cluster_to_addr(addr), SEEK_SET);
    for (int i = 0; i < dir->size(); ++i) {
        fwrite(&((*dir)[i]), sizeof(directory), 1, fs);
    }
    // doplnime nulama
    fwrite(zero_cluster, br->cluster_size - (dir->size()) * sizeof(directory), 1, fs);


}

int FileSystem::put_file(const std::string &file, const std::string &path) {
    auto dir_cluster = cd(path);
    auto dir = get_dir(dir_cluster);
    if (dir->size() >= max_files_in_dir) return EXIT_DIR_FULL;
    auto first = first_unused(dir_cluster + 1);
    FILE *input_file = fopen(file.c_str(), "rb");

    // zjistime velikost
    fseek(input_file, 0, SEEK_END);
    int32_t size = (int32_t) ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    int32_t cluster_count = size / br->cluster_size;
    cluster_count++;
    // pocet zapsanych clusteru
    int32_t writen = 0;
    auto old = first;
    while (1) {
        memset(buffer_cluster, '\0', br->cluster_size);
        fread(buffer_cluster, br->cluster_size, 1, input_file);
        fseek(fs, cluster_to_addr(old), SEEK_SET);
        fwrite(buffer_cluster, br->cluster_size, 1, fs);
        writen++;
        if (writen >= cluster_count) {
            fat[old] = FAT_FILE_END;
            break;
        } else {
            int32_t next = first_unused(old + 1);
            fat[old] = next;
            old = next;
        }
    }
    fclose(input_file);

    directory *dir_item = new directory;
    dir_item->isFile = true;
    dir_item->start_cluster = first;
    memset(dir_item->name,'\0',sizeof(directory::name));
    split(path, '/').back().copy(dir_item->name,sizeof(directory::name)-1);
    dir_item->size = size;

    dir->push_back(*dir_item);
    delete dir_item;
    write_dir(dir_cluster, dir);

    flush_fat();
    return EXIT_OK;

}

int32_t FileSystem::cluster_to_addr(int32_t cluster) {
    return cluster * br->cluster_size + clusters_offset;
}

void FileSystem::flush_fat() {
    fseek(fs, sizeof(boot_record), SEEK_SET);
    for (int i = 0; i < br->fat_copies; ++i) {
        fwrite(fat, br->usable_cluster_count * sizeof(int32_t), 1, fs);
    }

}

int FileSystem::del_file(std::string &path) {
    auto dir_cluster = cd(path);
    auto dir = get_dir(dir_cluster);
    std::string &filename = split(path, '/').back();
    for (int i = 0; i < dir->size(); ++i) {
        if (filename.compare((*dir)[i].name) == 0) {
            int32_t next = (*dir)[i].start_cluster;
            while (fat[next] != FAT_FILE_END) {
                int32_t tmp = fat[next];
                fat[next] = FAT_UNUSED;
                next = tmp;
            }
            fat[next] = FAT_UNUSED;
            dir->erase(dir->begin() + i);
            write_dir(dir_cluster, dir);
            flush_fat();
            return EXIT_OK;
        }
    }

    return EXIT_PATH_NOT_FOUND;
}

std::string FileSystem::list_of_clusters_of_file(const std::string &path) {
    std::stringstream r;
    auto dir_cluster = cd(path);
    auto dir = get_dir(dir_cluster);
    std::string &filename = split(path, '/').back();
    for (int i = 0; i < dir->size(); ++i) {
        if (filename.compare((*dir)[i].name) == 0) {
            int32_t next = (*dir)[i].start_cluster;
            while (fat[next] != FAT_FILE_END) {
                int32_t tmp = fat[next];
                r << next << ":";
                next = tmp;
            }
            r << next;
            return r.str(); //vrati se plny seznam clusteru
        }
    }
    return ""; // vrati se prazny - zname ze se nenaslo
}

void FileSystem::print_file(const std::string &path) {
    auto dir_cluster = cd(path);
    auto dir = get_dir(dir_cluster);
    std::string filename = split(path, '/').back().c_str(); // clion bez c_str hlasi chybu i kdyz tam neni pri kompilaci
    for (int i = 0; i < dir->size(); ++i) {
        if (filename.compare((*dir)[i].name) == 0) {
            int32_t next = (*dir)[i].start_cluster;
            std::cout << filename <<  ": ";
            while (fat[next] != FAT_FILE_END) {
                int32_t tmp = fat[next];
                fseek(fs, cluster_to_addr(next), SEEK_SET);
                fread(buffer_cluster, br->cluster_size, 1, fs);
                std::cout << buffer_cluster;
                next = fat[next];
            }
            fseek(fs, cluster_to_addr(next), SEEK_SET);
            fread(buffer_cluster, br->cluster_size, 1, fs);

            std::cout.write(buffer_cluster, (size_t) (*dir)[i].size % br->cluster_size); // limitujem jenom na konec souboru
            std::cout << std::endl; // diskutabilni zda mame jeste davat na konec mezeru

            return;
        }
    }
    print_code(EXIT_PATH_NOT_FOUND);
    return;

}

void FileSystem::print_code(int code) {
    std::cout << FileSystem::CODES_STR[code] << std::endl;
}

int FileSystem::mk_dir(const std::string &dir_name, const std::string &path) {
    auto dir_cluster = cd(path);
    if (dir_cluster == -1)
        return EXIT_PATH_NOT_FOUND;
    auto dir = get_dir(dir_cluster);
    if (dir->size() >= max_files_in_dir) return EXIT_DIR_FULL;
    int32_t new_cluster = first_unused(dir_cluster + 1);
    directory *dir_item = new directory;
    dir_item->isFile = false;
    dir_item->start_cluster = new_cluster;
    memset(dir_item->name,'\0',sizeof(directory::name));
    dir_name.copy(dir_item->name,sizeof(directory::name)-1);
    dir_item->size = 0;
    dir->push_back(*dir_item);
    delete dir_item;
    write_dir(dir_cluster, dir);
    fat[new_cluster] = FAT_DIRECTORY;
    flush_fat();
    return 0;
}

void FileSystem::print_tree() {
    auto dir = get_dir(0);
    if (dir->size() == 0) {
        std::cout << CODES_STR[EXIT_EMPTY] << std::endl;
    } else {
        print_dir("ROOT", 0, 0);
    }
}

void FileSystem::print_dir(const std::string &dir_name, int32_t cluster, int32_t depth) {
    std::string tabs;
    for (int i = 0; i < depth; ++i) {
        tabs = tabs + "\t";
    }

    auto dir = get_dir(cluster);
    if (depth > 0) {
        std::cout << tabs << "+" << dir_name << " " << cluster << std::endl;
    } else {
        std::cout << tabs << "+" << dir_name << std::endl; // protoze root nema mit cislo clusteru
    }
    for (int i = 0; i < dir->size(); ++i) {
        if ((*dir)[i].isFile) {
            std::cout << tabs << "\t" << "-" << (*dir)[i].name
                      << " " << (*dir)[i].start_cluster << " " << ((*dir)[i].size / br->cluster_size + 1) << std::endl;
        } else {
            print_dir((*dir)[i].name, (*dir)[i].start_cluster, depth + 1);
        }
    }
    std::cout << tabs << "--" << std::endl;

}

int FileSystem::rm_dir(const std::string &path) {
    int32_t dir_cluster = cd(path);
    if (dir_cluster < 0) {
        return EXIT_PATH_NOT_FOUND;
    }

    auto dir = get_dir(dir_cluster);
    if (dir->size() != 0) {
        return EXIT_PATH_NOT_EMPTY;
    }

    std::string parent_path = path;
    parent_path.pop_back(); // umazeme posledni '/'
    std::string dir_name = (split(parent_path, '/').back()); // vemem nazev adresare

    auto parent_cluster = cd(parent_path);
    if (parent_cluster < 0) {
        return EXIT_PATH_NOT_FOUND;
    }
    auto parent_dir = get_dir(parent_cluster);

    for (int i = 0; i < parent_dir->size(); ++i) {
        if (dir_name.compare((*parent_dir)[i].name) == 0) {
            parent_dir->erase(parent_dir->begin() + i);
            write_dir(parent_cluster, parent_dir);

            // znulujem cluster s adresarem
            fseek(fs, cluster_to_addr(dir_cluster), SEEK_SET);
            fwrite(zero_cluster, br->cluster_size, 1, fs);

            fat[dir_cluster] = FAT_UNUSED;
            flush_fat();
            return EXIT_OK;
        }
    }
    return EXIT_PATH_NOT_FOUND;

}






