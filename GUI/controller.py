# controller.py
# 
# Tiffany Liu
# 29 October 2025

import sys
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QTimer
from view import Window
from .. import engine

class Controller:
    def __init__(self):
        self.app = QApplication(sys.argv)
        self.app.setQuitOnLastWindowClosed(False)
        self.engine = engine.Engine("hw:1,0", "hw:1,0")
        self.engine.start()

        self.window = Window(engine)
        self.window.showFullscreen()

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_gui)
        self.timer.start(50)

        self.app.aboutToQuit.connect(self.shutdown)
    
    def update_gui(self):
        params = self.engine.getParams()
        for p in self.window.pages:
            if hasattr(p, "update_from_engine"):
                p.update_from_engine(params)
    
    def run(self):
        sys.exit(self.app.exc())

    def shutdown(self):
        self.engine.stop()
        self.app.quit()  
        sys.exit()


if __name__ == "__main__":
    ctrl = Controller()
