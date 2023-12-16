/******* dioapphider *******
 * this shows a little square 
 * hovering over the square will run the user defined command
 * leaving the area of the square closes the user command
*/

#include <glib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "configsgen.c"
#include "externvars.h"
#include "outputtochar.h"
#include "subprocessrun.h"
#include "replacenthline.h"
#include "getvaluefromconf.h"

GtkApplication	*app;
GtkWidget		*windowR;
static gint		ifEnter = 0;
static pid_t	app_pid = -1;
GtkWidget		*popApphider;
GtkWidget		*entryappToOpen;
GtkAdjustment	*adjustmentPosx;
GtkAdjustment	*adjustmentPosy;
GtkAdjustment	*adjustmentSizeWidth;
GtkAdjustment	*adjustmentSizeHeight;

static void remove_button() {
	gtk_popover_popdown(GTK_POPOVER(popApphider));
	gtk_window_close(GTK_WINDOW(windowR));
	g_application_quit(G_APPLICATION(app));
}

static void close_save_cb(GtkWidget *window) {
	gint width = gtk_adjustment_get_value(adjustmentSizeWidth);
	gchar widthText[30];
	sprintf(widthText, "width=%d", width);
	replacenthline(pathToConfig, 1, (const char *)widthText);

	gint height = gtk_adjustment_get_value(adjustmentSizeHeight);
	gchar heightText[30];
	sprintf(heightText, "height=%d", height);
	replacenthline(pathToConfig, 2, (const char *)heightText);

	gint posx = gtk_adjustment_get_value(adjustmentPosx);
	gchar posxText[30];
	sprintf(posxText, "posx=%d", posx);
	replacenthline(pathToConfig, 3, (const char *)posxText);

	gint posy = gtk_adjustment_get_value(adjustmentPosy);
	gchar posyText[30];
	sprintf(posyText, "posy=%d", posy);
	replacenthline(pathToConfig, 4, (const char *)posyText);

	GtkEntryBuffer *buffEntryOpen;
	buffEntryOpen = gtk_entry_get_buffer(GTK_ENTRY(entryappToOpen));
	const char *entryOpenText = gtk_entry_buffer_get_text(buffEntryOpen);
	gchar openCMDText[777];
	snprintf(openCMDText, sizeof(openCMDText), "runCMD=%s", entryOpenText);
	replacenthline(pathToConfig, 5, (const char *)openCMDText);

	gtk_widget_set_size_request(popApphider, width, height);
	gtk_popover_set_offset(GTK_POPOVER(popApphider), posx, posy);
	gtk_popover_popdown(GTK_POPOVER(popApphider));
	gtk_popover_popup(GTK_POPOVER(popApphider));
	gtk_window_close(GTK_WINDOW(window));
}

