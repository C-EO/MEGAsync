#include "SyncsQmlDialog.h"

#include "MegaApplication.h"
#include "SyncInfo.h"

#include <QEvent>

bool SyncsQmlDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close || event->type() == QEvent::Show)
    {
        if (!isBackup() && event->type() == QEvent::Close)
        {
            emit MegaSyncApp->syncsDialogClosed();
        }
    }

    return QmlDialog::event(event);
}

bool SyncsQmlDialog::isBackup() const
{
    return mIsBackup;
}

void SyncsQmlDialog::setIsBackup(bool newIsBackup)
{
    mIsBackup = newIsBackup;
}
