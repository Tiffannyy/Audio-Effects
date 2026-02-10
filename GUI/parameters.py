from view import (
                  QVBoxLayout,
                  QLabel,
                  QGroupBox,
                  QSize,
                  Qt,
                  QWidget,
                  QSlider,
                  QGroupBox,
                  QDial
                  )

from abc import ABCMeta

class ABCWidgetMeta(ABCMeta, type(QWidget)):
    pass

class EffectPanel(QWidget, metaclass=ABCWidgetMeta):
    "PLACEHOLDER PANEL"
    # TODO: Populate with actual effect controls

    def __init__(self, effect:str):
        super().__init__()
        self.name = effect
        self.dials = {}

        self.effect_layout = QVBoxLayout()
        self.effect_layout.setContentsMargins(5, 5, 5, 5)
        self.effect_layout.setSpacing(5)
        self.setLayout(self.effect_layout)

        title = QLabel(f"{self.name} Effect")
        title.setAlignment(Qt.AlignCenter)
        title.setStyleSheet("font-weight: bold; font-size: 20px;")
        self.effect_layout.addWidget(title)

        self.param_box = QGroupBox("Parameters")
        self.param_box_layout = QVBoxLayout()
        self.param_box_layout.setSpacing(10)
        for dial in self.dials:
            self.param_box_layout.addWidget(dial)
        self.param_box.setLayout(self.param_box_layout)
        self.effect_layout.addWidget(self.param_box, stretch=1)

    def add_dial(self, name:str, min_val=0, max_val=100, init_val=50):
        dial = QDial()
        dial.setRange(min_val, max_val)
        dial.setValue(init_val)
        dial.setNotchesVisible(True)
        dial.setWrapping(False)
        dial.setFixedSize(80, 80)
        dial.setToolTip(name)

        label = QLabel(name)
        label.setAlignment(Qt.AlignCenter)

        container = QWidget()
        layout = QVBoxLayout()
        layout.setSpacing(5)
        layout.addWidget(dial)
        layout.addWidget(label)
        container.setLayout(layout)

        self.param_box_layout.addWidget(container)
        self.dials[name] = dial
        return dial

class CleanPanel(EffectPanel):
    def __init__(self, effect='Clean'):
        super().__init__(effect)
        self.params = {
            'Volume': 50
        }
        for name, val in self.params.items():
            self.add_dial(name, min_val=0, max_val=100, init_val=val)

class TremoloPanel(EffectPanel):
     def __init__(self, effect='Tremolo'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Frequency': 50,
            'Depth': 50,
            'Phase': 50
        }
        for name, val in self.params.items():
            self.add_dial(name, min_val=0, max_val=100, init_val=val)

class DelayPanel(EffectPanel):
     def __init__(self, effect='Delay'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Delay': 50,
            'Feedback': 50
        }
        for name, val in self.params.items():
            self.add_dial(name, min_val=0, max_val=100, init_val=val)

class ReverbPanel(EffectPanel):
     def __init__(self, effect='Reverb'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Taps': 50,
            'Decay': 50
        }
        for name, val in self.params.items():
            self.add_dial(name, min_val=0, max_val=100, init_val=val)

class BitcrushPanel(EffectPanel):
     def __init__(self, effect='Bitcrush'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Downsample': 50,
            'Bit Depth': 50
        }
        for name, val in self.params.items():
            self.add_dial(name, min_val=0, max_val=100, init_val=val)

class OverdrivePanel(EffectPanel):
     def __init__(self, effect='Overdrive'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Drive': 50,
            'Tone': 50
        }
        for name, val in self.params.items():
            self.add_dial(name, min_val=0, max_val=100, init_val=val)

class DistortionPanel(EffectPanel):
     def __init__(self, effect='Distortion'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Drive': 50,
            'Tone': 50
        }
        for name, val in self.params.items():
            self.add_dial(name, min_val=0, max_val=100, init_val=val)

class FuzzPanel(EffectPanel):
     def __init__(self, effect='Fuzz'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Drive': 50,
            'Tone': 50
        }
        for name, val in self.params.items():
            self.add_dial(name, min_val=0, max_val=100, init_val=val)





    #TODO: implement specific sliders for clean effect

