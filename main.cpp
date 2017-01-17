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




    if (args[ARG_OP] == "-a") {
        fs->mount(argv[ARG_FAT_FILE]);
        //fs->upload_file()
        fs->umount();
    }

    if (args[ARG_OP].compare("-x") == 0) {
        fs->create(args[ARG_FAT_FILE], stoi(args[ARG_OTHER_1]), (int16_t) stoi(args[ARG_OTHER_2]), 2, "my fatsystem", "qwerty");
    }




    return EXIT_SUCCESS;
}