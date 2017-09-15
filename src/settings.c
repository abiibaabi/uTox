#include "settings.h"
#include "../minIni/dev/minIni.h"

#include "debug.h"
#include "flist.h"
#include "groups.h"
#include "tox.h"

// TODO do we want to include the UI headers here?
// Or would it be better to supply a callback after settings are loaded?
#include "ui/edit.h"
#include "ui/switch.h"
#include "ui/dropdown.h"

#include "layout/settings.h"

#include "native/filesys.h"
#include "native/keyboard.h"

#include "main.h" // UTOX_VERSION_NUMBER, MAIN_HEIGHT, MAIN_WIDTH, all save things..

#include <stdlib.h>
#include <string.h>

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(key, n) == 0
#define BOOL_TO_STR(b) b ? "true" : "false"
#define STR_TO_BOOL(s) strcmp(s, "true") == 0 ? true : false
#define NAMEOF(s) strchr((const char *)(#s), '>') == NULL ? #s : (strchr((const char *)(#s), '>') + 1)

uint16_t loaded_audio_out_device = 0;
uint16_t loaded_audio_in_device  = 0;

const uint16_t proxy_address_size = 256; // Magic number inside Toxcore.

const char *general_section = "general";
const char *interface_section = "interface";
const char *av_section = "av";
const char *notifications_section = "notifications";
const char *advanced_section = "advanced";

SETTINGS settings = {
    // .last_version                // included here to match the full struct
    .curr_version = UTOX_VERSION_NUMBER,
    .next_version = UTOX_VERSION_NUMBER,

    .show_splash = false,

    // Low level settings (network, profile, portable-mode)
    .enable_udp     = true,
    .enable_ipv6    = true,

    .use_proxy      = false,
    .force_proxy    = false,
    .proxy_port     = 0,

    // Tox level settings
    .block_friend_requests  = false,
    .save_encryption        = true,

    // uTox internals
    .auto_update        = false,
    .update_to_develop  = false,
    .send_version       = false,

    // .portable_mode               // included here to match the full struct


    // User interface settings
    .language               = LANG_EN,
    .audiofilter_enabled    = true,
    .push_to_talk           = false,
    .audio_preview          = false,
    .video_preview          = false,
    .send_typing_status     = false,
    // .inline_video                // included here to match the full struct
    .use_long_time_msg      = true,
    .accept_inline_images   = true,

    // UX Settings
    .logging_enabled        = true,
    .close_to_tray          = false,
    .start_in_tray          = false,
    .start_with_system      = false,
    .use_mini_flist         = false,
    .magic_flist_enabled    = false,

    .video_fps              = 25,

    // Notifications / Alerts
    .ringtone_enabled       = true,
    .status_notifications   = true,
    .group_notifications    = GNOTIFY_ALWAYS,

    .verbose = LOG_LVL_ERROR,
    .debug_file = NULL,

    // .theme                       // included here to match the full struct
    // OS interface settings
    .window_x             = 0,
    .window_y             = 0,
    .window_height        = MAIN_HEIGHT,
    .window_width         = MAIN_WIDTH,
    .window_baseline      = 0,

    .window_maximized     = 0,
};

void write_config_value_int(const char *filename, const char *section, const char *key, const long value) {
    if (ini_putl(section, key, value, filename) != 1) {
        LOG_ERR("Settings", "Error saving config value: %lu", value);
    }
}

void write_config_value_str(const char *filename, const char *section, const char *key, const char *value) {
    if (ini_puts(section, key, value, filename) != 1) {
        LOG_ERR("Settings", "Error saving config value: %s", value);
    }
}

void write_config_value_bool(const char *filename, const char *section, const char *key, const bool value) {
    if (ini_puts(section, key, BOOL_TO_STR(value), filename) != 1) {
        LOG_ERR("Settings", "Error saving config value: %s", value);
    }
}

