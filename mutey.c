// FROM: https://wiki.ubuntu.com/DesktopExperienceTeam/ApplicationIndicators
// https://github.com/dorkbox/SystemTray/blob/master/test/example.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

// MYNE
// gcc -Os `pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1` -I/usr/include/libappindicator3-0.1/ -l asound mutey.c -o mutey

//  gcc example.c `pkg-config --cflags --libs gtk+-2.0 appindicator-0.1` -I/usr/include/libappindicator-0.1/ -o example && ./example

// apt-get install libgtk-3-dev libappindicator3-dev
// NOTE: there will be warnings, but the file will build and run. NOTE: this will not run as root on ubuntu (no dbus connection back to the normal user)
//  gcc example.c `pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1` -I/usr/include/libappindicator3-0.1/ -o example && ./example

int running = 1;

void *update_mic(void *args) {
  AppIndicator *indicator = args;
  int is_rec;
  snd_mixer_t *handle;
  snd_mixer_selem_id_t *sid;

  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, "hw:1"); // C920
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);

  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, "Mic");
  snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

  while (running) {
    snd_mixer_handle_events(handle); // update
    snd_mixer_selem_get_capture_switch(elem, SND_MIXER_SCHN_MONO, &is_rec);
    //usleep(100000);
    usleep(70000);
    if (is_rec)
      app_indicator_set_icon(indicator, "/home/zlice/bin/mutey/grn_mic.svg");
    else
      app_indicator_set_icon(indicator, "/home/zlice/bin/mutey/red_mic.svg");
  }
  snd_mixer_close(handle);
} // update_mic

static void quit(GtkWidget *wid, gpointer data) {
  gtk_main_quit();
  running = 0;
} // quit

int main (int argc, char **argv) {
  // don't wana deal w/ string cats so

  GtkWidget *indicator_menu;
  GtkWidget *menu_quit;
  AppIndicator *indicator;

  gtk_init (&argc, &argv);

  indicator = app_indicator_new ("mutey",
                                 "/home/zlice/bin/mutey/red_mic.svg",
                                 APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_icon(indicator, "/home/zlice/bin/mutey/org_mic.svg");

  indicator_menu = gtk_menu_new();
  menu_quit = gtk_menu_item_new_with_label("Quit");
  //               connect to, what action triggers, what to call,  gobject?
  g_signal_connect(menu_quit, "activate", G_CALLBACK(quit), NULL);
  gtk_menu_shell_insert(GTK_MENU_SHELL(indicator_menu), menu_quit, 0);
  gtk_widget_show(menu_quit);
  app_indicator_set_menu(indicator, GTK_MENU(indicator_menu));
  gtk_widget_show_all(indicator_menu);

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, update_mic, indicator);

  gtk_main();

  pthread_join(thread_id, NULL); // waits for thread to complete

  return 0;
} // main
