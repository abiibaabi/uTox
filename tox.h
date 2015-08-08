/* todo: proper system for posting messages to the toxcore thread, comments, better names (?), proper cleanup of a/v and a/v thread*/
/* -proper unpause/pause file transfers, resuming file transfers + what if new file transfer with same id gets created before the main thread receives the message for the old one?
>= GiB file sizes with FILE_*_PROGRESS on 32bit */

/* details about messages and their (param1, param2, data) values are in the message handlers in tox.c*/

typedef struct {
    uint8_t msg;
    uint32_t param1, param2;
    void *data;
} TOX_MSG;

/* toxcore thread messages (sent from the client thread) */
enum {
    /* SHUTDOWNEVERYTHING! */
    TOX_KILL,

    /* Change our settings in core */
    TOX_SELF_SET_NAME,
    TOX_SELF_SET_STATUS,
    TOX_SELF_SET_STATE,

    /* Wooo pixturs */
    TOX_AVATAR_SET,
    TOX_AVATAR_UNSET,

    /* Interact with contacts */
    TOX_FRIEND_NEW,
    TOX_FRIEND_ACCEPT,
    TOX_FRIEND_DELETE,
    TOX_FRIEND_ONLINE,

    /* Default actions */
    TOX_SEND_MESSAGE, // 10
    TOX_SEND_ACTION, /* Should we deprecate this, now that core uses a single function? */
    TOX_SEND_TYPING,

    /* File Transfers */
    TOX_FILE_ACCEPT,
    TOX_FILE_SEND_NEW,
    TOX_FILE_SEND_NEW_INLINE,
    TOX_FILE_SEND_NEW_SLASH,

    TOX_FILE_RESUME,
    TOX_FILE_PAUSE,
    TOX_FILE_CANCEL,

    /* Audio/Video Calls */
    TOX_CALL_SEND, // 20
    TOX_CALL_INCOMING,
    TOX_CALL_ANSWER,
    TOX_CALL_PAUSE_AUDIO,
    TOX_CALL_PAUSE_VIDEO,
    TOX_CALL_DISCONNECT,

    TOX_GROUP_CREATE,
    TOX_GROUP_EXIT,
    TOX_GROUP_SEND_INVITE,
    TOX_GROUP_SET_TOPIC,
    TOX_GROUP_SEND_MESSAGE, // 30
    TOX_GROUP_SEND_ACTION,
    TOX_GROUP_AUDIO_START,
    TOX_GROUP_AUDIO_END,
};

struct TOX_SEND_INLINE_MSG {
    size_t image_size;
    UTOX_PNG_IMAGE image;
};

/* toxav thread messages (sent from the client thread) */
enum {
    AUDIO_KILL,
    AUDIO_SET_INPUT,
    AUDIO_SET_OUTPUT,
    AUDIO_PREVIEW_START,
    AUDIO_PREVIEW_END,
    AUDIO_CALL_START,
    AUDIO_CALL_END,
    AUDIO_PLAY_RINGTONE,
    AUDIO_STOP_RINGTONE,
    GROUP_AUDIO_CALL_START,
    GROUP_AUDIO_CALL_END,
};

enum {
    VIDEO_KILL,
    VIDEO_SET,
    VIDEO_PREVIEW_START,
    VIDEO_PREVIEW_END,
    VIDEO_CALL_START,
    VIDEO_CALL_END,
};


enum {
    TOXAV_KILL,
};
/* client thread messages (received by the client thread) */
enum {
    /* general messages */
    TOX_DONE,
    DHT_CONNECTED,
    DNS_RESULT,

    SET_AVATAR,

    SEND_FILES,
    SAVE_FILE,
    FILE_START_TEMP,
    FILE_ABORT_TEMP,

    NEW_AUDIO_IN_DEVICE,
    NEW_AUDIO_OUT_DEVICE,
    NEW_VIDEO_DEVICE,

    /* friend related */
    FRIEND_REQUEST,
    FRIEND_ACCEPT,
    FRIEND_ADD,
    FRIEND_DEL,
    FRIEND_MESSAGE,
    FRIEND_NAME,
    FRIEND_SETAVATAR,
    FRIEND_UNSETAVATAR,
    FRIEND_STATUS_MESSAGE,
    FRIEND_STATUS,
    FRIEND_TYPING,
    FRIEND_ONLINE,

    /* friend a/v */
    FRIEND_AV_STATUS_CHANGE,
    FRIEND_AV_INCOMING,
    FRIEND_AV_DISCONNECT,
    FRIEND_CALL_AUDIO_CONNECTED,
    FRIEND_CALL_AUDIO_DISCONNECTED,
    FRIEND_CALL_VIDEO,
    FRIEND_CALL_VIDEO_CONNECTED,
    FRIEND_CALL_VIDEO_DISCONNECTED,
    FRIEND_CALL_MEDIACHANGE,
    FRIEND_CALL_START_VIDEO,
    FRIEND_CALL_STOP_VIDEO,
    FRIEND_VIDEO_FRAME,
    PREVIEW_FRAME,
    PREVIEW_FRAME_NEW,

    /* friend file */
    FRIEND_FILE_NEW,
    FRIEND_FILE_UPDATE,
    FRIEND_INLINE_IMAGE,

    /* group */
    GROUP_ADD,
    GROUP_MESSAGE,
    GROUP_PEER_ADD,
    GROUP_PEER_DEL,
    GROUP_PEER_NAME,
    GROUP_TITLE,
    GROUP_AUDIO_START,
    GROUP_AUDIO_END,
    GROUP_UPDATE,

    TOOLTIP_SHOW,
};

/* AV STATUS LIST */
enum {
    UTOX_AV_NONE,
    UTOX_AV_INVITE,
    UTOX_AV_RINGING,
    UTOX_AV_STARTED,
};

/* Inter-thread communication vars. */
TOX_MSG tox_msg, audio_msg, video_msg, toxav_msg;
volatile _Bool tox_thread_msg, audio_thread_msg, video_thread_msg, toxav_thread_msg;

/** [log_read description] */
void log_read(Tox *tox, int fid);

/** [friend_meta_data_read description] */
void friend_meta_data_read(Tox *tox, int friend_id);

/** [init_avatar description]
 *
 * TODO move this to avatar.h
 */
//_Bool init_avatar(AVATAR *avatar, const char_t *id, uint8_t *png_data_out, uint32_t *png_size_out);

/* toxcore thread
 */
void tox_thread(void *args);

/* send a message to the toxcore thread
 */
void tox_postmessage(uint8_t msg, uint32_t param1, uint32_t param2, void *data);

/* send a message to the audio thread
 */
void toxaudio_postmessage(uint8_t msg, uint32_t param1, uint32_t param2, void *data);

/* send a message to the video thread
 */
void toxvideo_postmessage(uint8_t msg, uint32_t param1, uint32_t param2, void *data);

/* send a message to the toxav thread
 */
void toxav_postmessage(uint8_t msg, uint32_t param1, uint32_t param2, void *data);

/* read a message sent from the toxcore thread (sent with postmessage())
 */
void tox_message(uint8_t msg, uint16_t param1, uint16_t param2, void *data);

void tox_settingschanged(void);
