#ifndef TEXTDECORATOR_H
#define TEXTDECORATOR_H

#include "QObject"

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
public:
   explicit Link(const QString& link, QObject* parent = nullptr);
   void process(QString& input) const override;
private:
   QString mLinkAddress;
};

class ClearLink : public Decorator
{
public:
   explicit ClearLink(QObject* parent = nullptr);
   void process(QString& input) const override;
};

class Bold : public Decorator
{
public:
   explicit Bold(QObject* parent = nullptr);
   void process(QString& input) const override;
};
}

#endif // TEXTDECORATOR_H
