#ifndef TYPES_H
#define TYPES_H

enum DocumentType
{
    DT_LOCAL = 0,
    DT_REMOTE,
    DT_CACHED
};

enum MoveEvent {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    TO_BEGIN,
    TO_END
};

enum class ItemType
{
    TODO = 0,
    LOG
};


#endif // TYPES_H