static int config_parser(const char* section, const char* key, const char* value, void* config_v) {
    UTOX_SAVE *config = (UTOX_SAVE*) config_v;

    // general
    if (MATCH(general_section, NAMEOF(config->save_version))) {
        config->save_version = atoi(value);
    } else if (MATCH(general_section, NAMEOF(config->utox_last_version))) {
        config->utox_last_version = atoi(value);
    } else if (MATCH(general_section, NAMEOF(config->send_version))) {
        config->send_version = STR_TO_BOOL(value);
    } else if (MATCH(general_section, NAMEOF(config->update_to_develop))) {
        config->update_to_develop = STR_TO_BOOL(value);
    }

    // interface
    else if (MATCH(interface_section, NAMEOF(config->language))) {
        config->language = atoi(value);
    } else if (MATCH(interface_section, NAMEOF(config->window_x))) {
        config->window_x = atoi(value);
    } else if (MATCH(interface_section, NAMEOF(config->window_y))) {
        config->window_y = atoi(value);
    } else if (MATCH(interface_section, NAMEOF(config->window_width))) {
        config->window_width = atoi(value);
    } else if (MATCH(interface_section, NAMEOF(config->window_height))) {
        config->window_height = atoi(value);
    } else if (MATCH(interface_section, NAMEOF(config->theme))) {
        config->theme = atoi(value);
    } else if (MATCH(interface_section, NAMEOF(config->scale))) {
        config->scale = atoi(value);
    } else if (MATCH(interface_section, NAMEOF(config->logging_enabled))) {
        config->logging_enabled = STR_TO_BOOL(value);
    } else if (MATCH(interface_section, NAMEOF(config->close_to_tray))) {
        config->close_to_tray = STR_TO_BOOL(value);
    } else if (MATCH(interface_section, NAMEOF(config->start_in_tray))) {
        config->start_in_tray = STR_TO_BOOL(value);
    } else if (MATCH(interface_section, NAMEOF(config->auto_startup))) {
        config->auto_startup = STR_TO_BOOL(value);
    } else if (MATCH(interface_section, NAMEOF(config->use_mini_flist))) {
        config->use_mini_flist = STR_TO_BOOL(value);
    } else if (MATCH(interface_section, NAMEOF(config->filter))) {
        config->filter = STR_TO_BOOL(value);
    } else if (MATCH(interface_section, NAMEOF(config->magic_flist_enabled))) {
        config->magic_flist_enabled = STR_TO_BOOL(value);
    }

    // av
    else if (MATCH(av_section, NAMEOF(config->push_to_talk))) {
        config->push_to_talk = STR_TO_BOOL(value);
    } else if (MATCH(av_section, NAMEOF(config->audio_filtering_enabled))) {
        config->audio_filtering_enabled = STR_TO_BOOL(value);
    } else if (MATCH(av_section, NAMEOF(config->audio_device_in))) {
        config->audio_device_in = atoi(value);
    } else if (MATCH(av_section, NAMEOF(config->audio_device_out))) {
        config->audio_device_out = atoi(value);
    } else if (MATCH(av_section, NAMEOF(config->video_fps))) {
        config->video_fps = atoi(value);
    }

    // notifications
    else if (MATCH(notifications_section, NAMEOF(config->audible_notifications_enabled))) {
        config->audible_notifications_enabled = STR_TO_BOOL(value);
    } else if (MATCH(notifications_section, NAMEOF(config->status_notifications))) {
        config->status_notifications = STR_TO_BOOL(value);
    } else if (MATCH(notifications_section, NAMEOF(config->no_typing_notifications))) {
        config->no_typing_notifications = STR_TO_BOOL(value);
    } else if (MATCH(notifications_section, NAMEOF(config->group_notifications))) {
        config->group_notifications = atoi(value);
    }

    // advanced
    else if (MATCH(advanced_section, NAMEOF(config->enableipv6))) {
        config->enableipv6 = STR_TO_BOOL(value);
    } else if (MATCH(advanced_section, NAMEOF(config->disableudp))) {
        config->disableudp = STR_TO_BOOL(value);
    } else if (MATCH(advanced_section, NAMEOF(config->proxyenable))) {
        config->proxyenable = STR_TO_BOOL(value);
    } else if (MATCH(advanced_section, NAMEOF(config->proxy_port))) {
        config->proxy_port = atoi(value);
    } else if (MATCH(advanced_section, NAMEOF(config->proxy_ip))) {
        strcpy((char *)config->proxy_ip, value);
    } else if (MATCH(advanced_section, NAMEOF(config->force_proxy))) {
        config->force_proxy = STR_TO_BOOL(value);
    } else if (MATCH(advanced_section, NAMEOF(config->auto_update))) {
        config->auto_update = STR_TO_BOOL(value);
    }

    return 1;
}

UTOX_SAVE *utox_load_config(void) {
    UTOX_SAVE *save = calloc(1, sizeof(UTOX_SAVE) + proxy_address_size + 1);

    if (!save) {
        LOG_ERR("Settings", "Unable to calloc for UTOX_SAVE");
        return NULL;
    }

    char *config_path = get_filepath("utox_save.ini");

    if (!config_path) {
        LOG_ERR("Settings", "Unable to get utox_save.ini path");
        free(save);
        return NULL;
    }

    if (!ini_browse(config_parser, save, config_path)) {
        LOG_ERR("Settings", "Unable to parse utox_save.ini");
        free(config_path);
        free(save);
        return NULL;
    }

    free(config_path);

    return save;
}

