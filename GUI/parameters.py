from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QLabel,
    QGroupBox,
    QDial
)
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QGraphicsDropShadowEffect
from PyQt5.QtCore import QPropertyAnimation, QEasingCurve

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

        glow = QGraphicsDropShadowEffect()
        glow.setBlurRadius(0)
        glow.setOffset(0)
        container.setGraphicsEffect(glow)

        anim = QPropertyAnimation(container, b"maximumSize")
        anim.setDuration(150)
        anim.setEasingCurve(QEasingCurve.OutCubic)

        self.param_box_layout.addWidget(container)
        self.dials[name] = {
                "dial":dial,
                "container":container,
                "glow":glow,
                "anim":anim
                }
        return dial

    def highlight_dial(self, sel, adjusting=False):
        for i, item in enumerate(self.dials.values()):
            container = item["container"]
            glow = item["glow"]
            anim = item["anim"]

            if i == sel:
                if adjusting:
                    color = Qt.green
                    blur = 40
                    size = 110
                else:
                    color = Qt.orange
                    blur = 25
                    size = 100
            else:
                color = Qt.transparent
                blur = 0
                size = 90

            glow.setColor(color)
            glow.setBlurRadius(blur)

            anim.stop()
            anim.setStartValue(container.maximumSize())
            anim.setEndValue(container.sizeHint().expandedTo(Qt.QSize(size, size)))
            anim.start()

    def update_from_engine(self, params):
        for name, item in self.dials.values():
            dial = item["dial"]
            key = self.params_keys.get(dial)
            if not key or key not in params:
                continue

            value = params[key]

            min_val, max_val = self.param_range.get(key, (0, 1))
            dial_max = dial.maximum()
            dial_min = dial.minimum()

            scaled = (value - min_val) / (max_val - min_val)
            dial_scaled = (dial_min + scaled) * (dial_max - dial_min)

            dial.blockSignals(True)
            dial.setValue(int(dial_scaled))
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
        self.param_range = {
            'volume': (0.0,1.0),
            'mix': (0.0,1.0)
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
        self.param_range = {
            'volume': (0.0,1.0),
            'mix': (0.0,1.0),
            'trem_freq': (0.0,20.0),
            'trem_depth': (0.0,1.0)
        }
        for name, val in self.params.items():
            self.add_dial(name, val)

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
        self.param_range = {
            'volume': (0.0,1.0),
            'mix' : (0.0,1.0),
            'delay_ms': (0, 2000),
            'feedback': (0.0,1.0)
        }
        for name, val in self.params.items():
            self.add_dial(name, val)

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
        self.param_range = {
            'volume': (0.0,1.0),
            'mix': (0.0,1.0),
            'reverb_decay': (0.0,1.0)
        }
        for name, val in self.params.items():
            self.add_dial(name, val)

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
        self.param_range = {
            'volume': (0.0,1.0),
            'mix': (0.0,1.0),
            'bitcrush_rate': (0,20000),
            'bitcrush_depth': (0,20)
        }
        for name, val in self.params.items():
            self.add_dial(name, val)

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
        self.param_range = {
            'volume': (0.0,1.0),
            'mix': (0.0,1.0),
            'od_drive': (0.0,1.0),
            'od_tone': (0.0,1.0)
        }
        for name, val in self.params.items():
            self.add_dial(name, val)

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
        self.param_range = {
            'volume': (0.0,1.0),
            'mix': (0.0,1.0),
            'dist_drive': (0.0,1.0),
            'dist_tone': (0.0,1.0)
        }
        for name, val in self.params.items():
            self.add_dial(name, val)

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
        self.param_range = {
            'volume': (0.0,1.0),
            'mix': (0.0,1.0),
            'fuzz_drive': (0.0,1.0),
            'fuzz_tone': (0.0,1.0)
        }
        for name, val in self.params.items():
            self.add_dial(name, val)
