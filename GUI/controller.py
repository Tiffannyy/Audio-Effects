# controller.py
# 
# Tiffany Liu
# 29 October 2025

import sys
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QTimer
from .view import Window
from .. import engine

class Controller:
    def __init__(self):
        self.app = QApplication(sys.argv)
        self.app.setQuitOnLastWindowClosed(False)
        self.engine = engine.Engine("plughw:2,0", "plughw:2,0")
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
        
        if self.window.stack.currentIndex() != ui.effectSel:
            self.window.list_widget.blockSignals(True)
            self.window.list_widget.setCurrentRow(ui.effectSel)
            self.window.stack.setCurrentIndex(ui.effectSel)
            self.window.list_widget.blockSignals(False)
        
        selIndex = -1

        if ui.effectSel == 1:
            selIndex = ui.tremSel
        elif ui.effectSel == 2:
            selIndex = ui.delaySel
        elif ui.effectSel == 3:
            selIndex = ui.reverbSel
        elif ui.effectSel == 4:
            selIndex = ui.bitcrushSel
        elif ui.effectSel == 5:
            selIndex = ui.odSel
        elif ui.effectSel == 6:
            selIndex = ui.distSel
        elif ui.effectSel == 7:
            selIndex = ui.fuzzSel
        
        panel = self.window.stack.currentWidget()

        if hasattr(panel, "update_from_engine"):
            panel.update_from_engine(params)
        
        if hasattr(panel, "highlight_dial"):
            if ui.menuMode == 1:
                panel.highlight_dial(selIndex, adjusting=False)
            elif ui.menuMode == 2:
                panel.highlight_dial(selIndex, adjusting=True)
            else:
                panel.highlight_dial(-1)
        self.window.update_from_engine(params, ui)
    
    def run(self):
        sys.exit(self.app.exec())

    def shutdown(self):
        self.engine.stop()
        self.app.quit()


if __name__ == "__main__":
    ctrl = Controller()
    ctrl.run()
