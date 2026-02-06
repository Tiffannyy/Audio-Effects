from view import (
                  QVBoxLayout,
                  QLabel,
                  QGroupBox,
                  QSize,
                  Qt,
                  QWidget,
                  QSlider,
                  QGroupBox
                  )

from abc import ABC, abstractmethod

class EffectPanel(QWidget, ABC):
    "PLACEHOLDER PANEL"
    # TODO: Populate with actual effect controls

    def __init__(self, effect:str):
        super().__init__()
        self.name = effect
        self.sliders = []

        layout = QVBoxLayout()
        label = QLabel(f"{self.name} effect settings go here.")
        label.setAlignment(Qt.AlignCenter)
        layout.addWidget(label)
        self.setLayout(layout)

    def add_sliders(self, name):
        pass
            
    def add_param_box(self):
        param_box = QGroupBox("Parameters")
        param_box.setFixedSize(QSize(300, 400))
        param_box_layout = QVBoxLayout()
        for slider in self.sliders:
            param_box_layout.addWidget(slider)
        param_box.setLayout(param_box_layout)
        self.layout.addWidget(param_box)

class CleanPanel(EffectPanel):
    def __init__(self, effect='Clean'):
        super().__init__(effect)
        self.params = ['Volume']
    
    #TODO: implement specific sliders for clean effect

