#include "DialogOpener.h"

#include "MessageDialogComponent.h"
#include "MessageDialogData.h"
#include "QmlDialogWrapper.h"

#include <QApplication>
#include <QOperatingSystemVersion>
#include <QQuickWindow>
#include <QTimer>

QList<std::shared_ptr<DialogOpener::DialogInfoBase>> DialogOpener::mOpenedDialogs = QList<std::shared_ptr<DialogOpener::DialogInfoBase>>();
QQueue<std::shared_ptr<DialogOpener::DialogInfoBase>> DialogOpener::mDialogsQueue = QQueue<std::shared_ptr<DialogOpener::DialogInfoBase>>();
QMap<QString, DialogOpener::GeometryInfo> DialogOpener::mSavedGeometries = QMap<QString, DialogOpener::GeometryInfo>();


#ifdef Q_OS_WINDOWS
ExternalDialogOpener::ExternalDialogOpener()
    : QWidget(nullptr, Qt::SubWindow)
{
    if(QOperatingSystemVersion::current() <= QOperatingSystemVersion::Windows10)
    {
        setAttribute(Qt::WA_DeleteOnClose, true);
        setWindowFlag(Qt::WindowStaysOnBottomHint, true);
        setWindowFlag(Qt::FramelessWindowHint, true);
        setFixedSize(0,0);
        show();
        raise();
        activateWindow();
    }
}

ExternalDialogOpener::~ExternalDialogOpener()
{
    close();
}
#endif

DialogBlocker::DialogBlocker(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setGeometry(QRect(1,1,1,1));
    open();
}

DialogBlocker::~DialogBlocker()
{
    close();
}

void DialogOpener::showMessageDialog(QPointer<QmlMessageDialogWrapper> wrapper,
                                     QPointer<MessageDialogData> msgInfo)
{
    if (wrapper)
    {
        auto showWrapper = [](QPointer<QmlMessageDialogWrapper> dialog, bool ignoreCloseAll)
        {
#ifdef Q_OS_MACOS
            // If the message dialog is not WindowModal, the parent is not greyed out.
            // So, we use a dummy dialog WindowModal
            if (dialog && dialog->parentWidget())
            {
                auto blocker = new DialogBlocker(dialog->parentWidget());
                dialog->connect(dialog.data(),
                                &QmlDialogWrapperBase::finished,
                                blocker,
                                &QObject::deleteLater);
                qApp->setActiveWindow(dialog);
            }
#endif

            if (!dialog)
            {
                return;
            }

            dialog->setWindowModality(Qt::ApplicationModal);
            auto dialogInfo = showDialogImpl(dialog, false, false);
            if (dialogInfo)
            {
                if (QmlDialogWrapperUtilities::isShowWhenCreated(dialog))
                {
                    emit dialog->wrapper()->dataReady();
                }
                dialogInfo->setIgnoreCloseAllAction(ignoreCloseAll);
            }
        };

        wrapper->connect(wrapper.data(),
                         &QmlDialogWrapperBase::finished,
                         [msgInfo, wrapper, showWrapper]()
                         {
                             if (msgInfo->getFinishFunction())
                             {
                                 msgInfo->getFinishFunction()(msgInfo->result());
                             }

                             removeDialog(wrapper);
                             if (!mDialogsQueue.isEmpty())
                             {
                                 auto queueMsgBox =
                                     std::dynamic_pointer_cast<DialogInfo<QmlMessageDialogWrapper>>(
                                         mDialogsQueue.dequeue());
                                 if (queueMsgBox)
                                 {
                                     showWrapper(queueMsgBox->getDialog(),
                                                 queueMsgBox->ignoreCloseAllAction());
                                 }
                             }
                         });

        QString classType = className<QmlMessageDialogWrapper>();
        auto siblingDialogInfo = findSiblingDialogInfo<QmlMessageDialogWrapper>(classType);
        if (siblingDialogInfo && msgInfo->enqueue())
        {
            auto info = std::make_shared<DialogInfo<QmlMessageDialogWrapper>>();
            info->setDialog(wrapper);
            info->setDialogClass(classType);
            info->setIgnoreCloseAllAction(msgInfo->ignoreCloseAll());
            mDialogsQueue.enqueue(info);
        }
        else
        {
            showWrapper(wrapper, msgInfo->ignoreCloseAll());
        }
    }
}

QList<QPointer<QWidget>> DialogOpener::getAllOpenedDialogs()
{
    QList<QPointer<QWidget>> dialogs;

    foreach (const auto& dialogInfo, mOpenedDialogs)
    {
        auto dialogPtr = std::static_pointer_cast<DialogInfo<QWidget>>(dialogInfo);
        if (dialogPtr)
        {
            auto dialog = dialogPtr->getDialog();
            if (dialog)
            {
                dialogs.append(dialog);
            }
        }
    }

    return dialogs;
}

void DialogOpener::refreshOtherQmlWindows(QWindow* excludedWindow)
{
    const auto dialogs = getAllOpenedDialogs();
    for (const auto& dialog: dialogs)
    {
        // Only QML dialogs: their visible window is the inner QQuickWindow.
        auto* wrapper = qobject_cast<QmlDialogWrapperBase*>(dialog.data());
        if (!wrapper)
        {
            continue;
        }

        QQuickWindow* quickWindow = wrapper->getQmlWindow();
        if (!quickWindow || quickWindow == excludedWindow || !quickWindow->isVisible())
        {
            continue;
        }

        // Repaint now (covers the excluded window being hidden) and once
        // more on the next event loop iteration (covers the destruction
        // of its native surface, which happens after this call).
        quickWindow->update();
        QTimer::singleShot(0,
                           quickWindow,
                           [quickWindow]()
                           {
                               quickWindow->update();
                           });
    }
}
