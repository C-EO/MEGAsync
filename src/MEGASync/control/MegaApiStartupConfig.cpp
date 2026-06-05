#include "MegaApiStartupConfig.h"

#include "megaapi.h"
#include "Preferences.h"

#include <QDebug>

#include <memory>

void MegaApiStartupConfig::initialConfiguration(mega::MegaApi* megaApi)
{
    megaApi->setMaxPayloadLogSize(Preferences::instance()->getMaxPayloadLogSize());
}

void MegaApiStartupConfig::applyFileServiceReclaimOptions(mega::MegaApi* megaApi)
{
    if (megaApi->isLoggedIn())
    {
        QString errorMessage =
            QStringLiteral("configure shouldn't be called if we are already logged.");

#ifdef QT_DEBUG
        throw std::logic_error(errorMessage.toStdString().c_str());
#else
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, errorMessage.toStdString().c_str());
#endif
    }

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
