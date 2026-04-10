#ifndef MACUPDATEROUTING_H
#define MACUPDATEROUTING_H

#if defined(__APPLE__)

#include <sys/sysctl.h>

namespace MacUpdateRouting
{

inline constexpr const char* kArm64Track = "msyncarm64";
inline constexpr const char* kIntelTrack = "msyncv2";

enum class Architecture
{
    Unknown,
    X86_64,
    Arm64
};

struct RuntimeArchitecture
{
    Architecture systemArchitecture = Architecture::Unknown;
    Architecture binaryArchitecture = Architecture::Unknown;
    bool translated = false;
};

enum class UpdateRoutingCase
{
    NativeAppleSilicon,
    RosettaOnAppleSilicon,
    IntelBinaryOnAppleSilicon,
    NativeIntel,
    UnknownSystemFallback
};

struct UpdateRoutingDecision
{
    const char* updateTrack = "";
    UpdateRoutingCase routingCase = UpdateRoutingCase::UnknownSystemFallback;
    RuntimeArchitecture runtimeArchitecture;
};

inline Architecture currentBinaryArchitecture()
{
#if defined(__arm64__) || defined(__aarch64__)
    return Architecture::Arm64;
#elif defined(__x86_64__)
    return Architecture::X86_64;
#else
    return Architecture::Unknown;
#endif
}

inline bool readSysctlFlag(const char* name, bool& value)
{
    int flag = 0;
    size_t size = sizeof(flag);
    if (sysctlbyname(name, &flag, &size, nullptr, 0) == 0)
    {
        value = flag != 0;
        return true;
    }

    return false;
}

inline bool isRunningTranslated()
{
#if defined(__x86_64__)
    bool translated = false;
    if (readSysctlFlag("sysctl.proc_translated", translated))
    {
        return translated;
    }
#endif

    return false;
}

inline Architecture systemArchitecture()
{
    bool isArm64System = false;
    if (readSysctlFlag("hw.optional.arm64", isArm64System))
    {
        return isArm64System ? Architecture::Arm64 : Architecture::X86_64;
    }

    return Architecture::Unknown;
}

inline RuntimeArchitecture runtimeArchitecture()
{
    return {systemArchitecture(), currentBinaryArchitecture(), isRunningTranslated()};
}

inline const char* architectureName(Architecture architecture)
{
    switch (architecture)
    {
        case Architecture::Arm64:
            return "arm64";
        case Architecture::X86_64:
            return "x86_64";
        case Architecture::Unknown:
        default:
            return "unknown";
    }
}

inline const char* routingCaseName(UpdateRoutingCase routingCase)
{
    switch (routingCase)
    {
        case UpdateRoutingCase::NativeAppleSilicon:
            return "native_apple_silicon";
        case UpdateRoutingCase::RosettaOnAppleSilicon:
            return "rosetta_on_apple_silicon";
        case UpdateRoutingCase::IntelBinaryOnAppleSilicon:
            return "intel_binary_on_apple_silicon";
        case UpdateRoutingCase::NativeIntel:
            return "native_intel";
        case UpdateRoutingCase::UnknownSystemFallback:
        default:
            return "unknown_system_fallback";
    }
}

inline UpdateRoutingDecision selectUpdateRouting(const RuntimeArchitecture& architecture)
{
    switch (architecture.systemArchitecture)
    {
        case Architecture::Unknown:
            return {architecture.binaryArchitecture == Architecture::Arm64 ? kArm64Track :
                                                                             kIntelTrack,
                    UpdateRoutingCase::UnknownSystemFallback,
                    architecture};

        case Architecture::Arm64:
            if (architecture.binaryArchitecture == Architecture::Arm64)
            {
                return {kArm64Track, UpdateRoutingCase::NativeAppleSilicon, architecture};
            }

            if (architecture.binaryArchitecture == Architecture::X86_64)
            {
                return {kArm64Track,
                        architecture.translated ? UpdateRoutingCase::RosettaOnAppleSilicon :
                                                  UpdateRoutingCase::IntelBinaryOnAppleSilicon,
                        architecture};
            }

            return {kArm64Track, UpdateRoutingCase::NativeAppleSilicon, architecture};

        case Architecture::X86_64:
            return {kIntelTrack, UpdateRoutingCase::NativeIntel, architecture};
    }

    return {kIntelTrack, UpdateRoutingCase::NativeIntel, architecture};
}

} // namespace MacUpdateRouting

#endif // defined(__APPLE__)

#endif // MACUPDATEROUTING_H
