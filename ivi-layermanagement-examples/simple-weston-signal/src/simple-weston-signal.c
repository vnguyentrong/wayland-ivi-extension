#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <weston-simple-client-protocol.h>

static int running = 1;

typedef struct _WaylandContext {
        struct wl_display       *wl_display;
        struct wl_registry      *wl_registry;
        struct wl_compositor    *wl_compositor;
        struct weston_simple    *weston_simple;
}WaylandContextStruct;

static void
signal_format(void *data,
	      struct weston_simple *weston_simple,
	      uint32_t signal_response)
{
        if(signal_response == 1) {
                printf("[TEST] Server received a signal from client!\n");
        }
        else {
                printf("[TEST] Server did not receive a signal from client!\n");
        }
}

static struct weston_simple_listener signal_listenter = {
        signal_format
};

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                       const char *interface, uint32_t version)
{
        WaylandContextStruct* wlcontext = (WaylandContextStruct*)data;

        if (!strcmp(interface, "wl_compositor")) {
                wlcontext->wl_compositor =
                        wl_registry_bind(registry, name,
                                         &wl_compositor_interface, 1);
        }
        else if (!strcmp(interface, "weston_simple")) {
                wlcontext->weston_simple =
                        wl_registry_bind(registry, name,
                                         &weston_simple_interface, 1);
                weston_simple_add_listener(wlcontext->weston_simple, &signal_listenter, wlcontext);
        }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
                              uint32_t name)
{
        wl_registry_destroy(registry);
}

static const struct wl_registry_listener registry_listener = {
        registry_handle_global,
        registry_handle_global_remove
};

static int
init_wayland_context(WaylandContextStruct* wlcontext)
{
        wlcontext->wl_display = wl_display_connect(NULL);
        if (NULL == wlcontext->wl_display) {
                printf("Error: wl_display_connect failed\n");
                return -1;
        }

        wlcontext->wl_registry = wl_display_get_registry(wlcontext->wl_display);
        wl_registry_add_listener(wlcontext->wl_registry,
                                 &registry_listener, wlcontext);
        wl_display_roundtrip(wlcontext->wl_display);

        return 0;
}

static void
destroy_wayland_context(WaylandContextStruct* wlcontext)
{
        if(wlcontext->wl_compositor)
                wl_compositor_destroy(wlcontext->wl_compositor);

        if(wlcontext->wl_display)
                wl_display_disconnect(wlcontext->wl_display);
}

static void
destroy_signal(WaylandContextStruct* wlcontext)
{
        if (wlcontext->weston_simple)
                weston_simple_destroy(wlcontext->weston_simple);
}

static void
signal_int(int signum)
{
        running = 0;
}

int main (int argc, const char * argv[])
{
        unsigned int signal_send = 0;
        sscanf(argv[1], "%d", &signal_send);

        WaylandContextStruct* wlcontext;

        struct sigaction sigint;

        sigint.sa_handler = signal_int;
        sigemptyset(&sigint.sa_mask);
        sigaction(SIGINT, &sigint, NULL);
        sigaction(SIGTERM, &sigint, NULL);
        sigaction(SIGSEGV, &sigint, NULL);

        wlcontext = (WaylandContextStruct*)calloc(1, sizeof(WaylandContextStruct));

        if (init_wayland_context(wlcontext)) {
                fprintf(stderr, "init_wayland_context failed\n");
                goto ErrorContext;
        }

        weston_simple_send_mess(wlcontext->weston_simple, signal_send);

        wl_display_roundtrip(wlcontext->wl_display);

        destroy_signal(wlcontext);

ErrorContext:
        destroy_wayland_context(wlcontext);

        free(wlcontext);

        return 0;
}
