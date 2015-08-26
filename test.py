#! /usr/bin/env python

import logging
import serial
import sys
import json
from PyQt4 import QtGui
from PyQt4 import QtCore
from PyQt4.QtCore import pyqtSignal

log = logging.getLogger("Buzzer Test")
log.setLevel(logging.DEBUG)


class SerialReaderThread(QtCore.QThread):

    button_pressed = pyqtSignal(int)
    
    def __init__(self, serial_handler):
        super(SerialReaderThread, self).__init__()
        self.s = serial_handler
 
    def run(self):
        while True:
            try:
                line = self.s.readline()
                data = json.loads(line)
                self.button_pressed.emit(data['first_buzzer'])
            except ValueError:
                # If no valid json is found, ignore the ouput for testing purpose...
                pass


class Buzzer(QtGui.QWidget):
    
    def __init__(self, serial_handler):
        super(Buzzer, self).__init__()

        self.s = serial_handler
        self.initUI()

        # start serial reader thread
        self.serial_thread = SerialReaderThread(serial_handler)
        self.serial_thread.button_pressed.connect(self.button_pressed)
        self.serial_thread.start()

        # show widget
        self.setWindowTitle("Buzzer Test")
        self.show()
        
    def initUI(self):
        grid = QtGui.QGridLayout()
        self.setLayout(grid)

        # labels for pressed indicator
        self.buzzer_1_label = QtGui.QLabel("1")
        self.buzzer_2_label = QtGui.QLabel("2")
        self.buzzer_3_label = QtGui.QLabel("3")
        self.buzzer_4_label = QtGui.QLabel("4")

        # checkboxes for led handling
        self.buzzer_1_check = QtGui.QCheckBox()
        self.buzzer_2_check = QtGui.QCheckBox()
        self.buzzer_3_check = QtGui.QCheckBox()
        self.buzzer_4_check = QtGui.QCheckBox()

        self.buzzer_1_check.stateChanged.connect(self.set_buzzer_1_led)
        self.buzzer_2_check.stateChanged.connect(self.set_buzzer_2_led)
        self.buzzer_3_check.stateChanged.connect(self.set_buzzer_3_led)
        self.buzzer_4_check.stateChanged.connect(self.set_buzzer_4_led)

        # button for resetting the buzzer state
        self.reset_button = QtGui.QPushButton("Reset Buzzers")
        self.reset_button.clicked.connect(self.reset_buzzers)

        # fill the layout with the widgets
        grid.addWidget(self.buzzer_1_label, 0, 0)
        grid.addWidget(self.buzzer_2_label, 0, 1)
        grid.addWidget(self.buzzer_3_label, 0, 2)
        grid.addWidget(self.buzzer_4_label, 0, 3)

        grid.addWidget(self.buzzer_1_check, 1, 0)
        grid.addWidget(self.buzzer_2_check, 1, 1)
        grid.addWidget(self.buzzer_3_check, 1, 2)
        grid.addWidget(self.buzzer_4_check, 1, 3)

        grid.addWidget(self.reset_button, 2, 0, 1, 4)

    def reset_buzzers(self):
        """ All buzzers and their state is resetted and all led's are switched off"""
        self.s.write("RESET_BUZZERS\n")
        self.buzzer_1_check.setChecked(False)
        self.buzzer_2_check.setChecked(False)
        self.buzzer_3_check.setChecked(False)
        self.buzzer_4_check.setChecked(False)

        self.buzzer_1_label.setStyleSheet("")
        self.buzzer_2_label.setStyleSheet("")
        self.buzzer_3_label.setStyleSheet("")
        self.buzzer_4_label.setStyleSheet("")

    def set_led(self, buzzer, state):
        """ Helper method for enable/disable the led of a given buzzer"""
        if state:
            self.s.write("SET_LED %i 1\n" % buzzer)
        else:
            self.s.write("SET_LED %i 0\n" % buzzer)

    def set_buzzer_1_led(self, state):
        """ Callback when buzzer led 1 should be changed"""
        self.set_led(1, (state == QtCore.Qt.Checked))

    def set_buzzer_2_led(self, state):
        """ Callback when buzzer led 2 should be changed"""
        self.set_led(2, (state == QtCore.Qt.Checked))

    def set_buzzer_3_led(self, state):
        """ Callback when buzzer led 3 should be changed"""
        self.set_led(3, (state == QtCore.Qt.Checked))

    def set_buzzer_4_led(self, state):
        """ Callback when buzzer led 4 should be changed"""
        self.set_led(4, (state == QtCore.Qt.Checked))

    def button_pressed(self, buzzer):
        """ Slot which gets called from the serial thread when a buzzer gets pressed"""
        if buzzer == 1:
            self.buzzer_1_label.setStyleSheet("background-color: rgba(255, 0, 0, 255);")
            self.buzzer_1_check.setChecked(True)
        if buzzer == 2:
            self.buzzer_2_label.setStyleSheet("background-color: rgba(255, 0, 0, 255);")
            self.buzzer_2_check.setChecked(True)
        if buzzer == 3:
            self.buzzer_3_label.setStyleSheet("background-color: rgba(255, 0, 0, 255);")
            self.buzzer_3_check.setChecked(True)
        if buzzer == 4:
            self.buzzer_4_label.setStyleSheet("background-color: rgba(255, 0, 0, 255);")
            self.buzzer_4_check.setChecked(True)


def test():
    app = QtGui.QApplication(sys.argv)

    # TODO: Change the serial device accordingly to your plattform
    serial_device = '/dev/cu.usbmodem1411'
    s = serial.Serial(serial_device, 115200, timeout=2)
    b = Buzzer(s)
    sys.exit(app.exec_())


if __name__ == "__main__":
    import signal
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    test()
