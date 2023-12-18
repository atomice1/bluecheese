#ifndef OPTIONS_H
#define OPTIONS_H

struct Options
{
    enum class Feature {
        Ai
    };
    bool featureAiEnabled {false};
    bool isFeatureEnabled(Feature feature) const;
};

#endif // OPTIONS_H
