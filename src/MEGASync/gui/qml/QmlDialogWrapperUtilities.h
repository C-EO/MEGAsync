#ifndef QML_DIALOG_WRAPPER_UTILITIES_H
#define QML_DIALOG_WRAPPER_UTILITIES_H

#include <QObject>
#include <QQuickWindow>
#include <QRect>
#include <QVariant>

namespace QmlDialogWrapperUtilities
{

constexpr const char* SHOW_WHEN_CREATED_PROPERTY = "ShowWhenCreated";

// A dialog is QML when its visible window is a QQuickWindow. The caller must
// obtain the window calling windowHandle() on the dialog's static type:
// QmlDialogWrapperBase::windowHandle() hides (does not override) the
// non-virtual QWidget::windowHandle(), and it is the one returning the inner
// QQuickWindow. For plain widget dialogs, windowHandle() is null or a plain
// QWindow, so the cast fails.
inline bool isQML(const QWindow* window)
{
    return qobject_cast<const QQuickWindow*>(window) != nullptr;
}

inline void setShowWhenCreated(QObject* dialog, bool showWhenCreated)
{
    if (dialog)
    {
        dialog->setProperty(SHOW_WHEN_CREATED_PROPERTY, showWhenCreated);
    }
}

inline bool isShowWhenCreated(const QObject* dialog)
{
    if (!dialog)
    {
        return false;
    }

    auto value(dialog->property(SHOW_WHEN_CREATED_PROPERTY));
    return value.isValid() && value.toBool();
}
}

#endif // QML_DIALOG_WRAPPER_UTILITIES_H
