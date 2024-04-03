#ifndef QML_COLOR_THEME_MANAGER_TARGET_H
#define QML_COLOR_THEME_MANAGER_TARGET_H

#include "IQMLThemeTarget.h"

#include <QString>

namespace DTI
{
    class QMLColorThemeManagerTarget : public IQMLThemeTarget
    {
    public:
        void deploy(const ThemedColourMap& themedColourMap) const override;

    private:
        static bool registered;

        bool checkThemeData(const ThemedColourMap& themeData) const;
    };
}

#endif
