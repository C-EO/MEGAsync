#include "MegaApiStartupConfig.h"

#include "megaapi.h"
#include "Preferences.h"

#include <QDebug>

#include <memory>

void MegaApiStartupConfig::initialConfiguration(mega::MegaApi* megaApi)
{
    megaApi->setMaxPayloadLogSize(Preferences::MAX_PAY_LOAD_LOG_SIZE);
}

void MegaApiStartupConfig::applyFileServiceReclaimOptions(mega::MegaApi* megaApi)
{
    Q_ASSERT_X(!megaApi->isLoggedIn(),
               __PRETTY_FUNCTION__,
               "configure shouldn't be called if we are already logged.");

    std::unique_ptr<mega::MegaFileServiceReclaimOptions> options{
        mega::MegaFileServiceReclaimOptions::create()};

    if (!options)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Unable to create FileService reclaim options.");
        return;
    }

    options->setAgeThreshold(Preferences::RECLAIM_AGE_THRESHOLD_MINUTES);
    options->setBatchSize(Preferences::RECLAIM_BATCH_SIZE);
    options->setDelay(Preferences::RECLAIM_DELAY_SECONDS);
    options->setPeriod(Preferences::RECLAIM_PERIOD_SECONDS);
    options->setReclaimTarget(Preferences::RECLAIM_TARGET_BYTES);
    options->setReclaimThreshold(Preferences::RECLAIM_THRESHOLD_BYTES);

    megaApi->fileServiceSetReclaimOptions(options.get());

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                       "FileService reclaim options configured for MEGAsync.");
}
