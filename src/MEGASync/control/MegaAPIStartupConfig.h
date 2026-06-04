#ifndef MEGA_API_STARTUP_CONFIG_H
#define MEGA_API_STARTUP_CONFIG_H

namespace mega
{
class MegaApi;
}

class MegaApiStartupConfig
{
public:
    MegaApiStartupConfig() = delete;

    static void configure(mega::MegaApi* megaApi,
                          bool configureFileServiceCacheReclamation = false);

private:
    static void applyFileServiceReclaimOptions(mega::MegaApi* megaApi);
};

#endif
