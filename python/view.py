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
)


class Window(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Omni-Pedal - Effects GUI")
        self.showFullScreen()

        # TODO: customize font
        self.sidebar_font = QFont("Arial", 18)

        # stacked area in the main window
        self.stack = QStackedWidget(self)

        '''sidebar / dock'''
        self.dock = QDockWidget("Effects", self)
        self.dock.setAllowedAreas(Qt.LeftDockWidgetArea)
        # remove all dock features (minimize, exit)
        self.dock.setFeatures(QDockWidget.NoDockWidgetFeatures)
        # create container for list widget
        sidebar_container = QWidget()
        sidebar_layout = QVBoxLayout(sidebar_container)
        sidebar_layout.setContentsMargins(8, 8, 8, 8)
        sidebar_layout.setSpacing(12)

        # list widget
        self.list_widget = QListWidget()
        self.list_widget.setFont(self.sidebar_font)
        items = ["Clean", "Tremolo", "Delay", "Reverb",
                 "Distortion", "Fuzz", "Overdrive"]
        self.list_widget.addItems(items)
        self.list_widget.setStyleSheet("QListWidget::item { padding: 8px; }")

        # add list widget to container
        sidebar_layout.addWidget(self.list_widget)
        sidebar_layout.addStretch()  # Push everything to the top
        self.dock.setWidget(sidebar_container)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.dock)

        # populate stacked pages / panel
        for i in range(self.list_widget.count()):
            name = self.list_widget.item(i).text()
            self.stack.addWidget(EffectPanel(name))

        # connect selection to panel
        self.list_widget.currentRowChanged.connect(self.stack.setCurrentIndex)
        self.list_widget.setCurrentRow(0)  # default selection

        # insert the sidebar widget above the stacked widget contents
        container = QWidget()
        container_layout = QVBoxLayout()
        container_layout.setContentsMargins(0, 0, 0, 0)
        container_layout.addWidget(self.stack)
        container.setLayout(container_layout)
        self.setCentralWidget(container)

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

        '''    # override close event - minimize instead of fully exiting
        def closeEvent(self, event):
        self.showMinimized()
        event.ignore()
        '''

if __name__ == "__main__":
    app = QApplication(sys.argv)
    # keep window running
    app.setQuitOnLastWindowClosed(False)
    window = Window()
    window.show()
    app.exec()
