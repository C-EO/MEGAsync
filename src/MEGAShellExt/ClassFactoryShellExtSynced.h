#ifndef CLASSFACTORYSHELLEXTSYNCED_H
#define CLASSFACTORYSHELLEXTSYNCED_H

#include "ClassFactory.h"
class ClassFactoryShellExtSynced :
    public ClassFactory
{
public:
    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
};

#endif