bool utox_save_config(UTOX_SAVE *config) {
    char *config_path = get_filepath("utox_save.ini");

    if (!config_path) {
        LOG_ERR("Settings", "Unable to get utox_save.ini path");
        return NULL;
    }

    // general
    write_config_value_int(config_path, general_section, NAMEOF(config->save_version), config->save_version);
    write_config_value_int(config_path, general_section, NAMEOF(config->utox_last_version), config->utox_last_version);
    write_config_value_bool(config_path, general_section, NAMEOF(config->send_version), config->send_version);
    write_config_value_bool(config_path, general_section, NAMEOF(config->update_to_develop), config->update_to_develop);

    // interface
    write_config_value_int(config_path, interface_section, NAMEOF(config->language), config->language);
    write_config_value_int(config_path, interface_section, NAMEOF(config->window_x), config->window_x);
    write_config_value_int(config_path, interface_section, NAMEOF(config->window_y), config->window_y);
    write_config_value_int(config_path, interface_section, NAMEOF(config->window_width), config->window_width);
    write_config_value_int(config_path, interface_section, NAMEOF(config->window_height), config->window_height);
    write_config_value_int(config_path, interface_section, NAMEOF(config->theme), config->theme);
    write_config_value_int(config_path, interface_section, NAMEOF(config->scale), config->scale);
    write_config_value_bool(config_path, interface_section, NAMEOF(config->logging_enabled), config->logging_enabled);
    write_config_value_bool(config_path, interface_section, NAMEOF(config->close_to_tray), config->close_to_tray);
    write_config_value_bool(config_path, interface_section, NAMEOF(config->start_in_tray), config->start_in_tray);
    write_config_value_bool(config_path, interface_section, NAMEOF(config->auto_startup), config->auto_startup);
    write_config_value_bool(config_path, interface_section, NAMEOF(config->use_mini_flist), config->use_mini_flist);
    write_config_value_bool(config_path, interface_section, NAMEOF(config->filter), config->filter);
    write_config_value_bool(config_path, interface_section, NAMEOF(config->magic_flist_enabled), config->magic_flist_enabled);

    // av
    write_config_value_bool(config_path, av_section, NAMEOF(config->push_to_talk), config->push_to_talk);
    write_config_value_bool(config_path, av_section, NAMEOF(config->audio_filtering_enabled), config->audio_filtering_enabled);
    write_config_value_int(config_path, av_section, NAMEOF(config->audio_device_in), config->audio_device_in);
    write_config_value_int(config_path, av_section, NAMEOF(config->audio_device_out), config->audio_device_out);
    write_config_value_int(config_path, av_section, NAMEOF(config->video_fps), config->video_fps);
    // TODO: video_input_device

    // notifications
    write_config_value_bool(config_path, notifications_section, NAMEOF(config->audible_notifications_enabled), config->audible_notifications_enabled);
    write_config_value_bool(config_path, notifications_section, NAMEOF(config->status_notifications), config->status_notifications);
    write_config_value_bool(config_path, notifications_section, NAMEOF(config->no_typing_notifications), config->no_typing_notifications);
    write_config_value_int(config_path, notifications_section, NAMEOF(config->group_notifications), config->group_notifications);

    // advanced
    write_config_value_bool(config_path, advanced_section, NAMEOF(config->enableipv6), config->enableipv6);
    write_config_value_bool(config_path, advanced_section, NAMEOF(config->disableudp), config->disableudp);
    write_config_value_bool(config_path, advanced_section, NAMEOF(config->proxyenable), config->proxyenable);
    write_config_value_int(config_path, advanced_section, NAMEOF(config->proxy_port), config->proxy_port);
    write_config_value_str(config_path, advanced_section, NAMEOF(config->proxy_ip), (const char *)config->proxy_ip);
    write_config_value_bool(config_path, advanced_section, NAMEOF(config->force_proxy), config->force_proxy);
    write_config_value_bool(config_path, advanced_section, NAMEOF(config->auto_update), config->auto_update);
    // TODO: block_friend_requests

    free(config_path);

    return true;
}

