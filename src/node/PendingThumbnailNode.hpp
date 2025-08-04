#pragma once
#include <Geode/Geode.hpp>

class PendingThumbnailNode : public CCNode {
public:
    static PendingThumbnailNode* create();
    virtual bool init();
    void fetchPendingThumbnails();
    void addThumbnail(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);

private:
    EventListener<web::WebTask> m_listener;
};
