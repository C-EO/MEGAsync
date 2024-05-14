#ifndef WIDGETS_DESIGN_TARGET_H
#define WIDGETS_DESIGN_TARGET_H

#include "Types.h"

#include <QString>

namespace  DTI
{
    class IWidgetsDesignTarget
    {
    public:
        virtual void deploy(const DesignAssets& designAssets) const = 0;
        virtual ~IWidgetsDesignTarget() = default;
    };
}

#endif