// TODO refactor to match same order in main.h
UTOX_SAVE *config_load(void) {
    UTOX_SAVE *save = utox_load_config();

    // TODO: Remove this in ~0.18.0 release
    if (!save) {
        LOG_NOTE("Settings", "New utox_save.ini not found. Trying old utox_save.");
        save = utox_data_load_utox();
    }

    if (!save) {
        LOG_ERR("Settings", "unable to load utox_save data");
        /* Create and set defaults */
        save = calloc(1, sizeof(UTOX_SAVE));
        if (!save) {
            LOG_FATAL_ERR(EXIT_MALLOC, "Settings", "Unable to malloc for default settings.");
        }

        save->enableipv6  = true;
        save->disableudp  = false;
        save->proxyenable = false;
        save->force_proxy = false;

        save->audio_filtering_enabled       = true;
        save->audible_notifications_enabled = true;
    }

    if (save->scale > 30) {
        save->scale = 30;
    } else if (save->scale < 5) {
        save->scale = 10;
    }

    if (save->window_width < MAIN_WIDTH) {
        save->window_width = MAIN_WIDTH;
    }
    if (save->window_height < MAIN_HEIGHT) {
        save->window_height = MAIN_HEIGHT;
    }

    /* UX Settings */

    dropdown_language.selected = dropdown_language.over = settings.language = save->language;

    dropdown_dpi.selected = dropdown_dpi.over = save->scale - 5;

    switch_save_chat_history.switch_on  = save->logging_enabled;
    switch_close_to_tray.switch_on      = save->close_to_tray;
    switch_start_in_tray.switch_on      = save->start_in_tray;
    switch_mini_contacts.switch_on      = save->use_mini_flist;
    switch_magic_sidebar.switch_on      = save->magic_flist_enabled;

    switch_ipv6.switch_on             = save->enableipv6;
    switch_udp.switch_on              = !save->disableudp;
    switch_udp.panel.disabled         = save->force_proxy;
    switch_proxy.switch_on            = save->proxyenable;
    switch_proxy_force.switch_on      = save->force_proxy;
    switch_proxy_force.panel.disabled = !save->proxyenable;

    switch_auto_startup.switch_on       = save->auto_startup;
    switch_auto_update.switch_on        = save->auto_update;

    settings.group_notifications = dropdown_global_group_notifications.selected =
        dropdown_global_group_notifications.over = save->group_notifications;

    switch_audible_notifications.switch_on = save->audible_notifications_enabled;
    switch_audio_filtering.switch_on       = save->audio_filtering_enabled;
    switch_push_to_talk.switch_on          = save->push_to_talk;
    switch_status_notifications.switch_on  = save->status_notifications;

    dropdown_theme.selected = dropdown_theme.over = save->theme;

    switch_typing_notes.switch_on = !save->no_typing_notifications;

    flist_set_filter(save->filter); /* roster list filtering */

    /* Network settings */
    settings.enable_ipv6 = save->enableipv6;
    settings.enable_udp  = !save->disableudp;
    settings.use_proxy   = !!save->proxyenable;
    settings.proxy_port  = save->proxy_port;
    settings.force_proxy = save->force_proxy;

    if (strlen((char *)save->proxy_ip) <= proxy_address_size){
        strcpy((char *)proxy_address, (char *)save->proxy_ip);
    }

    edit_proxy_ip.length = strlen((char *)save->proxy_ip);

    strcpy((char *)edit_proxy_ip.data, (char *)save->proxy_ip);

    if (save->proxy_port) {
        edit_proxy_port.length =
            snprintf((char *)edit_proxy_port.data, edit_proxy_port.maxlength + 1, "%u", save->proxy_port);
        if (edit_proxy_port.length >= edit_proxy_port.maxlength + 1) {
            edit_proxy_port.length = edit_proxy_port.maxlength;
        }
    }

    /* UX settings */
    settings.logging_enabled        = save->logging_enabled;
    settings.close_to_tray          = save->close_to_tray;
    settings.start_in_tray          = save->start_in_tray;
    settings.start_with_system      = save->auto_startup;
    settings.use_mini_flist         = save->use_mini_flist;
    settings.magic_flist_enabled    = save->magic_flist_enabled;

    settings.ringtone_enabled       = save->audible_notifications_enabled;
    settings.audiofilter_enabled    = save->audio_filtering_enabled;

    settings.send_typing_status     = !save->no_typing_notifications;
    settings.status_notifications   = save->status_notifications;

    settings.window_width           = save->window_width;
    settings.window_height          = save->window_height;

    settings.last_version           = save->utox_last_version;

    loaded_audio_out_device         = save->audio_device_out;
    loaded_audio_in_device          = save->audio_device_in;

    settings.auto_update            = save->auto_update;
    switch_auto_update.switch_on    = save->auto_update;
    settings.update_to_develop      = save->update_to_develop;
    settings.send_version           = save->send_version;

    settings.video_fps              = save->video_fps;

    edit_video_fps.length =
        snprintf((char *)edit_video_fps.data, edit_video_fps.maxlength + 1, "%u", save->video_fps);
    if (edit_video_fps.length > edit_video_fps.maxlength) {
        edit_video_fps.length = edit_video_fps.maxlength;
    }

    // TODO: Don't clobber (and start saving) commandline flags.

    // Allow users to override theme on the cmdline.
    // 0 is the default theme.
    // TODO: `utox -t default` is still broken.
    if (settings.theme == 0) {
        settings.theme              = save->theme;
    }

    ui_set_scale(save->scale);

    if (save->push_to_talk) {
        init_ptt();
    }

    return save;
}

