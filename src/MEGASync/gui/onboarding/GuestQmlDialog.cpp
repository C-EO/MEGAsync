#include "GuestQmlDialog.h"

#include "Platform.h"

GuestQmlDialog::GuestQmlDialog(QWindow *parent)
    : QmlDialog(parent)
{
    // Qt::Tool keeps the frameless guest popup out of the taskbar (it is a tray
    // popup, not a top-level window) while remaining interactive, unlike Qt::Popup.
    // Set here in the constructor, before the native window exists, to avoid the
    // window recreation that setFlags() triggers once the HWND is created.
    setFlags(flags() | Qt::FramelessWindowHint | Qt::Tool);

    QObject::connect(this, &GuestQmlDialog::activeChanged, [=]() {
        emit guestActiveChanged(this->isActive());
    });
}

bool GuestQmlDialog::isHiddenForLongTime() const
{
    return (QDateTime::currentMSecsSinceEpoch() - mLastHideTime) > 500;
}

void GuestQmlDialog::realocate()
{
    int posx, posy;
    Platform::getInstance()->calculateInfoDialogCoordinates(geometry(), &posx, &posy);
    setX(posx);
    setY(posy);
}

void GuestQmlDialog::showEvent(QShowEvent *event)
{
    realocate();
    QmlDialog::showEvent(event);

    emit initializePageFocus();
}

void GuestQmlDialog::hideEvent(QHideEvent *event)
{
    mLastHideTime = QDateTime::currentMSecsSinceEpoch();
    QmlDialog::hideEvent(event);

    emit hideRequested();
}
