#ifndef QML_MANAGER_H
#define QML_MANAGER_H

#include <QQmlEngine>

#include <memory>

class QmlManager
{
public:
    static std::shared_ptr<QmlManager> instance();

    QmlManager(const QmlManager&) = delete;
    QmlManager& operator=(const QmlManager&) = delete;

    void finish();

    void setRootContextProperty(QObject* value);
    void setRootContextProperty(const QString& name, QObject* value);
    void setRootContextProperty(const QString& name, const QVariant& value);

    bool isRootContextPropertySet(QObject* value);

    // Derives the QML-side identifier from a QObject's class name
    // (e.g. "MessageDialogData" -> "messageDialogDataAccess").
    //
    // Exposed publicly (SNC-6567 Phase 1) so QmlDialogWrapper can register the
    // same name on a per-dialog child QQmlContext before qmlComponent.create()
    // runs. Both this getter and setRootContextProperty(QObject*) derive names
    // through this single helper, so the C++ side and the QML side agree on
    // identifiers without anyone having to spell them as string literals.
    //
    // Pure function, no state mutation.
    QString getObjectRootContextName(QObject* value);

    void addImageProvider(const QString& id, QQmlImageProviderBase*);
    void removeImageProvider(const QString& id);

    void retranslate();

    QQmlEngine* getEngine();

private:
    QQmlEngine* mEngine;

    QmlManager();
    void registerCommonQmlElements();
    void createPlatformSelectorsFlags();
};

#endif // QML_MANAGER_H
