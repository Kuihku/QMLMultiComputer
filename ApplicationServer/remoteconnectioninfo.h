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

#define REMOTETOSTRING(d) \
    d == Remote::NorthWest ? "NorthWest" : \
    d == Remote::North ? "North" : \
    d == Remote::NorthEast ? "NorthEast" : \
    d == Remote::West ? "West" : \
    d == Remote::East ? "East" : \
    d == Remote::SouthWest ? "SouthWest" : \
    d == Remote::South ? "South" : \
    d == Remote::SouthEast ? "SouthEast" : "Undefined"


#endif // REMOTECONNECTIONINFO

