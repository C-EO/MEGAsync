#ifndef MEGAUSERALERTEXT_H
#define MEGAUSERALERTEXT_H

#include "megaapi.h"

#include <QObject>

#include <memory>

class MegaUserAlertExt : public QObject
{
    Q_OBJECT

public:
    MegaUserAlertExt() = delete;
    MegaUserAlertExt(mega::MegaUserAlert* megaUserAlert, QObject *parent = nullptr);
    ~MegaUserAlertExt();

    MegaUserAlertExt& operator=(MegaUserAlertExt&& megaUserAlert);

    QString getEmail() const;
    void setEmail(QString email);
    bool isValid() const;
    void reset(mega::MegaUserAlert* alert);

    virtual unsigned getId() const;
    virtual bool getSeen() const;
    virtual bool getRelevant() const;
    virtual int getType() const;
    virtual mega::MegaHandle getUserHandle() const;
    virtual int64_t getTimestamp(unsigned index) const;
    virtual int64_t getNumber(unsigned index) const;
    virtual mega::MegaHandle getNodeHandle() const;
    virtual const char* getString(unsigned index) const;
    virtual const char* getTitle() const;

signals:
    void emailChanged();

private:
    std::unique_ptr<mega::MegaUserAlert> mMegaUserAlert;
    QString mEmail;

    void init();
};

#endif // MEGAUSERALERTEXT_H
