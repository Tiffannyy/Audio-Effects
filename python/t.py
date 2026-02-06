# view.py
#
# Tiffany Liu
# 29 October 2025

import sys
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QWidget,
    QDockWidget,
    QMainWindow,
    QApplication,
    QListWidget,
    QStackedWidget,
    QVBoxLayout,
    QLabel,
    QPushButton,
    QHBoxLayout,
    QStyle,
    QSystemTrayIcon,
    QAction,
    QMenu,
)

class EffectPanel(QWidget):
    "PLACEHOLDER PANEL"

    def __init__(self, name: str):
        super().__init__()
        layout = QVBoxLayout()
        label = QLabel(f"{name} effect settings go here.")
        label.setAlignment(Qt.AlignCenter)
        layout.addWidget(label)
        self.setLayout(layout)


class Window(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Guitar Effects - Controller")
        self.resize(1000, 600)

        # Central stacked area where effect UIs will be displayed
        self.stack = QStackedWidget(self)
        self.setCentralWidget(self.stack)

        # Sidebar dock
        self.dock = QDockWidget("Effects", self)
        self.dock.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)
        self.list_widget = QListWidget()
        self.list_widget.addItems(["Clean", "Overdrive", "Fuzz", "Delay", "Reverb"])
        self.dock.setWidget(self.list_widget)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.dock)

        # Populate stacked pages
        for i in range(self.list_widget.count()):
            name = self.list_widget.item(i).text()
            self.stack.addWidget(EffectPanel(name))

        # Connect selection
        self.list_widget.currentRowChanged.connect(self.stack.setCurrentIndex)
        self.list_widget.setCurrentRow(0)

        # Toolbar-style toggle to collapse/hide the sidebar
        toggle_btn = QPushButton("Hide Sidebar")
        toggle_btn.setCheckable(True)
        toggle_btn.clicked.connect(self.toggle_sidebar)
        # place toggle in a small widget at top-left of central area
        top_widget = QWidget()
        top_layout = QHBoxLayout()
        top_layout.setContentsMargins(6, 6, 6, 6)
        top_layout.addWidget(toggle_btn)
        top_layout.addStretch()
        top_widget.setLayout(top_layout)

        # Insert the top widget above the stacked widget contents
        container = QWidget()
        container_layout = QVBoxLayout()
        container_layout.setContentsMargins(0, 0, 0, 0)
        container_layout.addWidget(top_widget)
        container_layout.addWidget(self.stack)
        container.setLayout(container_layout)
        self.setCentralWidget(container)

        # System tray icon so user can restore window when minimized
        self.tray = QSystemTrayIcon(self)
        icon = self.style().standardIcon(QStyle.SP_MediaPlay)
        self.tray.setIcon(icon)
        self.tray.setToolTip("Guitar Effects - running (cannot be closed)")
        # Restore action on tray
        tray_menu = QMenu()
        restore_act = QAction("Restore", self)
        restore_act.triggered.connect(self.showNormal)
        tray_menu.addAction(restore_act)
        self.tray.setContextMenu(tray_menu)
        self.tray.show()

    def toggle_sidebar(self, checked: bool):
        if checked:
            self.dock.hide()
            # change text on the toggle button (find it)
            sender = self.sender()
            if isinstance(sender, QPushButton):
                sender.setText("Show Sidebar")
        else:
            self.dock.show()
            sender = self.sender()
            if isinstance(sender, QPushButton):
                sender.setText("Hide Sidebar")

    def closeEvent(self, event):
        """Prevent the app from exiting; instead minimize to the dock/tray.

        This implements the "cannot exit, maybe minimized to the side" behavior.
        """
        # Minimize instead of closing
        self.showMinimized()
        # Keep the tray icon visible so the user can restore the window
        event.ignore()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    # On macOS ensure the app keeps running when windows are hidden
    app.setQuitOnLastWindowClosed(False)
    window = Window()
    window.show()
    app.exec()