static void on_right_click() {
	gint width				= get_int_value_from_conf(pathToConfig, "width");
	gint height				= get_int_value_from_conf(pathToConfig, "height");
	gint posx				= get_int_value_from_conf(pathToConfig, "posx");
	gint posy				= get_int_value_from_conf(pathToConfig, "posy");
	const gchar *runCMD		= get_char_value_from_conf(pathToConfig, "runCMD");

	windowR = gtk_window_new();
	gtk_window_set_title(GTK_WINDOW(windowR), "DioAppHider Settings");
	gtk_window_set_icon_name(GTK_WINDOW(windowR), "application-x-deb");

	GtkWidget *removeButton;
	removeButton = gtk_button_new_with_label("Remove this panel");
	gtk_widget_set_halign(removeButton, GTK_ALIGN_CENTER);
	gtk_widget_set_margin_start(removeButton, 10);
	gtk_widget_set_margin_end(removeButton, 10);

	GtkWidget *closeButton;
	closeButton = gtk_button_new_with_label("Close/Apply");
	gtk_widget_set_halign(closeButton, GTK_ALIGN_CENTER);
	gtk_widget_set_margin_start(closeButton, 10);
	gtk_widget_set_margin_end(closeButton, 10);

	// panel width
	GtkWidget *labelWidth;
	labelWidth = gtk_label_new("Panel Width");

	adjustmentSizeWidth = gtk_adjustment_new(width, 0, 700, 10, 1, 1);

	GtkWidget *entrySizeWidth;
	entrySizeWidth = gtk_spin_button_new(adjustmentSizeWidth, 1, 0);

	// panel height
	GtkWidget *labelHeight;
	labelHeight = gtk_label_new("Panel Height");

	adjustmentSizeHeight = gtk_adjustment_new(height, 0, 700, 10, 1, 1);

	GtkWidget *entrySizeHeight;
	entrySizeHeight = gtk_spin_button_new(adjustmentSizeHeight, 1, 0);

	GtkWidget *boxWidthHeight;
	boxWidthHeight = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxWidthHeight), TRUE);
	gtk_box_prepend(GTK_BOX(boxWidthHeight), labelWidth);
	gtk_box_append(GTK_BOX(boxWidthHeight), entrySizeWidth);
	gtk_box_append(GTK_BOX(boxWidthHeight), labelHeight);
	gtk_box_append(GTK_BOX(boxWidthHeight), entrySizeHeight);

	// panel position x
	GtkWidget *labelX;
	labelX = gtk_label_new("Position X");

	adjustmentPosx = gtk_adjustment_new(posx, 0, 4000, 10, 1, 1);

	GtkWidget *entryPosx;
	entryPosx = gtk_spin_button_new(adjustmentPosx, 1, 0);

	// panel position y
	GtkWidget *labelY;
	labelY = gtk_label_new("Position Y");

	adjustmentPosy = gtk_adjustment_new(posy, 0, 4000, 10, 1, 1);

	GtkWidget *entryPosy;
	entryPosy = gtk_spin_button_new(adjustmentPosy, 1, 0);

	GtkWidget *boxPosition;
	boxPosition = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxPosition), TRUE);
	gtk_box_prepend(GTK_BOX(boxPosition), labelX);
	gtk_box_append(GTK_BOX(boxPosition), entryPosx);
	gtk_box_append(GTK_BOX(boxPosition), labelY);
	gtk_box_append(GTK_BOX(boxPosition), entryPosy);

	//******* application to open when hovering over panel
	GtkWidget *appToOpen;
	appToOpen = gtk_label_new("App to open");
	gtk_widget_set_margin_start(appToOpen, 20);
	gtk_widget_set_halign(appToOpen, GTK_ALIGN_START);

	GtkEntryBuffer *buffappToOpen;
	buffappToOpen = gtk_entry_buffer_new(runCMD, -1);

	entryappToOpen = gtk_entry_new_with_buffer(buffappToOpen);
	gtk_widget_set_size_request(entryappToOpen, 340, 0);

	GtkWidget *boxAppToOpen;
	boxAppToOpen = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
	gtk_box_prepend(GTK_BOX(boxAppToOpen), appToOpen);
	gtk_box_append(GTK_BOX(boxAppToOpen), entryappToOpen);

	// main box that contains both vertical boxes
	GtkWidget *boxAll;
	boxAll = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_prepend(GTK_BOX(boxAll), boxWidthHeight);
	gtk_box_set_homogeneous(GTK_BOX(boxAll), TRUE);
	gtk_box_append(GTK_BOX(boxAll), boxPosition);
	gtk_box_append(GTK_BOX(boxAll), boxAppToOpen);
	gtk_box_append(GTK_BOX(boxAll), removeButton);
	gtk_box_append(GTK_BOX(boxAll), closeButton);

	g_signal_connect_swapped(removeButton, "clicked", G_CALLBACK(remove_button), windowR);
	g_signal_connect_swapped(closeButton, "clicked", G_CALLBACK(close_save_cb), windowR);

	gtk_window_set_child(GTK_WINDOW(windowR), boxAll);
	gtk_window_present(GTK_WINDOW(windowR));
}

