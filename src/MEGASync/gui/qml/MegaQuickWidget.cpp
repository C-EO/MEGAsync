#include "MegaQuickWidget.h"

#include "QmlManager.h"
#include "ThemeManager.h"

#include <QCoreApplication>
#include <QStyle>
#include <QUrl>

MegaQuickWidget::MegaQuickWidget(QWidget* parent):
    QQuickWidget(QmlManager::instance()->getEngine(), parent)
{
    // This allows for rendering of edges properly, but will no qt widgets can be drawn on top of
    // this
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setClearColor(Qt::transparent);
}

MegaQuickWidget::~MegaQuickWidget()
{
    // QQmlDelegateModel destroys released delegates with deleteLater()
    // (QQmlDelegateModelItem::destroyObject). If those deferred deletions are
    // processed after the base ~QQuickWidget has destroyed the offscreen
    // window, tearing a delegate down dereferences the freed window in
    // QQuickItemPrivate::addToDirtyList and crashes. Destroy the QML content
    // now, while the offscreen window is still alive, and flush the pending
    // DeferredDelete events so those delegates are torn down safely.
    setSource(QUrl());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

bool MegaQuickWidget::event(QEvent* event)
{
    if (event->type() == ThemeManager::ThemeChanged)
    {
        this->style()->unpolish(this);
        this->style()->polish(this);
    }
    return QQuickWidget::event(event);
}
