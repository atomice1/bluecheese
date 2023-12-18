#include "options.h"

bool Options::isFeatureEnabled(Options::Feature feature) const
{
    switch (feature) {
    case Feature::Ai:
        return featureAiEnabled;
    default:
        return false;
    }
}
