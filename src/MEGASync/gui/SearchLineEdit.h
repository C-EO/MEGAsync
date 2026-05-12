#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include "ButtonIconManager.h"

#include <QDialog>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QIcon>
#include <QPointer>
#include <QPropertyAnimation>

namespace Ui
{
class SearchLineEdit;
}

class ButtonIconManager;

class SearchLineEdit: public QFrame
{
    Q_OBJECT

public:
    Q_PROPERTY(Mode mode MEMBER mMode WRITE setMode)

    explicit SearchLineEdit(QWidget* parent = nullptr);
    ~SearchLineEdit();
    void setIcon(const QIcon& icon);
    void setText(const QString& text);
    void showTextEntry(bool state, bool force = false);

    enum Mode
    {
        ALWAYS_EXPANDED,
        EXPANDABLE
    };
    Q_ENUM(Mode)

    void setMode(Mode mode);

    void addCustomWidget(QWidget* widget);

protected:
    bool eventFilter(QObject* obj, QEvent* evnt) override;
    bool event(QEvent* event) override;

signals:
    void search(const QString& text);
    void cleared();

public slots:
    void onClearClicked();

private slots:
    void onTextChanged(const QString& text);
    void onSearchButtonClicked();
    void animationFinished();

private:
    void toggleClearButton(bool fadeIn);
    QPropertyAnimation* runWidthAnimation(QWidget* target, bool expand);
    QPropertyAnimation* runOpacityAnimation(QWidget* target, bool fadeIn);
    QPropertyAnimation* runGeometryAnimation(QWidget* target,
                                             const QRect& startRect,
                                             const QRect& endRect,
                                             QEasingCurve type);
    void expand();

    Ui::SearchLineEdit* ui;
    ButtonIconManager mButtonManager;
    QString mOldString;
    QPointer<QDialog> mTopParent;
    Mode mMode;
};

#endif // SEARCHLINEEDIT_H
