#include "TokenParserWidgetManager.h"

#include "ThemeManager.h"
#include "IconTokenizer.h"

#include <QDir>
#include <QWidget>
#include <QToolButton>
#include <QBitmap>
#include <QtConcurrent/QtConcurrent>

namespace // anonymous namespace to hide names from other translation units
{
    static const QMap<Preferences::ThemeType, QString> THEME_NAMES = {
        {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("Light")},
        {Preferences::ThemeType::DARK_THEME,  QObject::tr("Dark")}
    };

    static QRegularExpression colorTokenRegularExpression(QString::fromUtf8("(#.*) *; *\\/\\* *colorToken\\.(.*)\\*\\/"));
    static QRegularExpression iconColorTokenRegularExpression(QString::fromUtf8(" *\\/\\* *ColorTokenIcon;(.*);(.*);(.*);(.*);colorToken\\.(.*) *\\*\\/"));
    static QRegularExpression replaceThemeTokenRegularExpression(QString::fromUtf8(".*\\/(light|dark)\\/.*; *\\/\\* *replaceThemeToken *\\*\\/"));

    static const QString jsonThemedColorTokenFile = QString::fromUtf8(":/colors/ColorThemedTokens.json");

    enum ColorTokenCaptureIndex
    {
        ColorWholeMatch,
        ColorHexColorValue,
        ColorDesignTokenName
    };

    enum IconTokenCaptureIndex
    {
        IconTokenWholeMatch,
        IconTokenTargetProperty,
        IconTokenTargetElementId,
        IconTokenTargetMode,
        IconTokenTargetState,
        IconTokenDesignTokenName
    };

    enum ReplaceThemeTokenCaptureIndex
    {
        ReplaceThemeTokenWholeMatch,
        ReplaceThemeTokenTheme
    };
}

TokenParserWidgetManager::TokenParserWidgetManager(QObject *parent)
    : QObject{parent},
    mCurrentWidget{nullptr}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &TokenParserWidgetManager::onThemeChanged);

    colorTokenRegularExpression.optimize();
    iconColorTokenRegularExpression.optimize();

    loadColorThemeJson();
}

void TokenParserWidgetManager::loadColorThemeJson()
{
    mColorThemedTokens.clear();

    QFile file(jsonThemedColorTokenFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << __func__ << " Error opening file : " << file.fileName();
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject())
    {
        qDebug() << __func__ << " Error invalid json format on file : " << file.fileName();
        return;
    }

    QJsonObject rootObj = jsonDoc.object();
    for (auto it = rootObj.begin(); it != rootObj.end(); ++it)
    {
        QMap<QString, QString> tokens;

        QString theme = it.key();
        QJsonObject token = it.value().toObject();

        for (auto innerIt = token.begin(); innerIt != token.end(); ++innerIt)
        {
            tokens.insert(innerIt.key(), innerIt.value().toString());
        }

        mColorThemedTokens.insert(theme, tokens);
    }
}

void TokenParserWidgetManager::applyCurrentTheme(QWidget* widget)
{
    if (widget == nullptr || widget == mCurrentWidget || widget->styleSheet().isEmpty())
    {
        return;
    }

    mCurrentWidget = widget;

    applyTheme(widget);
}

void TokenParserWidgetManager::applyTheme(QWidget* widget)
{
    auto theme = ThemeManager::instance()->getSelectedTheme();
    auto currentTheme = themeToString(theme);

    if (!mColorThemedTokens.contains(currentTheme))
    {
        qDebug() << __func__ << " Error theme not found : " << currentTheme;
        return;
    }

    const auto& colorTokens = mColorThemedTokens.value(currentTheme);

    QString styleSheet = widget->styleSheet();

    bool updatedStyleSheet = false;

    updatedStyleSheet |= replaceColorTokens(styleSheet, colorTokens);
    updatedStyleSheet |= replaceIconColorTokens(widget, styleSheet, colorTokens);
    updatedStyleSheet |= replaceThemeTokens(styleSheet, currentTheme);

    if (updatedStyleSheet)
    {
        widget->setStyleSheet(styleSheet);
    }
}

bool TokenParserWidgetManager::replaceColorTokens(QString& styleSheet, const ColorTokens& colorTokens)
{
    bool updated = false;

    QRegularExpressionMatchIterator matchIterator = colorTokenRegularExpression.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == ColorTokenCaptureIndex::ColorDesignTokenName)
        {
            const QString& tokenValue = colorTokens.value(match.captured(ColorTokenCaptureIndex::ColorDesignTokenName));

            auto startIndex = match.capturedStart(ColorTokenCaptureIndex::ColorHexColorValue);
            auto endIndex = match.capturedEnd(ColorTokenCaptureIndex::ColorHexColorValue);
            styleSheet.replace(startIndex, endIndex-startIndex, tokenValue);
            updated = true;
        }
    }

    return updated;
}

bool TokenParserWidgetManager::replaceIconColorTokens(QWidget* widget, QString& styleSheet, const ColorTokens& colorTokens)
{
    bool updated = false;

    QRegularExpressionMatchIterator matchIterator = iconColorTokenRegularExpression.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == IconTokenCaptureIndex::IconTokenDesignTokenName)
        {
            const QString& targetElementProperty = match.captured(IconTokenCaptureIndex::IconTokenTargetProperty);
            const QString& targetElementId = match.captured(IconTokenCaptureIndex::IconTokenTargetElementId);
            const QString& mode = match.captured(IconTokenCaptureIndex::IconTokenTargetMode);
            const QString& state = match.captured(IconTokenCaptureIndex::IconTokenTargetState);
            const QString& tokenId = match.captured(IconTokenCaptureIndex::IconTokenDesignTokenName);

            IconTokenizer::process(widget, mode, state, colorTokens, targetElementId, targetElementProperty, tokenId);
        }
    }

    return updated;
}

bool TokenParserWidgetManager::replaceThemeTokens(QString& styleSheet, const QString& currentTheme)
{
    bool updated = false;

    QRegularExpressionMatchIterator matchIterator = replaceThemeTokenRegularExpression.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == ReplaceThemeTokenCaptureIndex::ReplaceThemeTokenTheme)
        {
            auto startIndex = match.capturedStart(ReplaceThemeTokenCaptureIndex::ReplaceThemeTokenTheme);
            auto endIndex = match.capturedEnd(ReplaceThemeTokenCaptureIndex::ReplaceThemeTokenTheme);
            styleSheet.replace(startIndex, endIndex-startIndex, currentTheme.toLower());
            updated = true;
        }
    }

    return updated;
}

std::shared_ptr<TokenParserWidgetManager> TokenParserWidgetManager::instance()
{
    static std::shared_ptr<TokenParserWidgetManager> manager(new TokenParserWidgetManager());
    return manager;
}

QString TokenParserWidgetManager::themeToString(Preferences::ThemeType theme) const
{
    return THEME_NAMES.value(theme, QLatin1String("Light"));
}

void TokenParserWidgetManager::onThemeChanged(Preferences::ThemeType theme)
{
    Q_UNUSED(theme)

    if (mCurrentWidget != nullptr)
    {
        applyTheme(mCurrentWidget);
    }
}
