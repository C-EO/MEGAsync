#ifndef CLASSFACTORYSHELLEXTSYNCING_H
#define CLASSFACTORYSHELLEXTSYNCING_H

#include "ClassFactory.h"
class ClassFactoryShellExtSyncing :
    public ClassFactory
{
public:
    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
};

#endif
