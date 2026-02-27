from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QLabel,
    QGroupBox,
    QDial
)
from PyQt5.QtCore import Qt

from abc import ABCMeta

class ABCWidgetMeta(ABCMeta, type(QWidget)):
    pass

class EffectPanel(QWidget, metaclass=ABCWidgetMeta):

    def __init__(self, effect:str):
        super().__init__()
        self.name = effect
        self.dials = {}
        self.params_keys= {}

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
        self.param_box.setLayout(self.param_box_layout)
        self.effect_layout.addWidget(self.param_box, stretch=1)

    def add_dial(self, name:str, val, min_val=0, max_val=100):
        dial = QDial()
        dial.setRange(min_val, max_val)
        dial.setValue(val)
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

    def highlight_dial(self, sel, adjusting=False):
        for i, (name, dial) in enumerate(self.dials.items()):
            if i == sel:
                if adjusting:
                    dial.setStyleSheet("border: 3px solid #00ff00;")
                else:
                    dial.setStyleSheet("border: 3px solid #ffaa00;")
            else:
                dial.setStyleSheet("")

    def update_from_engine(self, params):
        for name, dial in self.dials.items():
            key = self.params_keys.get(name)
            if not key or key not in params:
                continue

            value = params[key]

            normalized_keys = {
                "volume", "mix",
                "trem_freq", "trem_depth",
                "delay_feedback",
                "reverb_decay",
                "od_drive", "od_tone",
                "dist_drive", "dist_tone",
                "fuzz_drive", "fuzz_tone"
            }

            if key in normalized_keys:
                value = int(value * 100)

            dial.blockSignals(True)
            dial.setValue(int(value))
            dial.blockSignals(False)

class CleanPanel(EffectPanel):
    def __init__(self, effect='Clean'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Mix': 50
        }
        self.params_keys = {
            'Volume': 'volume',
            'Mix' : 'mix'
        }
        for name, val in self.params.items():
            self.add_dial(name, val)

class TremoloPanel(EffectPanel):
     def __init__(self, effect='Tremolo'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Mix': 50,
            'Frequency': 50,
            'Depth': 50,
        }
        self.params_keys = {
            'Volume': 'volume',
            'Mix' : 'mix',
            'Frequency': 'trem_freq',
            'Depth': 'trem_depth',
        }
        for name, val in self.params.items():
            self.add_dial(name, val, min_val=0, max_val=100)

class DelayPanel(EffectPanel):
     def __init__(self, effect='Delay'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Mix': 50,
            'Delay': 50,
            'Feedback': 50,
        }
        self.params_keys = {
            'Volume': 'volume',
            'Mix' : 'mix',
            'Delay': 'delay_ms',
            'Feedback': 'delay_feedback'
        }
        for name, val in self.params.items():
            self.add_dial(name, val, min_val=0, max_val=100)

class ReverbPanel(EffectPanel):
     def __init__(self, effect='Reverb'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Mix': 50,
            'Decay': 50,
        }
        self.params_keys = {
            'Volume': 'volume',
            'Mix' : 'mix',
            'Decay': 'reverb_decay'
        }
        for name, val in self.params.items():
            self.add_dial(name, val, min_val=0, max_val=100)

class BitcrushPanel(EffectPanel):
     def __init__(self, effect='Bitcrush'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Mix' : 50,
            'Downsample': 50,
            'Bit Depth': 50
        }
        self.params_keys = {
            'Volume': 'volume',
            'Mix' : 'mix',
            'Downsample': 'bitcrush_rate',
            'Bit Depth': 'bitcrush_depth'
        }
        for name, val in self.params.items():
            self.add_dial(name, val, min_val=0, max_val=100)

class OverdrivePanel(EffectPanel):
     def __init__(self, effect='Overdrive'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Mix' : 50,
            'Drive': 50,
            'Tone': 50
        }
        self.params_keys = {
            'Volume': 'volume',
            'Mix' : 'mix',
            'Drive': 'od_drive',
            'Tone': 'od_tone'
        }
        for name, val in self.params.items():
            self.add_dial(name, val, min_val=0, max_val=100)

class DistortionPanel(EffectPanel):
     def __init__(self, effect='Distortion'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Mix' : 50,
            'Drive': 50,
            'Tone': 50
        }
        self.params_keys = {
            'Volume': 'volume',
            'Mix' : 'mix',
            'Drive': 'dist_drive',
            'Tone': 'dist_tone'
        }
        for name, val in self.params.items():
            self.add_dial(name, val, min_val=0, max_val=100)

class FuzzPanel(EffectPanel):
     def __init__(self, effect='Fuzz'):
        super().__init__(effect)
        self.params = {
            'Volume': 50,
            'Mix' : 50,
            'Drive': 50,
            'Tone': 50
        }
        self.params_keys = {
            'Volume': 'volume',
            'Mix' : 'mix',
            'Drive': 'fuzz_drive',
            'Tone': 'fuzz_tone'
        }
        for name, val in self.params.items():
            self.add_dial(name, val, min_val=0, max_val=100)
