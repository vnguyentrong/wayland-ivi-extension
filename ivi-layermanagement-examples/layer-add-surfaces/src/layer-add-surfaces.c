/*
 * Copyright (C) 2015 Advanced Driver Information Technology Joint Venture GmbH
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <getopt.h>

#include "ilm_control.h"

t_ilm_uint screenWidth;
t_ilm_uint screenHeight;
static t_ilm_uint layer = 0;
pthread_mutex_t mutex;
static pthread_cond_t  waiterVariable = PTHREAD_COND_INITIALIZER;
static int number_of_surfaces = 0;
static int number_of_surfaces_1 = 0;
char display_name[256] = {0};

// modify
t_ilm_uint *arr;
t_ilm_uint overide_status = 0;
t_ilm_uint count_surface = 0;
t_ilm_uint count_arr = 0;
t_ilm_uint surface_x = 0;
t_ilm_uint surface_y = 0;
t_ilm_uint surface_order = 0;
t_ilm_uint surface_passed = 0;
t_ilm_uint square = 0;
t_ilm_uint total_loop = 0;
t_ilm_uint status_loop = 0;
t_ilm_uint total_square = 0;

/*check override between called surface and drawed surface*/
t_ilm_uint check_override(t_ilm_uint* arr, t_ilm_uint width, t_ilm_uint height)
{
    printf("In check overide \n");
    printf("number of surface: %d\n",number_of_surfaces_1);
    printf("surface order: %d\n",surface_order);
    t_ilm_uint size = 8*surface_order;
    printf("size of arr: %d \n",size);
    t_ilm_uint bounder_x = surface_x + width;
    t_ilm_uint bounder_y = surface_y + height;
    surface_passed = 0;
    if (size == 0)
    {
        overide_status = 1;
        return overide_status; 
    }
    else
    {
        for (t_ilm_uint i=0; i < size; i = i+8){
        if(arr[i+2] < surface_x){
            overide_status = 1;
            printf("x + width: %d = arr[%d], x_surface: %d \n",arr[i+2], i+2, surface_x);
            printf("pass:1 \n");
            //return overide_status;           
        }
        if(arr[i] > bounder_x){
            overide_status = 1;
            printf("x: %d = arr[%d], x_surface + width: %d \n",arr[i], i, bounder_x);
            printf("pass:2 \n");   
            //return overide_status;        
        }
        if(arr[i+5]< surface_y){
            overide_status = 1;
            printf("y + height: %d = arr[%d], y_surface: %d \n",arr[i+5], i+5, surface_y);
            printf("pass:3 \n");   
            //return overide_status;        
        }
        if(arr[i+1] > bounder_y){
            overide_status = 1;
            printf("y: %d = arr[%d], y_surface + height: %d \n",arr[i+1], i+1, bounder_y);
            printf("pass:4 \n");   
            //return overide_status;        
        }
        if (overide_status == 1)
        {
            surface_passed = surface_passed + 1;
            overide_status = 0;  
            if (surface_order == surface_passed)
            {
                overide_status = 1;
            }
        } 
        }
    }
    
    return overide_status;
}

/*check surface*/
void check_surface(t_ilm_uint* arr, t_ilm_uint width, t_ilm_uint height)
{
    t_ilm_uint status = 0;
    t_ilm_uint count_loop = 0;
	t_ilm_uint surface_square = 0;

	surface_square = width*height;
    srand((int)time(0));
    while(status == 0){
        surface_x = rand() % (screenWidth - width);
        surface_y = rand() % (screenHeight - height);
        
        printf("surface_x_in_random: %d \n",surface_x);
        printf("surface_y_in_random: %d \n",surface_y);

        //bounder_status = check_bounder(surface_x, surface_y, width, height);
        overide_status = check_override(arr, width, height);
        printf("overide: %d\n", overide_status);
        if (overide_status == 1 && surface_order == surface_passed)
        {
            status = 1;
        }
        count_loop = count_loop + 1;
        if(count_loop == total_loop | total_square < surface_square)
        {
            status_loop = 1;
            count_loop = 0;
            number_of_surfaces = number_of_surfaces + 1;
            goto end;
        }
    }

    count_surface = count_surface + 1;
    // point 1
    arr[count_arr + 0] = surface_x;
    arr[count_arr + 1] = surface_y;
    // point 2
    arr[count_arr + 2] = surface_x + width;
    arr[count_arr + 3] = surface_y;
    // point 3
    arr[count_arr + 4] = surface_x;
    arr[count_arr + 5] = surface_y + height;
    // point 4
    arr[count_arr + 6] = surface_x + width;
    arr[count_arr + 7] = surface_y + height;
    
    printf("surface_x_in_check_surface: %d \n",surface_x);
    printf("surface_y_in_check_surface: %d \n",surface_y);
    for(int i = 0; i < 8; i++){
        printf("a[%d]:%d \n",count_arr+i,arr[count_arr+i]);
    }
    count_arr =  count_arr + 8;
    overide_status = 0;
    surface_order  = surface_order + 1;
    square = square + width*height;
    end:
    printf("Do not enough space to draw \n");
    printf("Please type other surface with size smaller!\n");
}

