#ifndef MEGAAPISTARTUPCONFIG_H
#define MEGAAPISTARTUPCONFIG_H

namespace mega
{
class MegaApi;
}

class MegaAPIStartupConfig
{
public:
    static void apply(mega::MegaApi& primaryApi, mega::MegaApi& secondaryApi);

private:
    MegaAPIStartupConfig() = delete;
};

#endif // MEGAAPISTARTUPCONFIG_H
