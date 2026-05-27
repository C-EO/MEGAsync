#ifndef DIALOGOPENER_H
#define DIALOGOPENER_H

#include "HighDpiResize.h"
#include "megaapi.h"
#include "Platform.h"
#include "QmlDialogWrapperUtilities.h"
#include "TokenParserWidgetManager.h"

#include <QApplication>
#include <QDialog>
#include <QMap>
#include <QMessageBox>
#include <QPointer>
#include <QQueue>
#include <QRect>
#include <QWindow>

#include <functional>
#include <memory>

template<typename T>
class QmlDialogWrapper;

class MessageDialogComponent;
class MessageDialogData;

#ifdef Q_OS_WINDOWS
class ExternalDialogOpener : public QWidget
{
    Q_OBJECT

public:
    ExternalDialogOpener();
    ~ExternalDialogOpener();
};
#endif

class DialogBlocker : public QDialog
{
    Q_OBJECT

public:
    DialogBlocker(QWidget* parent);
    ~DialogBlocker();
};

class DialogOpener
{
private:
    class DialogInfoBase
    {
    public:
        DialogInfoBase() = default;
        virtual ~DialogInfoBase() = default;

        QString getDialogClass() const {return mDialogClass;}
        void setDialogClass(const QString &newDialogClass) {mDialogClass = newDialogClass;}

        virtual bool sameDialog(QObject* check) const = 0;
        virtual void raise(bool raiseIfMinimized = false) = 0;
        virtual void show() = 0;
        virtual void close() = 0;
        virtual bool isVisible() = 0;
        virtual bool isActive() = 0;
        virtual bool isParent(QObject* parent) = 0;
        virtual QRect frameGeometry() const = 0;
        virtual void applyCurrentTheme() = 0;

        bool ignoreCloseAllAction() const {return mIgnoreCloseAllAction;}
        void setIgnoreCloseAllAction(bool newIgnoreCloseAllAction){mIgnoreCloseAllAction = newIgnoreCloseAllAction;}

        bool ignoreRaiseAllAction() const {return mIgnoreRaiseAllAction;}
        void setIgnoreRaiseAllAction(bool newIgnoreRaiseAllAction){mIgnoreRaiseAllAction = newIgnoreRaiseAllAction;}

    protected:
        QString mDialogClass;
        bool mIgnoreCloseAllAction = false;
        bool mIgnoreRaiseAllAction = false;
    };

    template <class DialogType>
    class DialogInfo : public DialogInfoBase
    {
    public:
        QPointer<DialogType> getDialog() const { return mDialog;}
        void setDialog(QPointer<DialogType> newDialog)
        {
            mDialog = newDialog;
        }

        bool sameDialog(QObject* check) const override
        {
            return mDialog == check;
        }

        void raise(bool raiseIfMinimized = false) override
        {
            if(raiseIfMinimized && mDialog->isMinimized())
            {
                mDialog->showNormal();
            }

            if(!mDialog->isMinimized())
            {
                mDialog->raise();
                mDialog->activateWindow();
            }
        }

        void show() override
        {
            mDialog->setWindowState(Qt::WindowActive);
        }

        bool isVisible() override
        {
            return mDialog->isVisible();
        }

        bool isActive() override
        {
            return mDialog->isActiveWindow();
        }
        void close() override
        {
            Platform::getInstance()->closeFileFolderSelectors(mDialog);
            mDialog->close();
        }

        void clear()
        {
            mDialogClass.clear();
            DialogOpener::removeDialog<DialogType>(mDialog);
        }

        bool isParent(QObject* parent) override
        {
            return mDialog->parent() == parent;
        }

        QRect frameGeometry() const override
        {
            return mDialog->frameGeometry();
        }

        bool operator==(const DialogInfo &info)
        {
            return (info.mDialog == mDialog);
        }

        void applyCurrentTheme() override
        {
            Platform::getInstance()->applyCurrentThemeOnCurrentDialogFrame(mDialog->windowHandle());
        }

    private:
        QPointer<DialogType> mDialog;
    };

    struct GeometryInfo
    {
        bool maximized = false;
        QRect geometry;
        bool isEmpty() const {return geometry.isEmpty();}
    };

public:
    static QPoint initialDialogPosition(const QSize& dialogSize)
    {
        return Platform::getInstance()->initialDialogPosition(dialogSize);
    }

