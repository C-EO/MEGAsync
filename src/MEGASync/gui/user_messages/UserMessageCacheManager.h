#ifndef USER_MESSAGE_CACHE_MANAGER_H
#define USER_MESSAGE_CACHE_MANAGER_H

#include <QWidget>
#include <QCache>

class UserMessage;
class UserMessageWidget;

class UserMessageCacheManager
{

public:
    UserMessageCacheManager();
    virtual ~UserMessageCacheManager() = default;

    UserMessageWidget* createOrGetWidget(qsizetype row, UserMessage* data, QWidget* parent);

private:
    QCache<qsizetype, UserMessageWidget> mUserMessageItems;

    template<class Item>
    UserMessageWidget* createOrGetWidget(qsizetype cacheIndex, UserMessage* data, QWidget* parent);

    UserMessageWidget* getWidgetFromCache(qsizetype cacheIndex);
};

#endif // USER_MESSAGE_CACHE_MANAGER_H
