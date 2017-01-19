#include <iostream>
#include <vector>
#include "file_system.h"
using namespace std;

FileSystem *fs;


#define ARG_FAT_FILE 0
#define ARG_OP 1
#define ARG_OTHER_1 2
#define ARG_OTHER_2 3




int main(int argc, char *argv[]) {

    if (argc<3) {
        cout << "Wrong number of arguments" << endl;
        return EXIT_FAILURE;
    }

    vector<string> args(argv + 1, argv + argc); // prevedem parametry na neco normalniho
    fs = new FileSystem();
    int e = FileSystem::EXIT_OK;

    if (args[ARG_OP] == "-a") {
        fs->mount(args[ARG_FAT_FILE]);
        e = fs->put_file(args[ARG_OTHER_1],args[ARG_OTHER_2]);
        fs->umount();
        FileSystem::print_code(e);
    }

    if (args[ARG_OP] == "-f") {
        fs->mount(args[ARG_FAT_FILE]);
        e = fs->del_file(args[ARG_OTHER_1]);
        fs->umount();
        fs->print_code(e);
    }

    if (args[ARG_OP] == "-c") {
        fs->mount(args[ARG_FAT_FILE]);
        auto list = fs->list_of_clusters_of_file(args[ARG_OTHER_1]);
        fs->umount();
        if (list.size()==0) {
            FileSystem::print_code(FileSystem::EXIT_PATH_NOT_FOUND);
        } else {
            cout << args[ARG_FAT_FILE] << " " << list << endl;
        }
    }

    if (args[ARG_OP] == "-f") {
        fs->mount(args[ARG_FAT_FILE]);
        e = fs->del_file(args[ARG_OTHER_1]);
        fs->umount();
        FileSystem::print_code(e);
    }

    if (args[ARG_OP] == "-m") {
        fs->mount(args[ARG_FAT_FILE]);
        e = fs->mk_dir(args[ARG_OTHER_1], args[ARG_OTHER_2]);
        fs->umount();
        FileSystem::print_code(e);
    }



    if (args[ARG_OP] == "-l") {
        fs->mount(args[ARG_FAT_FILE]);
        fs->print_file(args[ARG_OTHER_1]);
        fs->umount();
    }

    if (args[ARG_OP] == "-p") {
        fs->mount(args[ARG_FAT_FILE]);
        fs->print_tree();
        fs->umount();
    }


    //vytvareci na tesy
    if (args[ARG_OP].compare("-x") == 0) {
        fs->create(args[ARG_FAT_FILE], stoi(args[ARG_OTHER_1]), (int16_t) stoi(args[ARG_OTHER_2]), 2, "my fatsystem", "qwerty");
    }

    return EXIT_SUCCESS;
}