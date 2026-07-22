#ifndef NODESELECTORBREADCRUMBSEGMENT_H
#define NODESELECTORBREADCRUMBSEGMENT_H

#include "megaapi.h"

#include <QString>

struct NodeSelectorBreadcrumbSegment
{
    mega::MegaHandle handle = mega::INVALID_HANDLE;
    QString text;

    bool operator==(const NodeSelectorBreadcrumbSegment& other) const
    {
        return handle == other.handle && text == other.text;
    }

    bool operator!=(const NodeSelectorBreadcrumbSegment& other) const
    {
        return !(*this == other);
    }
};

#endif // NODESELECTORBREADCRUMBSEGMENT_H
