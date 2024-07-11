#ifndef APPSTATSEVENTS_H
#define APPSTATSEVENTS_H

#include <QMap>
#include <QObject>

class AppStatsEvents
{
    Q_GADGET

public:

    // Event IDs sent to servers for statistics purpose.
    enum class EventType
    {
        NONE,
        FIRST_START,
        FIRST_SYNC,
        FIRST_SYNCED_FILE,
        FIRST_WEBCLIENT_DL,
        UNINSTALL,
        ACC_CREATION_START,
        PRO_REDIRECT,
        MEM_USAGE,
        UPDATE,
        UPDATE_OK,
        DUP_FINISHED_TRSF,
        OVER_STORAGE_DIAL,
        OVER_STORAGE_NOTIF,
        OVER_STORAGE_MSG,
        ALMOST_OVER_STORAGE_MSG,
        ALMOST_OVER_STORAGE_NOTIF,
        MAIN_DIAL_WHILE_OVER_QUOTA,
        MAIN_DIAL_WHILE_ALMOST_OVER_QUOTA,
        RED_LIGHT_USED_STORAGE_MISMATCH,
        TRSF_OVER_QUOTA_DIAL,
        TRSF_OVER_QUOTA_NOTIF,
        TRSF_OVER_QUOTA_MSG,
        TRSF_ALMOST_OVER_QUOTA_MSG,
        PAYWALL_NOTIF,
        SYNC_ADD_FAIL_API_EACCESS,
        TRSF_ALMOST_OVERQUOTA_NOTIF,
        FIRST_BACKUP,
        FIRST_BACKED_UP_FILE,
        SI_NAMECONFLICT_SOLVED_MANUALLY,
        SI_NAMECONFLICT_SOLVED_AUTOMATICALLY,
        SI_NAMECONFLICT_SOLVED_SEMI_AUTOMATICALLY,
        SI_LOCALREMOTE_SOLVED_MANUALLY,
        SI_LOCALREMOTE_SOLVED_AUTOMATICALLY,
        SI_LOCALREMOTE_SOLVED_SEMI_AUTOMATICALLY,
        SI_IGNORE_SOLVED_MANUALLY,
        SI_STALLED_ISSUE_RECEIVED,
        SI_IGNORE_ALL_SYMLINK,
        SI_SMART_MODE_FIRST_SELECTED,
        SI_ADVANCED_MODE_FIRST_SELECTED,
        SI_CHANGE_TO_SMART_MODE,
        SI_CHANGE_TO_ADVANCED_MODE,
        SI_FINGERPRINT_MISSING_SOLVED_MANUALLY,
        SI_MOVERENAME_CANNOT_OCCUR_SOLVED_MANUALLY,
        DAILY_ACTIVE_USER,
        MONTHLY_ACTIVE_USER,
        LOGIN_CLICKED,
        LOGOUT_CLICKED,
        TRANSFER_TAB_CLICKED,
        NOTIFICATION_TAB_CLICKED,
        NOTIFICATION_SETTINGS_CLICKED,
        UPGRADE_ACCOUNT_CLICKED,
        OPEN_TRANSFER_MANAGER_CLICKED,
        ADD_SYNC_CLICKED,
        ADD_BACKUP_CLICKED,
        UPLOAD_CLICKED,
        AVATAR_CLICKED,
        MENU_CLICKED,
        MENU_ABOUT_CLICKED,
        MENU_CLOUD_DRIVE_CLICKED,
        MENU_ADD_SYNC_CLICKED,
        MENU_ADD_BACKUP_CLICKED,
        MENU_OPEN_LINKS_CLICKED,
        MENU_UPLOAD_CLICKED,
        MENU_DOWNLOAD_CLICKED,
        MENU_STREAM_CLICKED,
        MENU_SETTINGS_CLICKED,
        MENU_EXIT_CLICKED,
        SETTINGS_GENERAL_TAB_CLICKED,
        SETTINGS_ACCOUNT_TAB_CLICKED,
        SETTINGS_SYNC_TAB_CLICKED,
        SETTINGS_BACKUP_TAB_CLICKED,
        SETTINGS_SECURITY_TAB_CLICKED,
        SETTINGS_FOLDERS_TAB_CLICKED,
        SETTINGS_NETWORK_TAB_CLICKED,
        SETTINGS_NOTIFICATIONS_TAB_CLICKED,
        SETTINGS_EXPORT_KEY_CLICKED,
        SETTINGS_CHANGE_PASSWORD_CLICKED,
        SETTINGS_REPORT_ISSUE_CLICKED
    };
    Q_ENUM(EventType)

    static const char* getEventMessage(EventType event);
    static int getEventType(EventType event);
    static EventType getEventType(int event);

private:
    static QMap<EventType, int> mTypeMap;
    static QMap<EventType, const char*> mMessageMap;

};

#endif // APPSTATSEVENTS_H
