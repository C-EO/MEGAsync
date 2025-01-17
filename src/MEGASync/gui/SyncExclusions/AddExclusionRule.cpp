#include "AddExclusionRule.h"

#include "MegaIgnoreManager.h"
#include "ExclusionRulesModel.h"

AddExclusionRule::AddExclusionRule(QObject *parent, const QStringList &folders)
    : QMLComponent{parent}
    , mFolders(folders)
{
}

QUrl AddExclusionRule::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/sync_exclusions/AddRuleDialog.qml"));
}

QString AddExclusionRule::contextName()
{
    return QString::fromUtf8("addRuleDialogAccess");
}

void AddExclusionRule::appendRuleToFolders(int targetType, int wildCard, QString ruleValue)
{
    if(ruleValue.trimmed().isEmpty())
    {
        return;
    }

    auto splitted = ruleValue.split(QString::fromUtf8(","));
    for (const auto& folder : mFolders)
    {
        MegaIgnoreManager megaIgnoreLoader(folder, true);
        for (auto value : splitted)
        {
            value = value.trimmed();
            if (value.isEmpty())
            {
                continue;
            }

            if (targetType == ExclusionRulesModel::TargetType::EXTENSION)
            {
                megaIgnoreLoader.addExtensionRule(MegaIgnoreNameRule::Class::EXCLUDE, value);
                continue;
            }

            if((targetType == ExclusionRulesModel::TargetType::FILE
                    || targetType == ExclusionRulesModel::TargetType::FOLDER)
                && wildCard == MegaIgnoreNameRule::WildCardType::EQUAL)
            {
                value = getRelative(folder, value);
            }
            MegaIgnoreNameRule::Target target = MegaIgnoreNameRule::Target::a;
            switch (targetType)
            {
                case ExclusionRulesModel::TargetType::FILE:
                    target = MegaIgnoreNameRule::Target::f;
                    break;
                case ExclusionRulesModel::TargetType::FOLDER:
                    target = MegaIgnoreNameRule::Target::d;
                    break;
                case ExclusionRulesModel::TargetType::FILES_AND_FOLDERS:
                    target = MegaIgnoreNameRule::Target::a;
                    break;
                default:
                    target = MegaIgnoreNameRule::Target::a;
                    break;
            }
            megaIgnoreLoader.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                         value,
                                         target,
                                         MegaIgnoreNameRule::Type::NONE,
                                         static_cast<MegaIgnoreNameRule::WildCardType>(wildCard));
        }
        megaIgnoreLoader.applyChanges();
    }
}

QString AddExclusionRule::getRelative(const QString& path, const QString& fullPath)
{
    auto folder = QDir::toNativeSeparators(QDir(path).canonicalPath());
    if(!folder.isNull() && !folder.isEmpty())
    {
        QString relativePath = QDir(folder).relativeFilePath(fullPath);
        return relativePath;
    }
    return path;
}
