#pragma once

#include "clibssh.h"


namespace ssh
{

class LibSSHInitializer
{
public:
    LibSSHInitializer(const LibSSHInitializer &) = delete;
    LibSSHInitializer & operator=(const LibSSHInitializer &) = delete;

    static LibSSHInitializer & instance()
    {
        static LibSSHInitializer _instance;
        return _instance;
    }

    ~LibSSHInitializer();

private:
    LibSSHInitializer(); // NOLINT
};

}
