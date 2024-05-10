#ifndef SYNCEXCLUSIONS_H
#define SYNCEXCLUSIONS_H

#include "qml/QmlDialogWrapper.h"
#include "gui/SyncExclusions/ExclusionRulesModel.h"
#include <QScreen>

class SyncExclusions : public QMLComponent
{
    Q_OBJECT
    Q_PROPERTY(int minimumAllowedSize MEMBER mMinimumAllowedSize READ getMinimumAllowedSize WRITE setMinimumAllowedSize NOTIFY minimumAllowedSizeChanged)
    Q_PROPERTY(int maximumAllowedSize MEMBER mMaximumAllowedSize READ getMaximumAllowedSize WRITE setMaximumAllowedSize NOTIFY maximumAllowedSizeChanged)
    Q_PROPERTY(int minimumAllowedUnit MEMBER mMinimumAllowedUnit READ getMinimumAllowedUnit WRITE setMinimumAllowedUnit NOTIFY minimumAllowedUnitChanged)
    Q_PROPERTY(int maximumAllowedUnit MEMBER mMaximumAllowedUnit READ getMaximumAllowedUnit WRITE setMaximumAllowedUnit NOTIFY maximumAllowedUnitChanged)
    Q_PROPERTY(ExclusionRulesModel* rulesModel MEMBER mRulesModel CONSTANT)
    Q_PROPERTY(SizeExclusionStatus sizeExclusionStatus READ getSizeExclusionStatus WRITE setSizeExclusionStatus NOTIFY sizeExclusionStatusChanged)
    Q_PROPERTY(QString folderName READ getFolderName NOTIFY folderNameChanged)
    Q_PROPERTY(bool askOnExclusionRemove READ isAskOnExclusionRemove WRITE setAskOnExclusionRemove NOTIFY askOnExclusionRemoveChanged)

public:
    SyncExclusions(QWidget *parent = 0, const QString &path = QString::fromUtf8(""));
    ~SyncExclusions();

    enum SizeExclusionStatus{
        OUTSIDE_OF = 2,
        SMALLER_THAN = 1,
        BIGGER_THAN = 0,
        DISABLED = 3
    };
    Q_ENUM(SizeExclusionStatus)

    int getMinimumAllowedSize() const {return mMinimumAllowedSize;}
    void setMinimumAllowedSize(int minimumSize);
    double getMaximumAllowedSize()  const {return mMaximumAllowedSize;}
    void setMaximumAllowedSize(int maximumSize);
    int getMinimumAllowedUnit() const {return mMinimumAllowedUnit;}
    void setMinimumAllowedUnit(int minimumUnit);
    int getMaximumAllowedUnit()  const {return mMaximumAllowedUnit;}
    void setMaximumAllowedUnit(int maximumUnit);
    SizeExclusionStatus getSizeExclusionStatus()  const;
    void setSizeExclusionStatus(SizeExclusionStatus);
    QString getFolderName() const { return mFolderName; }
    void setFolder(const QString& folderName);
    Q_INVOKABLE void restoreDefaults();
    Q_INVOKABLE void chooseFile();
    Q_INVOKABLE void chooseFolder();

    bool isAskOnExclusionRemove()  const;
    void setAskOnExclusionRemove(bool);

    QUrl getQmlUrl() override;

    QString contextName() override;


public slots:
    void applyChanges();

signals:
    void minimumAllowedSizeChanged(double);
    void maximumAllowedSizeChanged(double);
    void minimumAllowedUnitChanged(int);
    void maximumAllowedUnitChanged(int);
    void sizeExclusionStatusChanged(SizeExclusionStatus);
    void folderNameChanged(QString);
    void fileChoosen(QString relativeFileName);
    void folderChoosen(QString relativeFolderName);
    void askOnExclusionRemoveChanged(bool);

private:
    int mMinimumAllowedSize;
    int mMaximumAllowedSize;
    MegaIgnoreSizeRule::UnitTypes mMinimumAllowedUnit;
    MegaIgnoreSizeRule::UnitTypes mMaximumAllowedUnit;
    std::shared_ptr<MegaIgnoreManager> mMegaIgnoreManager; // TODO: Remove this and make all usage through the model
    ExclusionRulesModel* mRulesModel;
    QString mFolderName;
    QString mFolderFullPath;
};

#endif // SYNCEXCLUSIONS_H
