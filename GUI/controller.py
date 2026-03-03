# controller.py
# 
# Tiffany Liu
# 29 October 2025

import sys
import signal
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QTimer
from .view import Window
from .. import engine

class Controller:
    def __init__(self):
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        self.app = QApplication(sys.argv)
        self.app.setQuitOnLastWindowClosed(False)
        self.engine = engine.Engine("plughw:3,0", "plughw:2,0")
        self.engine.start()

        self.window = Window(self.engine)
        self.window.showFullScreen()

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_gui)
        self.timer.start(50)

        self.app.aboutToQuit.connect(self.shutdown)
    
    def update_gui(self):
        params = self.engine.getParams()
        ui = self.engine.get_ui_state()
        self.window.update_from_engine(params, ui)
 
    def run(self):
        sys.exit(self.app.exec())

    def shutdown(self):
        self.timer.stop()
        self.engine.stop()


if __name__ == "__main__":
    ctrl = Controller()
    ctrl.run()
