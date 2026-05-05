#ifndef CLASSFACTORYSHELLEXTPENDING_H
#define CLASSFACTORYSHELLEXTPENDING_H

#include "ClassFactory.h"
class ClassFactoryShellExtPending :
    public ClassFactory
{
public:
    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
};

#endif
