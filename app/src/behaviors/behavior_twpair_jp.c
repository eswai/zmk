/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_twpair_jp

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>

#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/keys.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct twpair_jp_continue_item {
    uint16_t page;
    uint32_t id;
    uint8_t implicit_modifiers;
};

struct behavior_twpair_jp_config {
    zmk_mod_flags_t mods;
    uint8_t index;
    uint8_t continuations_count;
    struct twpair_jp_continue_item continuations[];
};

struct behavior_twpair_jp_data {
    bool active;
};

static void activate_twpair_jp(const struct device *dev) {
    struct behavior_twpair_jp_data *data = dev->data;

    data->active = true;
}

static void deactivate_twpair_jp(const struct device *dev) {
    struct behavior_twpair_jp_data *data = dev->data;

    data->active = false;
}

static int on_twpair_jp_binding_pressed(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    const struct device *dev = device_get_binding(binding->behavior_dev);
    struct behavior_twpair_jp_data *data = dev->data;

    if (data->active) {
        deactivate_twpair_jp(dev);
    } else {
        activate_twpair_jp(dev);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_twpair_jp_binding_released(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_twpair_jp_driver_api = {
    .binding_pressed = on_twpair_jp_binding_pressed,
    .binding_released = on_twpair_jp_binding_released,
};

static int twpair_jp_keycode_state_changed_listener(const zmk_event_t *eh);

ZMK_LISTENER(behavior_twpair_jp, twpair_jp_keycode_state_changed_listener);
ZMK_SUBSCRIPTION(behavior_twpair_jp, zmk_keycode_state_changed);

static const struct device *devs[DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT)];

static bool twpair_jp_is_caps_includelist(const struct behavior_twpair_jp_config *config,
                                          uint16_t usage_page, uint8_t usage_id,
                                          uint8_t implicit_modifiers) {
    for (int i = 0; i < config->continuations_count; i++) {
        const struct twpair_jp_continue_item *continuation = &config->continuations[i];
        LOG_DBG("Comparing with 0x%02X - 0x%02X (with implicit mods: 0x%02X)", continuation->page,
                continuation->id, continuation->implicit_modifiers);

        if (continuation->page == usage_page && continuation->id == usage_id &&
            (continuation->implicit_modifiers &
             (implicit_modifiers | zmk_hid_get_explicit_mods())) ==
                continuation->implicit_modifiers) {
            LOG_DBG("Continuing capsword, found included usage: 0x%02X - 0x%02X", usage_page,
                    usage_id);
            return true;
        }
    }

    return false;
}

static bool twpair_jp_is_alpha(uint8_t usage_id) {
    return (usage_id >= HID_USAGE_KEY_KEYBOARD_A && usage_id <= HID_USAGE_KEY_KEYBOARD_Z);
}

static bool twpair_jp_is_numeric(uint8_t usage_id) {
    return (usage_id >= HID_USAGE_KEY_KEYBOARD_1_AND_EXCLAMATION &&
            usage_id <= HID_USAGE_KEY_KEYBOARD_0_AND_RIGHT_PARENTHESIS);
}

static void twpair_jp_enhance_usage(const struct behavior_twpair_jp_config *config,
                                    struct zmk_keycode_state_changed *ev) {
    if (ev->usage_page != HID_USAGE_KEY || !twpair_jp_is_alpha(ev->keycode)) {
        return;
    }

    LOG_DBG("Enhancing usage 0x%02X with modifiers: 0x%02X", ev->keycode, config->mods);
    ev->implicit_modifiers |= config->mods;
}

static int twpair_jp_keycode_state_changed_listener(const zmk_event_t *eh) {
    struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev == NULL || !ev->state) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    for (int i = 0; i < DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT); i++) {
        const struct device *dev = devs[i];
        if (dev == NULL) {
            continue;
        }

        struct behavior_twpair_jp_data *data = dev->data;
        if (!data->active) {
            continue;
        }

        const struct behavior_twpair_jp_config *config = dev->config;

        twpair_jp_enhance_usage(config, ev);

        if (!twpair_jp_is_alpha(ev->keycode) && !twpair_jp_is_numeric(ev->keycode) &&
            !is_mod(ev->usage_page, ev->keycode) &&
            !twpair_jp_is_caps_includelist(config, ev->usage_page, ev->keycode,
                                           ev->implicit_modifiers)) {
            LOG_DBG("Deactivating twpair_jp for 0x%02X - 0x%02X", ev->usage_page, ev->keycode);
            deactivate_twpair_jp(dev);
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

static int behavior_twpair_jp_init(const struct device *dev) {
    const struct behavior_twpair_jp_config *config = dev->config;
    devs[config->index] = dev;
    return 0;
}

#define twpair_jp_LABEL(i, _n) DT_INST_LABEL(i)

#define PARSE_BREAK(i)                                                                             \
    {                                                                                              \
        .page = ZMK_HID_USAGE_PAGE(i), .id = ZMK_HID_USAGE_ID(i),                                  \
        .implicit_modifiers = SELECT_MODS(i)                                                       \
    }

#define BREAK_ITEM(i, n) PARSE_BREAK(DT_INST_PROP_BY_IDX(n, continue_list, i))

#define KP_INST(n)                                                                                 \
    static struct behavior_twpair_jp_data behavior_twpair_jp_data_##n = {.active = false};         \
    static struct behavior_twpair_jp_config behavior_twpair_jp_config_##n = {                      \
        .index = n,                                                                                \
        .mods = DT_INST_PROP_OR(n, mods, MOD_LSFT),                                                \
        .continuations = {LISTIFY(DT_INST_PROP_LEN(n, continue_list), BREAK_ITEM, (, ), n)},       \
        .continuations_count = DT_INST_PROP_LEN(n, continue_list),                                 \
    };                                                                                             \
    DEVICE_DT_INST_DEFINE(n, behavior_twpair_jp_init, NULL, &behavior_twpair_jp_data_##n,          \
                          &behavior_twpair_jp_config_##n, APPLICATION,                             \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_twpair_jp_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)

#endif
