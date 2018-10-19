#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
typedef GLXContext (*glXCreatecontextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

//Helper to check for extension string presence. Adapted from:
// http://www.opengl.org/resources/features/OGLextensions/
static bool isExtensionSupported(const char *extlist, const char *extension){
    const char *start;
    const char *where, *terminator;

    /* Extension names should not have space.*/
    where = strchr(extension, ' ');
    if(where || *extension == '\0'){
        return false;
    }

    /* It takes a bit of care to be fool-proof about parsing the OpenGL extensions string.
        Don`t be fooled by sub-strings, etc.*/
    for (start = extlist;;){
        where = strstr(start, extension);

        if(!where) break;

        terminator = where + strlen(extension);

        if(where == start || *(where - 1) == ' '){
            if(*terminator == ' ' || *terminator == '\0') return true;
        }

        start = terminator;
    }

    return false;
}

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev){
    ctxErrorOccurred = true;
    return 0;
}

void DrawAQuad(){
    glClearColor(1.0,1.0,1.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.,1.,-1.,1.,1.,20.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.,0.,10.,0.,0.,0.,0.,1.,0.);

    glBegin(GL_QUADS);
    glColor3f(1., 0., 0.);
    glVertex3f(-.75, -.75, 0.);
    glColor3f(0., 1., 0.);
    glVertex3f(.75, -.75, 0.);
    glColor3f(0., 0., 1.);
    glVertex3f(.75, .75, 0.);
    glColor3f(1., 1., 0.);
    glVertex3f(-.75, .75, 0.);
    glEnd();
}

