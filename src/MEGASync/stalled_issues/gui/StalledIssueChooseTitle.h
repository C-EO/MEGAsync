#ifndef STALLEDISSUECHOOSETITLE_H
#define STALLEDISSUECHOOSETITLE_H

#include "StalledIssueActionTitle.h"

class StalledIssueChooseTitle : public StalledIssueActionTitle
{
    Q_OBJECT

public:
    StalledIssueChooseTitle(QWidget *parent = nullptr);

    void showIcon() override;
};

#endif // STALLEDISSUECHOOSETITLE_H
