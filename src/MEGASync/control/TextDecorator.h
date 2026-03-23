#ifndef TEXTDECORATOR_H
#define TEXTDECORATOR_H

#include <QObject>

namespace Text
{

class Decorator : public QObject
{
    Q_OBJECT
public:
    explicit Decorator(QObject* parent);
    virtual void process(QString& input) const;
};

class Link : public Decorator
{
    Q_OBJECT

public:
   explicit Link(const QStringList& links, QObject* parent = nullptr);
   explicit Link(const QString& link, QObject* parent = nullptr);
   void process(QString& input) const override;
private:
   QStringList mLinkAddresses;
};

class ClearLink : public Decorator
{
    Q_OBJECT

public:
   explicit ClearLink(QObject* parent = nullptr);
   void process(QString& input) const override;
};

class Bold : public Decorator
{
    Q_OBJECT

public:
   explicit Bold(QObject* parent = nullptr);
   void process(QString& input) const override;
};

class NewLine : public Decorator
{
    Q_OBJECT

public:
    explicit NewLine(QObject* parent = nullptr);
    void process(QString& input) const override;
    void process(QString& input, int brCount) const;
};

class RichText: public Decorator
{
    Q_OBJECT

public:
    explicit RichText(QObject* parent = nullptr);
    explicit RichText(const QStringList& links, QObject* parent = nullptr);
    explicit RichText(const QString& link, QObject* parent = nullptr);
    void process(QString& input) const override;

private:
    QStringList mLinkAddresses;
};
}

#endif // TEXTDECORATOR_H
