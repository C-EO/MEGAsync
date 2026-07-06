#ifndef ILOADINGVIEWMODEL_H
#define ILOADINGVIEWMODEL_H

// Interface for models attached to views managed by ViewLoadingScene.
// isWorking() must return true while the model is doing work in another thread;
// the loading scene refuses to hide while it returns true, and the model is
// responsible for requesting the hide again when that work finishes.
class ILoadingViewModel
{
public:
    virtual ~ILoadingViewModel() = default;

    virtual bool isWorking() const = 0;
};

#endif // ILOADINGVIEWMODEL_H
