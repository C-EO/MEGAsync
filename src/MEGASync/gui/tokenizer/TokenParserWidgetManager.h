#ifndef THEME_WIDGET_MANAGER_H
#define THEME_WIDGET_MANAGER_H

#include <QColor>
#include <QFileDialog>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QWidget>

#include <memory>

class TokenParserWidgetManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<TokenParserWidgetManager> instance();

    void applyCurrentTheme();
    void registerWidgetForTheming(QWidget* dialog);
    void applyCurrentTheme(QWidget* dialog);
    void polish(QWidget* widget);
    QColor getColor(const QString& colorToken);
    QColor getColor(const QString& colorToken, const QString& colorSchema);
    static void styleQFileDialog(QPointer<QFileDialog> dialog);

private:
    using ColorTokens = QMap<QString, QString>;

    explicit TokenParserWidgetManager(QObject *parent = nullptr);
    void loadColorThemeJson();
    void loadStandardStyleSheetComponents();
    void onThemeChanged();
    void onUpdateRequested();
    void applyTheme(QWidget* widget, bool prependStandardComponents = true);
    void replaceColorTokens(QString& styleSheet, const ColorTokens& colorTokens);
    void removeFrameOnDialogCombos(QWidget* widget);
    void tokenizeChildStyleSheets(QWidget* widget);

    QMap<QString, ColorTokens> mColorThemedTokens;
    QMap<QString, QString> mThemedStandardComponentsStyleSheet;
    QSet<QWidget*> mRegisteredWidgets;
};

#endif // THEMEWIDGET_H