// Action on entering active area (hovering mouse over the widget)
static void on_enter() {
	// action to run if hovered the first time over widget
	if (ifEnter == 0) {
		ifEnter = 1;
		const gchar *runCMD = get_char_value_from_conf(pathToConfig, "runCMD");
		char *args[30];

		// Copy the string to avoid modifying the original
		char copy[256];
		strncpy(copy, runCMD, sizeof(copy) - 1);
		copy[sizeof(copy) - 1] = '\0'; // Ensure null-terminated

		// Tokenize the string
		char *token = strtok(copy, " ");
		int i = 0;

		// Store tokens in the buffer
		while (token != NULL && i < 30) {
			args[i] = token;
			token = strtok(NULL, " ");
			i++;
		}

		// Null-terminate the array
		args[i] = NULL;

		pid_t pid = fork();

		if (pid == 0) {
			// Child process
			execvp(args[0], args);

			// If execvp fails
			perror("D:execvp");
			_exit(EXIT_FAILURE);
		}
		else if (pid > 0) {
			// Parent process
			app_pid = pid;
		}
		else {
			perror("fork");
		}
	}
	// action to run if hovered the second time over widget
	else {
		if (app_pid != -1) {
			ifEnter = 0;
			// If app was launched, send SIGTERM to the process
			kill(app_pid, SIGTERM);

			// Wait for the app process to finish
			int status;
			waitpid(app_pid, &status, 0);
			app_pid = -1; // Reset the process ID
		}
	}
}

/// activate signal handle
static void activate() {
	if (appstate == 1) {
		g_print("already running, only one instance allowed\n");
		return;
	}
	else {
		g_print("Welcome to DioAppHider!\n");
	}
	// sets flag that app is already running
	appstate = 1;
}

/// startup signal handle
static void startup(GtkApplication *app) {
	GtkWidget *window;
	window = gtk_application_window_new(GTK_APPLICATION(app));

	// get width and height from config
	int width = get_int_value_from_conf((char *)pathToConfig, "width");
	int height = get_int_value_from_conf((char *)pathToConfig, "height");
	int posx = get_int_value_from_conf((char *)pathToConfig, "posx");
	int posy = get_int_value_from_conf((char *)pathToConfig, "posy");

	popApphider = gtk_popover_new();
	gtk_widget_set_size_request(popApphider, width, height);
	gtk_popover_set_autohide(GTK_POPOVER(popApphider), FALSE);
	gtk_popover_set_has_arrow(GTK_POPOVER(popApphider), FALSE);
	gtk_popover_set_offset(GTK_POPOVER(popApphider), posx, posy);

	// adds right click action for new panel
	GtkGesture *rclick;
	rclick = gtk_gesture_click_new();

	GtkGestureSingle *rsingleclick;
	rsingleclick = GTK_GESTURE_SINGLE(rclick);

	gtk_gesture_single_set_button(rsingleclick, 3);

	GtkEventController *rightclick;
	rightclick = GTK_EVENT_CONTROLLER(rclick);
	gtk_event_controller_set_propagation_phase(rightclick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(popApphider, rightclick);

	g_signal_connect_swapped(rightclick, "pressed", G_CALLBACK(on_right_click), NULL);

	// adds mouse hover over actions
	GtkEventController *motionHoverEvent;
	motionHoverEvent = gtk_event_controller_motion_new();
	gtk_widget_add_controller(popApphider, motionHoverEvent);

	g_signal_connect_swapped(motionHoverEvent, "enter", G_CALLBACK(on_enter), NULL);
	//g_signal_connect_swapped(motionHoverEvent, "leave", G_CALLBACK(on_leave), NULL);

	gtk_window_set_child(GTK_WINDOW(window), popApphider);
	gtk_popover_popup(GTK_POPOVER(popApphider));
}	

int main() {
	// deifning HOME
	HOME = getenv("HOME");

	// create initial config
	create_configs();

	// setting config file
	snprintf((char *)pathToConfig, 777, "%s%s", HOME, configName);

	gint status;
	app = gtk_application_new("com.github.DiogenesN.dioapphider", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	g_signal_connect(app, "startup", G_CALLBACK(startup), NULL);
	status = g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);

	return status;
}	