static void configure_ilm_surface(t_ilm_uint id, t_ilm_uint width, t_ilm_uint height)
{
    printf("\nsurface: %d \n", count_surface);
    printf("surface_x_prev: %d \n",surface_x);
    printf("surface_y_prev: %d \n",surface_y);
    printf("width: %d \n", width);
    printf("height: %d \n", height);
    
    check_surface(arr, width, height);
    if(status_loop == 1)
    {
        status_loop = 0;
        goto end;
    }
    printf("surface_x_after: %d \n",surface_x);
    printf("surface_y_after: %d \n",surface_y);

    ilm_surfaceSetDestinationRectangle(id, surface_x, surface_y, width, height);
    ilm_surfaceSetSourceRectangle(id, 0, 0, width, height);
    ilm_surfaceSetVisibility(id, ILM_TRUE);
    ilm_layerAddSurface(layer,id);
    ilm_surfaceRemoveNotification(id);

    ilm_commitChanges();
    pthread_cond_signal( &waiterVariable );
    printf("layer-add-surfaces: surface (%u) configured with:\n"
           "    dst region: x:0 y:0 w:%u h:%u\n"
           "    src region: x:0 y:0 w:%u h:%u\n"
           "    visibility: TRUE\n"
           "    added to layer (%u)\n", id, width, height, width, height,layer);
    end:
    printf("\n");
}

static void surfaceCallbackFunction(t_ilm_uint id, struct ilmSurfaceProperties* sp, t_ilm_notification_mask m)
{
    if ((unsigned)m & ILM_NOTIFICATION_CONFIGURED)
    {
        configure_ilm_surface(id, sp->origSourceWidth, sp->origSourceHeight);
    }
}

static void callbackFunction(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void *user_data)
{
    (void)user_data;
    struct ilmSurfaceProperties sp;

    if (object == ILM_SURFACE) {
        if (created) {
            if (number_of_surfaces > 0) {
                number_of_surfaces--;
                printf("layer-add-surfaces: surface (%d) created\n",id);
                // always get configured event to follow the surface changings
                ilm_surfaceAddNotification(id,&surfaceCallbackFunction);
                ilm_commitChanges();
                ilm_getPropertiesOfSurface(id, &sp);

                if ((sp.origSourceWidth != 0) && (sp.origSourceHeight !=0))
                {   // surface is already configured
                    configure_ilm_surface(id, sp.origSourceWidth, sp.origSourceHeight);
                }
            }
        }
        else if(!created)
            printf("layer-add-surfaces: surface (%u) destroyed\n",id);
    } else if (object == ILM_LAYER) {
        if (created)
            printf("layer-add-surfaces: layer (%u) created\n",id);
        else if(!created)
            printf("layer-add-surfaces: layer (%u) destroyed\n",id);
    }
}

static void shutdownCallbackFunction(t_ilm_shutdown_error_type error_type,
                                     int errornum,
                                     void *user_data)
{
    (void) user_data;

    switch (error_type) {
        case ILM_ERROR_WAYLAND:
        {
            printf("layer-add-surfaces: exit, ilm shutdown due to wayland error: %s\n",
                   strerror(errornum));
            break;
        }
        case ILM_ERROR_POLL:
        {
            printf("layer-add-surfaces: exit, ilm shutdown due to poll error: %s\n",
                   strerror(errornum));
            break;
        }
        default:
        {
            printf("layer-add-surfaces: exit, ilm shutdown due to unknown reason: %s\n",
                   strerror(errornum));
        }
    }

    exit(1);
}

/* Choose the display with the largest resolution.*/
static t_ilm_uint choose_screen(void)
{
    struct ilmScreenProperties screenProperties;
    t_ilm_uint* screen_IDs = NULL;
    t_ilm_uint screen_ID = 0;
    t_ilm_uint screen_count = 0;
    t_ilm_uint choosen_width = 0;
    t_ilm_uint choosen_height = 0;
    t_ilm_uint i;

    ilm_getScreenIDs(&screen_count, &screen_IDs);

    for (i = 0; i<screen_count; i++)
    {
        ilm_getPropertiesOfScreen(screen_IDs[i], &screenProperties);
        if (!strcmp(screenProperties.connectorName, display_name)) {
            choosen_width = screenProperties.screenWidth;
            choosen_height = screenProperties.screenHeight;
            screen_ID = screen_IDs[i];
            break;
        }
        else if (screenProperties.screenWidth > choosen_width) {
            choosen_width = screenProperties.screenWidth;
            choosen_height = screenProperties.screenHeight;
            screen_ID = screen_IDs[i];
        }
    }

    screenWidth = choosen_width;
    screenHeight = choosen_height;

    free(screen_IDs);

    return screen_ID;
}

