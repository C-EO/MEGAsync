#ifndef QML_DIALOG_H
#define QML_DIALOG_H

#include <QPointer>
#include <QQuickWindow>
#include <QTimer>

// SNC-6567 (Phase 4): The previous QmlInstancesManager* property is removed.
// Data is now exposed to QML directly as child QQmlContext properties
// registered by QmlDialogWrapper before qmlComponent.create() (Phase 1).
class QmlDialog: public QQuickWindow
{
    Q_OBJECT

    Q_PROPERTY(QString iconSrc MEMBER mIconSrc WRITE setIconSrc)
    Q_PROPERTY(bool closeOnEscapePressed READ getCloseOnEscapePressed WRITE setCloseOnEscapePressed)
    Q_PROPERTY(
        bool initialLayoutComplete READ initialLayoutComplete NOTIFY initialLayoutCompleteChanged)

public:
    explicit QmlDialog(QWindow* parent = nullptr);
    ~QmlDialog();

public slots:
    void setIconSrc(const QString& iconSrc);
    void readyToBeShow();
    bool getCloseOnEscapePressed() const;
    void setCloseOnEscapePressed(bool active);
    bool initialLayoutComplete() const;

    // Attach this QML window to its parent's native window so the OS treats it
    // as an embedded modal dialog (centered over parent, blocks parent input,
    // not resizable as a standalone top-level window).
    // Safe to call multiple times; idempotent on the same parent.
    void attachToParentWindow(QWindow* parentWindow, bool embedded = true);

signals:
    void accept();
    void reject();
    void finished();
    void accepted();
    void rejected();
    void requestPageFocus();
    void initializePageFocus();
    void closeOnEscapePressedChanged();
    void initialLayoutCompleteChanged();

protected:
    bool event(QEvent* event) override;
    void onRequestPageFocus();

private:
    void placeAndRaise();
    void refreshOtherQmlWindows();

    QString mIconSrc;
    bool mCloseOnEscapePressed = false;
    bool mCenterAndRaiseAfterFirstHeightChangeEvent = false;
    bool mInitialLayoutComplete = false;
    QTimer mShowWhenCreatedFallbackTimer;
    QTimer mRestoreOpacityTimer;
    QSize mTrackedSize;
    qreal mPreviousOpacity = 1.0;
};

#endif // QML_DIALOG_H
