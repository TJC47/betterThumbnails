#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class PendingThumbnailLayer : public CCLayer {
public:
    EventListener<web::WebTask> m_listener;
    static PendingThumbnailLayer* create();
    static CCScene *scene();
    bool init() override;
    void onBackButton(CCObject *);
    void keyBackClicked() override;
};

