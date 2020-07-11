#!/usr/bin/python3
import os
import signal
import gi
gi.require_version('Gtk', '3.0')
gi.require_version('AppIndicator3', '0.1')
from gi.repository import Gtk as gtk, GLib as glib, AppIndicator3 as appindicator, GObject
from subprocess import check_output
from threading import Thread
import time


class Indicator():
  def __init__(self):
    self.name = "mutey"
    self.pwd = os.path.dirname(os.path.realpath(__file__)) + "/"
    print(self.pwd)
    self.indicator = appindicator.Indicator.new(
      self.name, self.pwd+"red_mic.svg",
      appindicator.IndicatorCategory.OTHER
    )
    self.indicator.set_status(appindicator.IndicatorStatus.ACTIVE)
    self.indicator.set_menu(self.menu() )

    self.update = Thread(target=self.mic_icon)
    # daemonize the thread to make the indicator stopable
    self.update.setDaemon(True)
    self.update.start()

  def menu(self):
    menu = gtk.Menu()

    exittray = gtk.MenuItem(label='Exit')
    exittray.connect('activate', self.quit)
    menu.append(exittray)

    menu.show_all()
    return menu

  def mic_icon(self):
    while True:
      time.sleep(1)
      mic = check_output("amixer -c 1 sget Mic | grep off || true ", shell=True)
      if mic:
        glib.idle_add(self.indicator.set_icon,
          self.pwd+"red_mic.svg")
      else:
        glib.idle_add(self.indicator.set_icon,
          self.pwd+"grn_mic.svg")

  def quit(self, source):
    gtk.main_quit()

Indicator()
GObject.threads_init()
signal.signal(signal.SIGINT, signal.SIG_DFL)
gtk.main()
