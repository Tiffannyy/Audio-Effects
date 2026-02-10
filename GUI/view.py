# view.py
#
# Description:
# view file for audio effects GUI. Contains code for the main window,
# sidebar, and effect panels.
#
# Tiffany Liu
# 29 October 2025

import sys
from parameters import *
from PyQt5.QtCore import Qt, QSize
from PyQt5.QtGui import QFont
from PyQt5.QtWidgets import (
    QWidget,
    QDockWidget,
    QMainWindow,
    QApplication,
    QListWidget,
    QStackedWidget,
    QVBoxLayout,
    QLabel,
    QStyle,
    QSystemTrayIcon,
    QAction,
    QMenu,
    QSlider,
    QGroupBox,
    QDial
)


class Window(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("OmniPedal")
        self.showFullScreen()

        # TODO: Customize font
        self.sidebar_font = QFont("Arial", 18)

        self._add_dock()
        self._add_list()
        self._add_pages()

        # connect selection to panel
        self.list_widget.currentRowChanged.connect(self.stack.setCurrentIndex)
        self.list_widget.setCurrentRow(0)  # default selection

        self._add_sidebar()

        self._restore_tray()

    def _add_dock(self):
        self.dock = QDockWidget("Effects", self)
        self.dock.setAllowedAreas(Qt.LeftDockWidgetArea)
        # remove all dock features (minimize, exit)
        self.dock.setFeatures(QDockWidget.NoDockWidgetFeatures)
        # create container for list widget

    def _add_list(self):
        self.list_widget = QListWidget()
        self.list_widget.setFont(self.sidebar_font)
        items = ["Clean", "Tremolo", "Delay", "Reverb",
                 "Distortion", "Fuzz", "Overdrive"]
        self.list_widget.addItems(items)
        self.list_widget.setStyleSheet("QListWidget::item { padding: 8px; }")
        self.list_widget.currentRowChanged.connect(self._change_stacked_page)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.dock)

    def _add_pages(self):
        # stacked area in the main window
        self.pages = []
        self.stack = QStackedWidget(self)
        self.pages.append(CleanPanel())
        self.pages.append(TremoloPanel())
        self.pages.append(DelayPanel())
        self.pages.append(ReverbPanel())
        self.pages.append(BitcrushPanel())
        self.pages.append(OverdrivePanel())
        self.pages.append(DistortionPanel())
        self.pages.append(FuzzPanel())

        # populate stacked pages / panel
        for i in (self.pages):
            self.stack.addWidget(i)

    def _add_sidebar(self):
        self.sidebar_container = QWidget()
        self.sidebar_layout = QVBoxLayout(self.sidebar_container)
        self.sidebar_layout.setContentsMargins(8, 8, 8, 8)
        self.sidebar_layout.setSpacing(12)
        # insert the sidebar widget above the stacked widget contents
        container = QWidget()
        container_layout = QVBoxLayout()
        container_layout.setContentsMargins(0, 0, 0, 0)
        container_layout.addWidget(self.sidebar_container)
        container_layout.addWidget(self.stack)
        container.setLayout(container_layout)
        self.setCentralWidget(container)
        self.sidebar_layout.addWidget(self.list_widget)
        self.sidebar_layout.addStretch()  # Push everything to the top
        self.dock.setWidget(self.sidebar_container)

    def _restore_tray(self):
         # system tray (restore)
        self.tray = QSystemTrayIcon(self)
        icon = self.style().standardIcon(QStyle.SP_MediaPlay)
        self.tray.setIcon(icon)
        self.tray.setToolTip("Omni-Pedal - running in background")

        tray_menu = QMenu()
        restore_act = QAction("Restore", self)
        restore_act.triggered.connect(self.showFullScreen)
        tray_menu.addAction(restore_act)
        self.tray.setContextMenu(tray_menu)
        self.tray.show()
    
    def _change_stacked_page(self, index):
        self.stack.setCurrentIndex(index)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    # keep window running
    app.setQuitOnLastWindowClosed(False)
    window = Window()
    window.show()
    app.exec()
