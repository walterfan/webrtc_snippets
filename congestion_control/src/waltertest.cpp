#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <string>
#include <iostream>
#include "detector.h"
#include "estimator.h"

#include <glib.h> 

GMainLoop* loop;

using namespace std;

gint counter = 10;

gboolean callback(gpointer arg)
{
    g_print(".");
    if(--counter ==0){
        g_print("\n");
        g_main_loop_quit(loop);
        return FALSE;
    }
    return TRUE;
}


int main(int argc, char** argv) {
    string command;

	if(argc > 1) {
		printf("execute command %s\n", argv[1]);
		
	} else {
		printf("usage: %s <command>, it runs a loop by default\n", argv[0]);
		
	}
	
	if(g_thread_supported() == 0)
        g_thread_init(NULL);

    g_print("create g_main_loop\n");
    loop = g_main_loop_new(NULL, FALSE);
	
	g_print("add a timer per 100ms\n");
    g_timeout_add(100,callback,NULL);
    
	g_print("start g_main_loop\n");
    g_main_loop_run(loop);
    
	g_print("end g_main_loop\n");
    g_main_loop_unref(loop);


	return 0;
}