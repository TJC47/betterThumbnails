#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ThumbnailInfoLayer : public CCLayer {
public:
    static CCScene* scene(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);
    static ThumbnailInfoLayer* create(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);

    bool init(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);

    void keyBackClicked() override;
    void onBackButton(CCObject*);

private:
    EventListener<web::WebTask> m_listener;
};
