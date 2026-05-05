#include "MegaAPIStartupConfig.h"

#include "megaapi.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace
{

// Prevents large glitches (Mac beachball) caused by network logging hundreds of MB
constexpr auto maxPayloadLogSize = std::size_t{10240};

constexpr auto reclaimAgeThresholdMinutes = 3 * 24 * 60;
constexpr auto reclaimBatchSize = std::size_t{4};
constexpr auto reclaimDelaySeconds = std::uint64_t{30 * 60};
constexpr auto reclaimPeriodSeconds = std::uint64_t{2 * 60 * 60};
constexpr auto reclaimTargetBytes = std::uint64_t{1024} * 1024 * 1024;
constexpr auto reclaimThresholdBytes =
    std::int64_t{10} * static_cast<std::int64_t>(reclaimTargetBytes);

void applyFileServiceReclaimOptions(mega::MegaApi& api)
{
    std::unique_ptr<mega::MegaFileServiceReclaimOptions> options{
        mega::MegaFileServiceReclaimOptions::create()};

    if (!options)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Unable to create FileService reclaim options.");
        return;
    }

    options->setAgeThreshold(reclaimAgeThresholdMinutes);
    options->setBatchSize(reclaimBatchSize);
    options->setDelay(reclaimDelaySeconds);
    options->setPeriod(reclaimPeriodSeconds);
    options->setReclaimTarget(reclaimTargetBytes);
    options->setReclaimThreshold(reclaimThresholdBytes);

    api.fileServiceSetReclaimOptions(options.get());

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                       "FileService reclaim options configured for MEGAsync.");
}

} // namespace

void MegaAPIStartupConfig::apply(mega::MegaApi& primaryApi, mega::MegaApi& secondaryApi)
{
    primaryApi.setMaxPayloadLogSize(maxPayloadLogSize);
    secondaryApi.setMaxPayloadLogSize(maxPayloadLogSize);

    applyFileServiceReclaimOptions(primaryApi);
}
