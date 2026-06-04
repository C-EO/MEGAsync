#include "MegaAPIStartupConfig.h"

#include "megaapi.h"
#include "Preferences.h"

#include <memory>

void MegaApiStartupConfig::configure(mega::MegaApi* megaApi,
                                     bool configureFileServiceCacheReclamation)
{
    if (Preferences::instance()->accountStateInGeneral() == Preferences::STATE_LOGGED_OK)
    {
        QString errorMessage =
            QStringLiteral("configure shouldn't be called if we are already logged.");

#ifdef QT_DEBUG
        throw std::logic_error(errorMessage.toStdString().c_str());
#else
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, errorMessage.toStdString().c_str());
#endif
    }

    megaApi->setMaxPayloadLogSize(Preferences::instance()->getMaxPayloadLogSize());

    if (configureFileServiceCacheReclamation)
    {
        applyFileServiceReclaimOptions(megaApi);
    }
}

void MegaApiStartupConfig::applyFileServiceReclaimOptions(mega::MegaApi* megaApi)
{
    std::unique_ptr<mega::MegaFileServiceReclaimOptions> options{
        mega::MegaFileServiceReclaimOptions::create()};

    if (!options)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Unable to create FileService reclaim options.");
        return;
    }

    options->setAgeThreshold(Preferences::instance()->getReclaimAgeThresholdMinutes());
    options->setBatchSize(Preferences::instance()->getReclaimBatchSize());
    options->setDelay(Preferences::instance()->getReclaimDelaySeconds());
    options->setPeriod(Preferences::instance()->getReclaimPeriodSeconds());
    options->setReclaimTarget(Preferences::instance()->getReclaimTargetBytes());
    options->setReclaimThreshold(Preferences::instance()->getReclaimThresholdBytes());

    megaApi->fileServiceSetReclaimOptions(options.get());

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                       "FileService reclaim options configured for MEGAsync.");
}
