#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <math.h>

#define SERV_ADDR 2200

Window root;
Display *display;

typedef struct {
    KeyCode keycode;
    int is_upper;
} key_info_t;

key_info_t parse_str(char *str)
{
    key_info_t info = {0, 0};

    //if(str[0] <= '9' && str[0] >= '0' || tolower(str[0]) <= 'z' && tolower(str[0]) >= 'a')
    info.keycode = XKeysymToKeycode(display, XStringToKeysym(str));
    info.is_upper = isupper(str[0]);
    return info;
}

void do_click(int rightclick, int down)
{
    XTestFakeButtonEvent(display, rightclick, down, 0);
    XFlush(display);
}

void do_x_stuff(float dx, float dy)
{
    int screenNumber = DefaultScreen(display);
    XEvent event;/* this structure will contain the event’s data, once received. */
    /* get info about current pointer position */
    XQueryPointer(display, RootWindow(display, DefaultScreen(display)),
        &event.xbutton.root, &event.xbutton.window,
        &event.xbutton.x_root, &event.xbutton.y_root,
        &event.xbutton.x, &event.xbutton.y,
        &event.xbutton.state);

    XWarpPointer(display, 0, root, 0, 0, 0, 0, event.xbutton.x + (int)dx, event.xbutton.y + (int)dy);
    XFlush(display);
}

void handle_data(char *data)
{
    const char *curLine = data;

    while(curLine)
    {
        char *nextLine = strchr(curLine, '\n');
        if(nextLine)
            *nextLine = '\0';
        if(strlen(curLine))
        {
            printf("%s\n", curLine);
            if(strcmp(curLine, "[ld]") == 0)
                do_click(1, 1);
            else if(strcmp(curLine, "[lu]") == 0)
                do_click(1, 0);
            else if(strcmp(curLine, "[rd]") == 0)
                do_click(3, 1);
            else if(strcmp(curLine, "[ru]") == 0)
                do_click(3, 0);
            else if(strcmp(curLine, "[sd]") == 0)
            {
                do_click(5, 1);
                do_click(5, 0);
            }
            else if(strcmp(curLine, "[su]") == 0)
            {
                do_click(4, 1);
                do_click(4, 0);
            }
            else if(strcmp(curLine, "[sl]") == 0)
            {
                do_click(6, 1);
                do_click(6, 0);
            }
            else if(strcmp(curLine, "[sr]") == 0)
            {
                do_click(7, 1);
                do_click(7, 0);
            }
            else if(curLine[0] == 'a')
            {
                char buf[10];
                sscanf(curLine, "a[%s ]", buf);
                key_info_t info = parse_str(buf);
                if(info.is_upper) XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Shift_L), 1, 0);
                XTestFakeKeyEvent(display, info.keycode, 1, 0);
                XTestFakeKeyEvent(display, info.keycode, 0, 0);
                if(info.is_upper) XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Shift_L), 0, 0);
                XFlush(display);
            }
            else
            {
                float x, y;
                sscanf(curLine, "[%f %f]", &x, &y);
                x *= abs(x < 0 ? floor(x / 3) : ceil(x / 3));
                y *= abs(y < 0 ? floor(y / 3) : ceil(y / 3));
                do_x_stuff(x, y);
            }
        }
        if(nextLine)
            *nextLine  = '\n';
        curLine = nextLine ? (nextLine + 1) : NULL;
    }
}

int main(int argc, char *argv[])
{
    display = XOpenDisplay(NULL);
    root = DefaultRootWindow(display);

    int socket_desc, new_socket, c, readsize;
    struct sockaddr_in server, client;
    char *message;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    server.sin_port = htons(argc > 1 ? atoi(argv[1]) : SERV_ADDR);

    printf("Starting server on port %d\n", argc > 1 ? atoi(argv[1]) : SERV_ADDR);

    bind(socket_desc, (struct sockaddr *)&server, sizeof(server));

    listen(socket_desc, 3);

    c = sizeof(struct sockaddr_in);
    while((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        puts("Got new connection");

        message = "Heyo!\n";
        write(new_socket, message, strlen(message));

        message = "Now what have you to say?\n";
        write(new_socket, message, strlen(message));
        char client_message[1000];
        while((readsize = recv(new_socket, client_message, 1000, 0)) > 0)
        {
            client_message[readsize] = 0;
            handle_data(client_message);
            //write(new_socket, client_message, strlen(client_message));
        }

        puts("Client disconnected");
        fflush(stdout);
    }

    XCloseDisplay(display);

    return 0;
}