
#include "config.h"

#include <assert.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/slist.h>

struct pressed_key {
    sys_snode_t _header;
    uint32_t code;
};

K_MEM_SLAB_DEFINE(
    pressed_keys_list_mem, 
    sizeof(struct pressed_key), 
    CONFIG_APP_MAX_KEYS_REPORTED, 
    4
);

static sys_slist_t pressed_keys_list_hnd = SYS_SLIST_STATIC_INIT(pressed_keys_list_hnd);

K_MUTEX_DEFINE(mutex);

static bool is_list_full(void)
{
    return CONFIG_APP_MAX_KEYS_REPORTED == sys_slist_len(&pressed_keys_list_hnd);
}

static int add_key_to_list(uint32_t code, k_timeout_t timeout)
{
    int rv = 0;

    do {

        size_t n_keys_in_list = sys_slist_len(&pressed_keys_list_hnd);
        if (CONFIG_APP_MAX_KEYS_REPORTED <= n_keys_in_list) {
            rv = -ENOMEM;
            break;
        }

        struct pressed_key* chunk = NULL;
        rv = k_mem_slab_alloc(
            &pressed_keys_list_mem,
            (void**)&chunk,
            timeout
        );
        if (0 != rv) {
            break;
        }

        chunk->code = code;

        sys_slist_append(&pressed_keys_list_hnd, &(chunk->_header));


    } while(false);

    return rv;
}

static int remove_key_from_list(uint32_t code)
{
    int rv = 0;

    do {
        struct pressed_key* chunk;
        sys_snode_t* header;
        struct pressed_key* chunk_to_remove = NULL;

        SYS_SLIST_FOR_EACH_NODE(&pressed_keys_list_hnd, header) {
            chunk = CONTAINER_OF(header, struct pressed_key, _header);
            if (code == chunk->code) {
                chunk_to_remove = chunk;
                break;
            }
        }

        if (NULL != chunk_to_remove) {
            rv = (true == sys_slist_find_and_remove(
                &pressed_keys_list_hnd,
                &(chunk_to_remove->_header)
            )) ? 0 : -EIO;

            if (0 != rv) {
                break;
            }

            k_mem_slab_free(
                &pressed_keys_list_mem,
                chunk_to_remove
            );
        }

    } while (false);

    return rv;    
}

static int remove_oldest_key_from_list(void)
{
    sys_snode_t* header = sys_slist_get(&pressed_keys_list_hnd);
    struct pressed_key* chunk = CONTAINER_OF(header, struct pressed_key, _header);
    k_mem_slab_free(
        &pressed_keys_list_mem,
        chunk
    );
    return 0;
}

static bool is_key_in_list(uint32_t code)
{
    sys_snode_t* header;

    SYS_SLIST_FOR_EACH_NODE(&pressed_keys_list_hnd, header) {
         struct pressed_key* chunk = CONTAINER_OF(header, struct pressed_key, _header);
        if (code == chunk->code) {
            return true;
        }
    }

    return false;
}


int app_keyboard_report_key_press(uint32_t code, bool is_pressed, k_timeout_t timeout)
{
    int rv = 0;

    rv = k_mutex_lock(&mutex, timeout);
    if (0 != rv) {
        return rv;
    }

    do {

        if (is_pressed) {
            if (false == is_key_in_list(code)) {
                if (is_list_full()) {
                    rv = remove_oldest_key_from_list();
                    if (0 != rv) {
                        break;
                    }
                }
                rv = add_key_to_list(code, timeout);
            }
        }
        else
        {
            rv = remove_key_from_list(code);
            assert(-EIO != rv);
        }

        

    } while (false);

    k_mutex_unlock(&mutex);

    return rv;
}

int app_keyboard_get_pressed(uint32_t codes[CONFIG_APP_MAX_KEYS_REPORTED], size_t* len, k_timeout_t timeout)
{
    int rv = 0;
    assert(len);

    do {
        size_t index = 0;
        *len = 0;

        rv = k_mutex_lock(&mutex, timeout);
        if (0 != rv) {
            break;
        }

        sys_snode_t* header;
        SYS_SLIST_FOR_EACH_NODE(&pressed_keys_list_hnd, header) {
            struct pressed_key* chunk = CONTAINER_OF(header, struct pressed_key, _header);
            assert(index < CONFIG_APP_MAX_KEYS_REPORTED);
            codes[index] = chunk->code;
            index++;
        }
        *len = index;

        k_mutex_unlock(&mutex);

    } while (false);

    return rv;
}

