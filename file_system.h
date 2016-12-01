#ifndef ZOS_FILE_SYSTEM_H
#define ZOS_FILE_SYSTEM_H

#include <string>

class FileSystem {
public:
    bool mount (std::string);
    bool umount ();

};

#endif //ZOS_FILE_SYSTEM_H
