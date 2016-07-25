#ifndef NETWORK_DEF_H
#define NETWORK_DEF_H

#include "inttypes.h"

#define DOCUMENT_NAME_MAX_LENGHT 100

enum PACKET_TYPE
{
    PT_ITEM_CREATED = 0,
    PT_ITEM_DELETED,
    PT_ITEM_CHANGED,

    PT_GET_ALL_ITEMS,
    PT_GET_DOC_LIST,
    PT_GET_ITEM,
    PT_GET_CHILDREN,

    PT_OPERATION_CONFIRMED
};

struct NetworkHeader
{
    uint64_t requestID;
    uint16_t type;

    uint64_t docId;
    uint64_t itemId;
    uint64_t parentId;
    uint32_t dataSize;
} __attribute__((packed));

struct DocumentDescriptor
{
    uint64_t id;
    uint8_t name[DOCUMENT_NAME_MAX_LENGHT];
} __attribute__((packed));

#define DEBUG_PORT 12054

//struct PT

#endif // NETWORK_DEF_H
