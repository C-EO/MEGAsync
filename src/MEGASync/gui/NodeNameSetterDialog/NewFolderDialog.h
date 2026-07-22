#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H

#include "megaapi.h"
#include "NodeNameSetterDialog.h"

#include <memory>

class NewFolderDialog : public NodeNameSetterDialog
{
    Q_OBJECT

public:
    NewFolderDialog(std::shared_ptr<mega::MegaNode> parentNode, QWidget* parent);
    ~NewFolderDialog() = default;

    std::unique_ptr<mega::MegaNode> getNewNode();

signals:
    // Emitted when the folder creation request is actually sent (the user confirmed). Lets the
    // node selector initialise the parent's children in the model in time, so the new node later
    // arrives via the visible add path and is selected through checkNewFolderAdded.
    void creatingFolder(mega::MegaHandle parentHandle);

protected:
    void onDialogAccepted() override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    QString dialogText() override;
    void title() override;

private:
    std::unique_ptr<mega::MegaNode> mNewNode;
    std::shared_ptr<mega::MegaNode> mParentNode;
};

#endif // NEWFOLDERDIALOG_H
