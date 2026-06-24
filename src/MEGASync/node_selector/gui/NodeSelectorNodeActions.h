#ifndef NODESELECTORNODEACTIONS_H
#define NODESELECTORNODEACTIONS_H

#include "megaapi.h"

#include <QList>
#include <QPointer>
#include <QWidget>

#include <memory>

class NodeSelectorModel;

class NodeSelectorNodeActions
{
public:
    explicit NodeSelectorNodeActions(mega::MegaApi* megaApi);

    void setModel(NodeSelectorModel* model);
    void setDialogParent(QWidget* dialogParent);

    void renameNode(mega::MegaHandle selectedHandle) const;
    void deleteNodes(const QList<mega::MegaHandle>& handles,
                     bool permanently,
                     bool showConfirmationMessageBox) const;
    void leaveShare(const QList<mega::MegaHandle>& handles) const;
    void exportLinks(const QList<mega::MegaHandle>& handles) const;

private:
    std::shared_ptr<mega::MegaNode> getNode(mega::MegaHandle handle) const;
    QPointer<QWidget> mDialogParent;
    NodeSelectorModel* mModel;
    mega::MegaApi* mMegaApi;
};

#endif // NODESELECTORNODEACTIONS_H