static int
usage(int ret)
{
    fprintf(stderr, "    -h,  --help                  display this help and exit.\n"
                    "    -d,  --display-name          name of the display which will be used,\n"
                    "                                 e.g.: HDMI-A-1, LVDS1\n"
                    "                                 If it is not set, display with highest resolution is used.\n"
                    "    -l,  --layer-id              id of the used ILM layer. It has to be set\n"
                    "    -s,  --surface-count         number of surfaces which will be added to\n"
                    "                                 the layer. It has to be set\n");
    exit(ret);
}

void parse_options(int argc, char *argv[])
{
    int opt;
    static const struct option options[] = {
        { "help",              no_argument, NULL, 'h' },
        { "layer-id",              required_argument, 0, 'l' },
        { "surface-count",           required_argument, 0, 's' },
        { "display-name", required_argument, NULL, 'd' },
        { 0,                   0,           NULL, 0 }
    };

    while (1) {
        opt = getopt_long(argc, argv, "hl:s:d:", options, NULL);

        if (opt == -1)
            break;

        switch (opt) {
            case 'h':
                usage(0);
                break;
            case 'l':
                layer = atoi(optarg);
                break;
            case 's':
                number_of_surfaces = atoi(optarg);
                break;
            case 'd':
                strcpy(display_name, optarg);
                break;
            default:
                usage(-1);
                break;
        }
    }

    printf("layer-add-surfaces: layer (%u) on display (%s) created, waiting for %d surfaces ...\n",
               layer,
               display_name,
               number_of_surfaces);
}

int main (int argc, char *argv[])
{
    // Get command-line options
    if ( argc < 3) {
        usage(-1);
    }

    // Check the first character of the first parameter
    if (!strncmp(argv[1], "-", 1)) {
        parse_options(argc, argv);
    } else {
        /* Convert a string to a long integer.  */
        layer = strtol(argv[1], NULL, 0);
        printf("Layer: %d\n", layer);
        number_of_surfaces = strtol(argv[2], NULL, 0);
        printf("number of surface: %d\n", number_of_surfaces);
    }
    
    // number_of_surface and layer must != 0
    if (!number_of_surfaces || !layer)
        usage(-1); 

    // declarare a attribute mutex
    pthread_mutexattr_t a;
    // check status init mutex: success = 0; fail != 0;
    if (pthread_mutexattr_init(&a) != 0)
    {
       return -1;
    }

    // check lock: infinity loop
    if (pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE) != 0)
    {
       pthread_mutexattr_destroy(&a);
       return -1;
    }
    // check initial mutex: success = 0; false != 0
    if (pthread_mutex_init(&mutex, &a) != 0)
    {
        pthread_mutexattr_destroy(&a);
        fprintf(stderr, "layer-add-surfaces: failed to initialize pthread_mutex\n");
        return -1;
    }

    pthread_mutexattr_destroy(&a);

    t_ilm_layer renderOrder[1];
    t_ilm_uint screen_ID;
    renderOrder[0] = layer;
    if (ilm_init() == ILM_FAILED) {
        fprintf(stderr, "layer-add-surfaces: ilm_init failed\n");
        return -1;
    }

    ilm_registerShutdownNotification(shutdownCallbackFunction, NULL);

    screen_ID = choose_screen();
    number_of_surfaces_1 = number_of_surfaces;
    printf("number_of_surfaces: %d", number_of_surfaces);
    t_ilm_uint size = 8*number_of_surfaces_1;
    arr = (t_ilm_uint*)malloc(size*sizeof(t_ilm_uint));
    total_loop = screenWidth*screenHeight;
	total_square = screenWidth*screenHeight;

    ilm_layerCreateWithDimension(&layer, screenWidth, screenHeight);
    printf("layer-add-surfaces: layer (%d) destination region: x:0 y:0 w:%u h:%u\n", layer, screenWidth, screenHeight);
    ilm_layerSetVisibility(layer,ILM_TRUE);
    printf("layer-add-surfaces: layer (%d) visibility TRUE\n", layer);
    ilm_displaySetRenderOrder(screen_ID, renderOrder, 1);
    ilm_commitChanges();
    ilm_registerNotification(callbackFunction, NULL);

    // if number_of_surface > 0 
    while (number_of_surfaces > 0) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait( &waiterVariable, &mutex);
    }

    ilm_unregisterNotification();
    ilm_destroy();
    free(arr);
    return 0;
}
