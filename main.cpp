#include <iostream>
#include <vector>
#include "file_system.h"
#include "file_system_defragmenter.h"

using namespace std;

FileSystem *fs;


#define ARG_FAT_FILE 0
#define ARG_OP 1
#define ARG_OTHER_1 2
#define ARG_OTHER_2 3

void print_help() {
    std::cout << " zos  - pseudofat manager                                  " << std::endl ;
    std::cout << "        Usage : zos FAT_FILE OTHER_ARGS                    " << std::endl ;
    std::cout << " -h                                                        " << std::endl ;
    std::cout << "        Prints help                                        " << std::endl ;
    std::cout << " -a file full_path                                         " << std::endl ;
    std::cout << "        Write file to path                                 " << std::endl ;
    std::cout << " -f full_path                                              " << std::endl ;
    std::cout << "        Delete file                                        " << std::endl ;
    std::cout << " -c full_path                                              " << std::endl ;
    std::cout << "        List file clusters                                 " << std::endl ;
    std::cout << " -p                                                        " << std::endl ;
    std::cout << "        Generate tree of filesystem                        " << std::endl ;
    std::cout << " -l full_path                                              " << std::endl ;
    std::cout << "        Print file to screen                               " << std::endl ;
    std::cout << " -m dir_name full_path                                     " << std::endl ;
    std::cout << "        Creates directory in path                          " << std::endl ;
    std::cout << " -r full_path                                              " << std::endl ;
    std::cout << "        Remove empty directory                             " << std::endl ;
    std::cout << " -x cluster_count cluster_size                             " << std::endl ;
    std::cout << "        Create empty filesystem                            " << std::endl ;
    std::cout << " -d worker_threads_count                                   " << std::endl ;
    std::cout << "        Defragment filesystem                              " << std::endl ;
    std::cout << "                                                           " << std::endl ;
    std::cout << " Paths can start with / but if you specify root then"
              << " it is mandatory directory path always end with / rest"
              << " after that is omited.                                     " << std::endl ;
};


int main(int argc, char *argv[]) {

    if (argc<3) {
        cout << "Wrong number of arguments" << endl;
        print_help();
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

    if (args[ARG_OP] == "-m") {
        fs->mount(args[ARG_FAT_FILE]);
        e = fs->mk_dir(args[ARG_OTHER_1], args[ARG_OTHER_2]);
        fs->umount();
        FileSystem::print_code(e);
    }

    if (args[ARG_OP] == "-r") {
        fs->mount(args[ARG_FAT_FILE]);
        e = fs->rm_dir(args[ARG_OTHER_1]);
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

    if (args[ARG_OP] == "-d") {
        FileSystemDefragmenter *fs =  new FileSystemDefragmenter();
        fs->mount(args[ARG_FAT_FILE]);
        fs->print_blocks();
        int n = stoi(args[ARG_OTHER_1]);
        fs->defragment(n);
        fs->print_blocks();
        fs->umount();
        delete fs;
    }

    if (args[ARG_OP] == "-h") {
        print_help();
    }

    //vytvareci na testy
    if (args[ARG_OP].compare("-x") == 0) {
        fs->create(args[ARG_FAT_FILE], stoi(args[ARG_OTHER_1]), (int16_t) stoi(args[ARG_OTHER_2]), 2, "my fatsystem", "qwerty");
    }

    delete fs;

    return EXIT_SUCCESS;
}