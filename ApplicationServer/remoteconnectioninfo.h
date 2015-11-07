#ifndef REMOTECONNECTIONINFO
#define REMOTECONNECTIONINFO

#define REMOTE_PORT 6120

namespace Remote {
    enum  Direction {
        Undefined,
        NorthWest = 1,
        North,
        NorthEast,
        West,
        East,
        SouthWest,
        South,
        SouthEast
    };
}


#endif // REMOTECONNECTIONINFO