    static QPoint initialDialogPosition(const QSize& dialogSize, const QRect& parentGeometry)
    {
        return Platform::getInstance()->initialDialogPosition(dialogSize, parentGeometry);
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findDialog()
    {
        auto classType = className<DialogType>();

        auto finder = [classType](const std::shared_ptr<DialogInfoBase>& dialogInfo) {
            return (dialogInfo->getDialogClass() == classType);
        };

        auto itOccurence = std::find_if(mOpenedDialogs.begin(), mOpenedDialogs.end(), finder);
        if (itOccurence != mOpenedDialogs.end())
        {
            return std::dynamic_pointer_cast<DialogInfo<DialogType>>(*itOccurence);
        }

        return nullptr;
    }

    template <class DialogType>
    static void removeDialogByClass()
    {
        auto dialogInfo = findDialog<DialogType>();
        if(dialogInfo)
        {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);
            if(dialogInfoByType)
            {
                removeDialog(dialogInfoByType->getDialog());
            }
        }
    }

    typedef QmlDialogWrapper<MessageDialogComponent> QmlMessageDialogWrapper;
    static void showMessageDialog(QPointer<QmlMessageDialogWrapper> wrapper,
                                  QPointer<MessageDialogData> msgInfo);

    template <class ParentType>
    static void closeDialogsByParentClass()
    {
        auto parentDialog = findDialog<ParentType>();
        if(parentDialog)
        {
            Platform::getInstance()->closeFileFolderSelectors(parentDialog->getDialog());

            foreach(auto dialogInfo, mOpenedDialogs)
            {
                if(dialogInfo->isParent(parentDialog->getDialog()))
                {
                    dialogInfo->close();
                }
            }
        }
    }

    template<class DialogType>
    static void closeDialogsByClass()
    {
        auto dialog = findDialog<DialogType>();
        if (dialog)
        {
            dialog->close();
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }

    template <class DialogType, class ParentType>
    static void setParent(QPointer<DialogType> dialog, bool whenParentIsActivated)
    {
        auto parentDialog = findDialog<ParentType>();
        if(parentDialog && ((whenParentIsActivated && parentDialog->getDialog()->isActiveWindow())
                            || (!whenParentIsActivated && !parentDialog->getDialog()->isMinimized())))
        {
            closeDialogsByParentClass<ParentType>();

            parentDialog->raise(true);
            dialog->setParent(parentDialog->getDialog(), dialog->windowFlags());
        }
    }

    template <class DialogType, class ParentType>
    static void showDialog(QPointer<DialogType> dialog, bool whenParentIsActivated, std::function<void()> func)
    {
        if(dialog)
        {
            setParent<DialogType, ParentType>(dialog,whenParentIsActivated);
            showDialog<DialogType>(dialog, func);
        }
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> showDialog(QPointer<DialogType> dialog, std::function<void()> func)
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &DialogType::finished, [func, dialog](){
                removeDialog(dialog);
                if(func)
                {
                    func();
                }
            });
            return showDialogImpl(dialog);
        }
        return nullptr;
    }

    template <class DialogType, class ParentType, class CallbackClass>
    static std::shared_ptr<DialogInfo<DialogType>> showDialog(QPointer<DialogType> dialog, bool whenParentIsActivated, CallbackClass* caller, void(CallbackClass::*func)(QPointer<DialogType>))
    {
        if(dialog)
        {
            setParent<DialogType, ParentType>(dialog, whenParentIsActivated);

            dialog->connect(dialog.data(), &DialogType::finished, [dialog, caller, func](){
                removeDialog(dialog);
                if(caller && func)
                {
                    (caller->*func)(dialog);
                }
            });

            return showDialogImpl(dialog);
        }
        return nullptr;
    }

    template <class DialogType, class CallbackClass>
    static std::shared_ptr<DialogInfo<DialogType>> showDialog(QPointer<DialogType> dialog, CallbackClass* caller, void(CallbackClass::*func)(QPointer<DialogType>))
    {
        if(dialog)
        {
            dialog->connect(dialog.data(), &DialogType::finished, [dialog, caller, func](){
                if(caller && func)
                {
                    (caller->*func)(dialog);
                }
                removeDialog(dialog);
            });

            return showDialogImpl(dialog);
        }
        return nullptr;
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> showNonModalDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            removeWhenClose(dialog);
            dialog->setModal(false);
            return showDialogImpl(dialog, false);
        }
        return nullptr;
    }

    template <class DialogType>
    static void showGeometryRetainerDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            removeWhenClose(dialog);
            showDialogImpl(dialog, false);
            auto classType = className<DialogType>();
            mSavedGeometries.insert(classType, GeometryInfo());
        }
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> showDialog(QPointer<DialogType> dialog)
    {
        std::shared_ptr<DialogInfo<DialogType>> ret = nullptr;
        if(dialog)
        {
            removeWhenClose(dialog);
            ret = showDialogImpl(dialog);
        }
        return ret;
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> addDialog(QPointer<DialogType> dialog)
    {
        std::shared_ptr<DialogInfo<DialogType>> ret = nullptr;

        if(dialog)
        {
            QString classType = className<DialogType>();
            auto info = findSiblingDialogInfo<DialogType>(classType);

            if(!info)
            {
                info = std::make_shared<DialogInfo<DialogType>>();
                info->setDialog(dialog);
                info->setDialogClass(classType);
                mOpenedDialogs.append(info);
                initDialog(dialog);
                removeWhenClose(dialog);
            }
            ret = info;
        }
        return ret;
    }

    template <class DialogType>
    static void removeDialog(QPointer<DialogType> dialog)
    {
        if(dialog)
        {
            auto classType = className<DialogType>();
            if(mSavedGeometries.contains(classType))
            {
                GeometryInfo info;
                info.maximized = dialog->isMaximized();
                info.geometry = dialog->geometry();
                mSavedGeometries.insert(classType, info);
            }

            if(dialog->parent())
            {
                dialog->parentWidget()->activateWindow();
            }

            dialog->deleteLater();
        }
    }

    static void raiseAllDialogs()
    {
        bool anyRaised = false;
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(!dialogInfo->ignoreRaiseAllAction())
            {
                dialogInfo->raise();
                anyRaised = true;
            }
        }

        if(anyRaised)
        {
            qApp->processEvents();
        }
    }

    static void closeAllDialogs()
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            if(!dialogInfo->ignoreCloseAllAction())
            {
                dialogInfo->close();
            }
        }
    }

    static void currentThemeChanged()
    {
        foreach(auto dialogInfo, mOpenedDialogs)
        {
            dialogInfo->applyCurrentTheme();
        }
    }

    static bool anyVisibleAndActiveDialogs()
    {
        return std::any_of(mOpenedDialogs.begin(),
                           mOpenedDialogs.end(),
                           [](std::shared_ptr<DialogInfoBase> dialogInfo)
                           {
                               return dialogInfo->isVisible() && dialogInfo->isActive();
                           });
    }

    static bool isAnyDialogVisible()
    {
        return std::any_of(mOpenedDialogs.begin(),
                           mOpenedDialogs.end(),
                           [](std::shared_ptr<DialogInfoBase> dialogInfo)
                           {
                               return dialogInfo->isVisible();
                           });
    }

    static QList<QPointer<QWidget>> getAllOpenedDialogs();