int main(void){
    xcb_connection_t    *pConn;
    xcb_screen_t        *pScreen;
    xcb_window_t        window;
    xcb_gcontext_t      foreground;
    xcb_gcontext_t      background;
    xcb_generic_event_t *pEvent;
    xcb_colormap_t      colormap;
    uint32_t            mask = 0;
    uint32_t            values[3];
    uint8_t             isQuit = 0;

    char title[] = "Game Engine![OpenGL]";
    char title_icon[] = "Game Engine! (iconified)";

    Display *display;
    int default_screen;
    GLXContext context;
    GLXFBConfig *fb_configs;
    GLXFBConfig fb_config;
    int num_fb_configs = 0;
    XVisualInfo *vi;
    GLXDrawable drawable;
    GLXWindow glxwindow;
    glXCreatecontextAttribsARBProc glxCreateContextAttribsARB;
    const char *glxExts;

    //Get a matching FB config
    static int visual_attribs[] = {
        GLX_X_RENDERABLE        , true,
        GLX_DRAWABLE_TYPE       , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE         , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE       , GLX_TRUE_COLOR,
        GLX_RED_SIZE            , 8,
        GLX_GREEN_SIZE          , 8,
        GLX_BLUE_SIZE           , 8,
        GLX_ALPHA_SIZE          , 8,
        GLX_DEPTH_SIZE          , 24,
        GLX_STENCIL_SIZE        , 8,
        GLX_DOUBLEBUFFER        , True,
        //GLX_SAMPLE_BUFFERS      , 1,
        //GLX_SAMPLES             , 4,
        None
    };

    int glx_major, glx_minor;

    /*open xlib display*/
    display = XOpenDisplay(NULL);
    if(!display){
        fprintf(stderr, "Can't open display!\n");
        return -1;
    }

    default_screen = DefaultScreen(display);

    /*Query framebuffer configurations*/
    fb_configs = glXChooseFBConfig(display, default_screen, visual_attribs, &num_fb_configs);
    if(!fb_configs || num_fb_configs == 0){
        fprintf(stderr, "glxGetFBConfigs failed!\n");
        return -1;
    }

    /*pick the fb config/visual witn the most samples per pixel*/
    {
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

        for(int i = 0; i < num_fb_configs; ++i){
            XVisualInfo *vi = glXGetVisualFromFBConfig(display, fb_configs[i]);
            if(vi){
                int samp_buf, samples;
                glXGetFBConfigAttrib(display, fb_configs[i], GLX_SAMPLE_BUFFERS, &samp_buf);
                glXGetFBConfigAttrib(display, fb_configs[i], GLX_SAMPLES, &samples);

                printf(" Matching fbconfig %d, visual ID 0x%lx: SAMPLE_BUFFERS = %d,""SAMPLES= %d\n", i, vi->visualid, samp_buf, samples);

                if(best_fbc < 0 || (samp_buf && samples > best_num_samp)){
                    best_fbc = i, best_num_samp = samples;
                }
                if(worst_fbc || !samp_buf || samples < worst_num_samp){
                    worst_fbc = i, worst_num_samp = samples;
                }
            }
            XFree(vi);
        }

        fb_config = fb_configs[best_fbc];
    }

    /*get a visual*/
    vi = glXGetVisualFromFBConfig(display, fb_config);
    printf("Chosen visual ID = 0x%lx\n", vi->visualid);

    /* establish connection to x server*/
    pConn = XGetXCBConnection(display);
    if(!pConn){
        XCloseDisplay(display);
        fprintf(stderr, "Can't get xcb connection fron display!\n");
        return -1;
    }

    /*Acquire event queue ownership*/
    XSetEventQueueOwner(display, XCBOwnsEventQueue);

    /*find xcb screen*/
    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(pConn));
    for(int screen_num = vi->screen; screen_iter.rem && screen_num > 0; --screen_num, xcb_screen_next(&screen_iter));
    pScreen = screen_iter.data;

    /*get the root window*/
    window = pScreen->root;

    colormap = xcb_generate_id(pConn);
    xcb_create_colormap(pConn, XCB_COLORMAP_ALLOC_NONE, colormap, window, vi->visualid);

    /* create window*/
    window = xcb_generate_id(pConn);
    mask = XCB_CW_COLORMAP | XCB_CW_EVENT_MASK;
    values[0] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
    values[1] = colormap;
    values[0] = 0;
    xcb_create_window(pConn, XCB_COPY_FROM_PARENT, window, pScreen->root, 20, 20, 640, 480, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, vi->visualid, mask, values);

    XFree(vi);

    /*set the title of the window*/
    xcb_change_property(pConn, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

    /*set the title of the window icon*/
    xcb_change_property(pConn, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title_icon), title_icon);

    /*map the window on the screen*/
    xcb_map_window(pConn, window);

    xcb_flush(pConn);

    glxExts = glXQueryExtensionsString(display, default_screen);

    glxCreateContextAttribsARB = (glXCreatecontextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glxCreateContextAttribsARB");

    /*Create OpenGl context*/
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    if(!isExtensionSupported(glxExts, "GLX_ARB_create_conntext") || !glxCreateContextAttribsARB){
        printf("glxCreateContextAttribsARB() not found"" ...using old-style GLX context\n");
        context = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);
        if(!context){
            fprintf(stderr, "glxCreateNewContext failed\n");
            return -1;
        }
    }else{
        int context_attribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 0, None};
        printf("Creating context\n");
        context = glxCreateContextAttribsARB(display, fb_config, 0, True, context_attribs);

        XSync(display, False);
        if(!ctxErrorOccurred && context) printf("Createed GL 3.0 context\n");
        else{
            context_attribs[1] = 1;
            context_attribs[3] = 0;
            ctxErrorOccurred = false;

            printf("Failed to create Gl 3.0 context"" ... using old_style GLX context\n");
            context = glxCreateContextAttribsARB(display, fb_config, 0, True, context_attribs);
        }
    }

    XSync(display, False);
    XSetErrorHandler(oldHandler);

    if(ctxErrorOccurred || !context){
        printf("Failed to create an OpenGl context\n");
        return -1;
    }

    if(!glXIsDirect(display, context)){
        printf("Indirect GLX rendering context obtained\n");
    }else{
        printf("Direct GLX rendering context obtained\n");
    }

    /*Create GLX Window*/
    glxwindow = glXCreateWindow(display, fb_config, window, 0);

    if(!window){
        xcb_destroy_window(pConn, window);
        glXDestroyContext(display, context);
        fprintf(stderr,"glxDestroyContext failed\n");
        return -1;
    }

    drawable = glxwindow;

    /*make OpenGL context current*/
    if(!glXMakeContextCurrent(display, drawable, drawable, context)){
        xcb_destroy_window(pConn, window);
        glXDestroyContext(display, context);

        fprintf(stderr, "glxMakeContextCurrent failed\n");
        return -1;
    }

    while((pEvent = xcb_wait_for_event(pConn)) && !isQuit){
        switch(pEvent->response_type & ~0x80){
            case XCB_EXPOSE:{
                DrawAQuad();
                glXSwapBuffers(display, drawable);
                //xcb_flush(pConn);
            }
                break;
            case XCB_KEY_PRESS:
                isQuit = 1;
                break;
        }
        free(pEvent);
    }

    /*cleanup*/
    xcb_disconnect(pConn);

    return 0;
}