#ifndef TABSELECTOR_H
#define TABSELECTOR_H

#include <QEvent>
#include <QPointer>
#include <QWidget>

namespace Ui
{
class TabSelector;
}

class TokenPropertySetter;

class TabSelector: public QWidget
{
    Q_OBJECT

public:
    explicit TabSelector(QWidget* parent = nullptr);
    ~TabSelector();

    Q_PROPERTY(QString title MEMBER mTitle WRITE setTitle READ getTitle)
    Q_PROPERTY(bool iconOnly WRITE setIconOnly READ isIconOnly)
    Q_PROPERTY(QString normal_off WRITE setNormalOff READ getNormalOff)
    Q_PROPERTY(QString normal_on WRITE setNormalOn READ getNormalOn)
    void setTitle(const QString& title);
    QString getTitle() const;

    Q_PROPERTY(QIcon icon WRITE setIcon READ getIcon)
    void setIcon(const QIcon& icon);
    QIcon getIcon() const;

    Q_PROPERTY(QSize iconSize READ getIconSize)
    QSize getIconSize() const;

    Q_PROPERTY(bool closeButtonVisible WRITE setCloseButtonVisible READ isCloseButtonVisible)
    void setCloseButtonVisible(bool state);
    bool isCloseButtonVisible() const;

    void setCounter(unsigned long long count);
    bool hasEmptyCount();

    void setSelected(bool state);
    bool isSelected() const;
    void toggleOffSiblings();

    void setIconOnly(bool state);
    bool isIconOnly() const;

    void setNormalOff(const QString& token);
    QString getNormalOff() const;

    void setNormalOn(const QString& token);
    QString getNormalOn() const;

    void setIconTokens(const std::shared_ptr<TokenPropertySetter>& newIconTokens);
    void hideIcon();

    void hide();

    void connectToDropEvent(std::function<void(std::shared_ptr<QDropEvent>)> slot);

    // Convenient method to set to all selectors
    static void applyTokens(QWidget* parent, std::shared_ptr<TokenPropertySetter> iconTokensSetter);

    // Convenient method to select tab
    static void selectTabIf(QWidget* parent, const char* property, const QVariant& value);

    // Convenient method to get the tabs
    static QList<TabSelector*> getTabSelectorByParent(QWidget* parent);

    // convenient method to apply actions over tabs
    static void applyActionToTabSelectors(QWidget* parent, std::function<void(TabSelector*)> func);

signals:
    void clicked();
    void hidden();
    void dropOnTabSelector(std::shared_ptr<QDropEvent> event);

protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    Ui::TabSelector* ui;
    QPointer<QWidget> mTabSelectorGroupParent;
    std::shared_ptr<TokenPropertySetter> mIconTokens;
    std::shared_ptr<TokenPropertySetter> mCloseButtonTokens;
    QString mTitle;
    bool mConnectedToDropEvent;
    bool mCloseButtonVisible;
    bool mIconOnly;
    QString mNormalOff;
    QString mNormalOn;
};

#endif // TABSELECTOR_H