// TODO refactor to match order in main.h
void config_save(UTOX_SAVE *save_in) {
    UTOX_SAVE *save = calloc(1, sizeof(UTOX_SAVE) + proxy_address_size);

    /* Copy the data from the in data to protect the calloc */
    save->window_x                      = save_in->window_x;
    save->window_y                      = save_in->window_y;
    save->window_width                  = save_in->window_width;
    save->window_height                 = save_in->window_height;

    save->save_version                  = UTOX_SAVE_VERSION;
    save->scale                         = ui_scale;
    save->proxyenable                   = switch_proxy.switch_on;
    save->audible_notifications_enabled = settings.ringtone_enabled;
    save->audio_filtering_enabled       = settings.audiofilter_enabled;
    save->push_to_talk                  = settings.push_to_talk;

    /* UX Settings */
    save->logging_enabled               = settings.logging_enabled;
    save->close_to_tray                 = settings.close_to_tray;
    save->start_in_tray                 = settings.start_in_tray;
    save->auto_startup                  = settings.start_with_system;
    save->use_mini_flist                = settings.use_mini_flist;
    save->magic_flist_enabled           = settings.magic_flist_enabled;

    save->video_fps                     = (settings.video_fps == 0) ? 25 : settings.video_fps;

    save->disableudp              = !settings.enable_udp;
    save->enableipv6              = settings.enable_ipv6;
    save->no_typing_notifications = !settings.send_typing_status;

    save->filter      = flist_get_filter();
    save->proxy_port  = settings.proxy_port;
    save->force_proxy = settings.force_proxy;

    save->audio_device_in  = dropdown_audio_in.selected;
    save->audio_device_out = dropdown_audio_out.selected;
    save->theme            = settings.theme;

    save->utox_last_version    = settings.curr_version;
    save->group_notifications  = settings.group_notifications;
    save->status_notifications = settings.status_notifications;

    save->auto_update           = settings.auto_update;
    save->update_to_develop     = settings.update_to_develop;
    save->send_version          = settings.send_version;

    save->language = settings.language;

    memcpy(save->proxy_ip, proxy_address, proxy_address_size);

    LOG_NOTE("uTox", "Writing uTox Save" );
    utox_save_config(save);

    // TODO: Remove this in ~0.18.0 release
    utox_data_save_utox(save, sizeof(UTOX_SAVE) + proxy_address_size);

    free(save);
}

// TODO: Remove this in ~0.18.0 release
bool utox_data_save_utox(UTOX_SAVE *data, size_t size) {
    FILE *fp = utox_get_file("utox_save", NULL, UTOX_FILE_OPTS_WRITE);

    if (!fp) {
        LOG_ERR("Settings", "Unable to open file for uTox settings.");
        return false;
    }

    if (fwrite(data, size, 1, fp) != 1) {
        LOG_ERR("Settings", "Unable to write uTox settings to file.");
        fclose(fp);
        return false;
    }

    flush_file(fp);
    fclose(fp);

    return true;
}

// TODO: Remove this in ~0.18.0 release
UTOX_SAVE *utox_data_load_utox(void) {
    size_t size = 0;
    FILE *fp = utox_get_file("utox_save", &size, UTOX_FILE_OPTS_READ);

    if (!fp) {
        LOG_ERR("Settings", "Unable to open utox_save.");
        return NULL;
    }

    UTOX_SAVE *save = calloc(1, size + 1);
    if (!save) {
        LOG_ERR("Settings", "Unable to malloc for utox_save.");
        fclose(fp);
        return NULL;
    }

    if (fread(save, size, 1, fp) != 1) {
        LOG_ERR("Settings", "Could not read save file");
        fclose(fp);
        free(save);
        return NULL;
    }

    fclose(fp);
    return save;
}
