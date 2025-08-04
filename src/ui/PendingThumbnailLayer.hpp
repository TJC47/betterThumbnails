#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class PendingThumbnailLayer : public CCLayer {
public:
    static PendingThumbnailLayer* create();
    static CCScene *scene();
    bool init() override;
    void onBackButton(CCObject *);
    void keyBackClicked() override;
};