private:
    static QList<std::shared_ptr<DialogInfoBase>> mOpenedDialogs;
    static QQueue<std::shared_ptr<DialogInfoBase>> mDialogsQueue;
    static QMap<QString, GeometryInfo> mSavedGeometries;

    template <class DialogType>
    static void removeWhenClose(QPointer<DialogType> dialog)
    {
        dialog->connect(dialog.data(), &DialogType::finished, [dialog]()
        {
            removeDialog(dialog);
        });
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> showDialogImpl(QPointer<DialogType> dialog, bool changeWindowModality = true, bool removeSiblings = true)
    {
        if(dialog)
        {
            QString classType = className<DialogType>();
            auto info = findSiblingDialogInfo<DialogType>(classType);

            bool isQML(QmlDialogWrapperUtilities::isQML(dialog));

            bool ignoreGeometry(isQML && QmlDialogWrapperUtilities::isShowWhenCreated(dialog));
            QRect geometry;
            QByteArray siblingGeometryState;

            if(info)
            {
                if(removeSiblings && info->getDialog() != dialog)
                {
                    geometry = info->getDialog()->geometry();
                    siblingGeometryState = info->getDialog()->saveGeometry();
                    dialog->setWindowFlags(info->getDialog()->windowFlags());
                    removeDialog(info->getDialog());
                    info->setDialog(dialog);
                    info->setDialogClass(classType);

                    initDialog(dialog);
                }
                else if (info->getDialog() == dialog)
                {
                    ignoreGeometry = true;
                }
            }
            else
            {
                info = std::make_shared<DialogInfo<DialogType>>();
                info->setDialog(dialog);
                info->setDialogClass(classType);
                mOpenedDialogs.append(info);

                initDialog(dialog);
            }

#ifdef Q_OS_WINDOWS
            ExternalDialogOpener externalOpener;
#endif

            if (!dialog)
            {
                ::mega::MegaApi::log(
                    ::mega::MegaApi::LOG_LEVEL_ERROR,
                    QString::fromUtf8("DialogOpener: Dialog %1 removed while being opened.")
                        .arg(classType)
                        .toUtf8()
                        .constData());

                return nullptr;
            }

            if (!isQML)
            {
                TokenParserWidgetManager::instance()->applyCurrentTheme(dialog);
            }

            // Use to reload the widget stylesheet. Without this line, the new stylesheet is not
            // correctly applied.
            dialog->setParent(dialog->parentWidget(), dialog->windowFlags());

            if (dialog->parent() && changeWindowModality)
            {
                dialog->setWindowModality(Qt::WindowModal);
            }

            // For QML dialog wrappers, the setParent() above can recreate the
            // native handle and drop the inner QQuickWindow's transient parent
            // binding. Re-bind it via the public slot.
            if (isQML && dialog && dialog->parent())
            {
                QMetaObject::invokeMethod(dialog, "attachQmlToParentWindow", Qt::DirectConnection);
            }

            if (ignoreGeometry)
            {
                dialog->show();
            }
            else
            {
                QRect parentGeo;
                // QML dialogs with parent are centered on its parent
                if (isQML)
                {
                    parentGeo = QmlDialogWrapperUtilities::getParentGeometry(dialog);
                }

                if (parentGeo.isValid())
                {
                    dialog->move(initialDialogPosition(dialog->geometry().size(), parentGeo));
                    dialog->show();
                }
                else
                {
                    auto savedGeo = mSavedGeometries.value(classType, GeometryInfo());
                    if (!savedGeo.isEmpty())
                    {
                        geometry = savedGeo.geometry;
                    }

                    if (!dialog->isVisible())
                    {
                        if (!siblingGeometryState.isEmpty())
                        {
                            // Reopening over a still-open sibling: restoreGeometry
                            // round-trips the frame position exactly on every platform,
                            // avoiding the upward drift that setGeometry() causes on macOS.
                            dialog->restoreGeometry(siblingGeometryState);
                            dialog->show();
                        }
                        else if (geometry.isValid())
                        {
                            // First time this is used
                            if (savedGeo.maximized)
                            {
                                dialog->showMaximized();
                            }
                            else
                            {
                                dialog->setGeometry(geometry);
                                dialog->show();
                            }
                        }
                        else
                        {
                            auto pos(initialDialogPosition(dialog->geometry().size()));
                            auto size(dialog->geometry().size());
                            dialog->setGeometry(
                                QRect(pos.x(), pos.y(), size.width(), size.height()));
                            dialog->show();
                        }
                    }
                }
            }

            if (dialog->parent())
            {
                auto parentInfo = findDialogInfo(dialog->parent());
                if (parentInfo)
                {
                    parentInfo->raise(true);
                }
            }

            info->raise(true);

            Platform::getInstance()->applyCurrentThemeOnCurrentDialogFrame(dialog->windowHandle());

            return info;
        }

        return nullptr;
    }

    template <class DialogType>
    static void initDialog(QPointer<DialogType> dialog)
    {
        dialog->connect(dialog.data(), &QObject::destroyed, [dialog](){
            auto info = findDialogInfo<DialogType>(dialog);
            if(info)
            {
                mOpenedDialogs.removeOne(info);
            }
        });
        auto dpiResize = new HighDpiResize<DialogType>(dialog);
        Q_UNUSED(dpiResize);
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findDialogInfo(QPointer<DialogType> dialog)
    {
        auto finder = [dialog](const std::shared_ptr<DialogInfoBase> dialogInfo) {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);

            return (dialogInfoByType && dialogInfoByType->getDialog() == dialog);
        };

        auto itOccurence = std::find_if(mOpenedDialogs.begin(), mOpenedDialogs.end(), finder);
        if (itOccurence != mOpenedDialogs.end())
        {
            return std::dynamic_pointer_cast<DialogInfo<DialogType>>(*itOccurence);
        }

        return nullptr;
    }

    static std::shared_ptr<DialogInfoBase> findDialogInfo(QObject* dialog)
    {
        auto finder = [dialog](const std::shared_ptr<DialogInfoBase> dialogInfo)
        {
            return dialogInfo->sameDialog(dialog);
        };

        auto itOccurence = std::find_if(mOpenedDialogs.begin(), mOpenedDialogs.end(), finder);
        if (itOccurence != mOpenedDialogs.end())
        {
            return (*itOccurence);
        }

        return nullptr;
    }

    template <class DialogType>
    static std::shared_ptr<DialogInfo<DialogType>> findSiblingDialogInfo(const QString& classType)
    {
        auto finder = [classType](const std::shared_ptr<DialogInfoBase> dialogInfo) {
            auto dialogInfoByType = std::dynamic_pointer_cast<DialogInfo<DialogType>>(dialogInfo);

            if(dialogInfoByType && !dialogInfoByType->getDialogClass().isEmpty())
            {
                return (dialogInfoByType->getDialogClass()  == classType);
            }

            return false;
        };

        auto itOccurence = std::find_if(mOpenedDialogs.begin(), mOpenedDialogs.end(), finder);
        if (itOccurence != mOpenedDialogs.end())
        {
            return std::dynamic_pointer_cast<DialogInfo<DialogType>>(*itOccurence);
        }

        return nullptr;
    }

    template <class DialogType>
    static QString className()
    {
        return QString::fromUtf8(typeid(DialogType).name());
    }
};

#endif // DIALOGOPENER_H
