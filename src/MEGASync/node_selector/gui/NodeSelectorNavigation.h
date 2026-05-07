#ifndef NODESELECTORNAVIGATION_H
#define NODESELECTORNAVIGATION_H

#include "megaapi.h"

#include <QList>

#include <optional>

class NodeSelectorNavigation
{
public:
    explicit NodeSelectorNavigation(mega::MegaApi* megaApi);

    bool canGoBack() const;
    bool canGoForward() const;
    bool shouldShowNavigationButtons() const;

    std::optional<mega::MegaHandle> goBack(mega::MegaHandle currentRootHandle);
    std::optional<mega::MegaHandle> goForward(mega::MegaHandle currentRootHandle);

    void onNavigateInto(mega::MegaHandle currentRootHandle, mega::MegaHandle targetHandle);
    void onRootChanged(mega::MegaHandle rootHandle);
    void onHandleRemoved(mega::MegaHandle handle);

    bool hasBackwardHandle(mega::MegaHandle handle) const;
    void clearBackward();
    void clear();

private:
    void removeFromForward(mega::MegaHandle handle);
    void remove(mega::MegaHandle handle);
    void appendToBackward(mega::MegaHandle handle);
    void appendToForward(mega::MegaHandle handle);

    mega::MegaApi* mMegaApi;
    QList<mega::MegaHandle> mForwardHandles;
    QList<mega::MegaHandle> mBackwardHandles;
};

#endif // NODESELECTORNAVIGATION_H
