from __future__ import division

import pygame, re
from pygame.locals import *

# TODO: Figure out where inversion should go (maybe have an InversionChannel)

# Intentions are direct gameplay actions controlled by input Channels
(INTENT_TRANS_X, INTENT_TRANS_Y, INTENT_TRANS_Z, INTENT_ROTATE_X, INTENT_ROTATE_Y, INTENT_ROTATE_Z,
INTENT_RUN_STANCE,
INTENT_UI_CONFIRM, INTENT_UI_BACK, INTENT_UI_X, INTENT_UI_Y,
INTENT_DEBUG_OPEN, INTENT_DEBUG_A_AXIS, INTENT_DEBUG_B_AXIS, INTENT_DEBUG_C_AXIS, INTENT_DEBUG_D_AXIS,
) = range(16)

DEAD_ZONE = 0.001 # How far from -1, 0, or 1 where we consider an axis to just be at those exact values

class Channel:
	"""A single source of simple input information, such as a particular keyboard key or gamepad axis."""
	
	def is_on(self):
		"""Returns True if, when considered as a digital button, the channel is on.

		For things that act like axes, this should return True whenever the axis leaves its neutral position.
		
		Must be implemented by subclass.
		"""
		raise NotImplemented
	
	def value(self):
		"""Returns a value in [-1,1] for this Channel when considered as an analog axis.
		
		Not all channels are capable of this entire range; for example, buttons (whether analog or digital) can only be in [0,1].
		
		Must be implemented by subclass.
		"""
		raise NotImplemented
	
	def desc(self):
		"""Returns a very short string describing this Channel.
		
		Must be implemented by subclass.
		"""
		raise NotImplemented


class ChannelSource:
	"""An object that represents a source for several distinct Channels, i.e. a keyboard or gamepad."""
	
	def update(self):
		"""Does whatever is necessary to update the Channels offered by this ChannelSource to the latest state.
		
		May optionally be implemented by subclass.
		"""
		pass

	def channels(self):
		"""Returns a sequence of all Channels offered by this ChannelSource.
		
		Must be implemented by subclass.
		"""
		raise NotImplemented


class Keyboard(ChannelSource):
	"""An object representing the keyboard, from which you can get KeyChannels.
	
	Data attributes:
	key_channels: A dictionary mapping pygame key constants to the same KeyChannels returned by channels().
	"""

	# Will not track keystrokes from keys with these names, since they've been known to cause problems
	IGNORE_KEYS = (
		'[-]',       # This key, whatever it is, seems to get locked on when I push PrintScreen on my laptop
		'numlock',   # This key always indicates pressed on my laptop
	)
	
	def __init__(self):
		self.update()
		self._channels = []
		self.key_channels = {}
		for keyconst, value in enumerate(self._pressed_dict):
			if pygame.key.name(keyconst) in self.IGNORE_KEYS:
				continue
			channel = KeyChannel(self, keyconst)
			self.key_channels[keyconst] = channel
			self._channels.append(channel)
	
	def update(self):
		self._pressed_dict = pygame.key.get_pressed()
	
	def channels(self):
		return self._channels


class KeyChannel(Channel):
	"""A Channel indicating the state of a particular keyboard key."""
	
	def __init__(self, _kbd, _keyconst):
		"""Creates a KeyChannel. Don't call this; use a Keyboard object to get KeyChannels instead."""
		self._kbd = _kbd
		self._keyconst = _keyconst
	
	def is_on(self):
		return self._kbd._pressed_dict[self._keyconst]
	
	def value(self):
		if self._kbd._pressed_dict[self._keyconst]:
			return 1.0
		return 0.0
	
	def desc(self):
		return "Key:%s" % pygame.key.name(self._keyconst)


class Gamepad(ChannelSource):
	"""An object representing a Gamepad, from which you can get GamepadButtonChannels and GamepadAxisChannels.
	
	Data attributes:
	axis_channels - A dictionary mapping axis indices to the same GamepadAxisChannels returned by channels().
	button_channels - A dictionary mapping button indices to the same GamepadButtonChannels returned by channels().
	"""
	
	# If a gamepad's name matches the regular expression, then we can use these more human-readable names for button indices
	_KNOWN_BTN_NAMES = {
		re.compile(r"PLAYSTATION\(R\)3") : {
			0  : "Select",
			3  : "Start",
			10 : "L1",
			8  : "L2",
			1  : "L3",
			11 : "R1",
			9  : "R2",
			2  : "R3",
			4  : "Up",
			7  : "Left",
			5  : "Right",
			6  : "Down",
			15 : "Square",
			14 : "X",
			13 : "Circle",
			12 : "Triangle",
			16 : "PS",
		}
	}
	
	# If a gamepad's name matches the regular expression, then we can use these more human-readable names for axis indices
	_KNOWN_AXIS_NAMES = {
		re.compile(r"PLAYSTATION\(R\)3") : {
			14: "L1",
			12: "L2",
			0:  "Left Stick X",
			1:  "Left Stick Y",
			15: "R1",
			13: "R2",
			2:  "Right Stick X",
			3:  "Right Stick Y",
			8:  "Up",
			11: "Left",
			9:  "Right",
			10: "Down",
			19: "Square",
			18: "X",
			17: "Circle",
			16: "Triangle",
		}
	}
	
	def __init__(self, js_num):
		"""Creates a Gamepad from the given numbered pygame joystick."""
		self._js = pygame.joystick.Joystick(js_num)
		self._js.init()
		self._numaxes = self._js.get_numaxes()
		self._numbtns = self._js.get_numbuttons()
		
		# TODO: Implement weird state detection, and come up with a better name for it
		# The PS3 controller, when first plugged in, acts kind of weird until you push the PS3 button
		# You can tell it's in this state because all axis readings return either a constant very near -1 or 0.0, though NOT the real neutral values
		# We assume we're in weirdState and report no axis data until we get numbers that look okay
		# Once we leave weirdState, then we can record real neutral values
		self._weirdState = True
		
		self._btn_names, self._axis_names = None, None
		jsname = self._js.get_name()
		for namepat, namedict in self._KNOWN_BTN_NAMES.iteritems():
			if namepat.search(jsname):
				self._btn_names = namedict
				break
		for namepat, namedict in self._KNOWN_AXIS_NAMES.iteritems():
			if namepat.search(jsname):
				self._axis_names = namedict
				break
		
		pygame.event.pump()
		self.update()
		self.axis_channels = {}
		self.button_channels = {}
		self._channels = []
		for idx in self._axes.iterkeys():
			channel = GamepadAxisChannel(self, idx)
			self.axis_channels[idx] = channel
			self._channels.append(channel)
		for idx in self._btns.iterkeys():
			channel = GamepadButtonChannel(self, idx)
			self.button_channels[idx] = channel
			self._channels.append(channel)
		self.set_neutral()
	
	def set_neutral(self):
		"""Takes the given gamepad state as neutral for all buttons and axes."""
		for channel in self._channels:
			channel.set_neutral()
	
	def update(self):
		self._axes = {}
		for i in xrange(self._numaxes):
			n = self._js.get_axis(i)
			if abs(n - 1) < DEAD_ZONE:
				n = 1
			elif abs(n + 1) < DEAD_ZONE:
				n = -1
			elif abs(n) < DEAD_ZONE:
				n = 0
			self._axes[i] = n
		
		self._btns = {}
		for i in xrange(self._numbtns):
			self._btns[i] = self._js.get_button(i)
	
	def channels(self):
		return self._channels


class GamepadButtonChannel(Channel):
	"""A Channel indicating the state of a digital gamepad button."""
	
	def __init__(self, gamepad, btn_num):
		"""Creates a channel for the given button. Don't call this; use a Gamepad object to get GamepadButtonChannels instead."""
		self._gamepad = gamepad
		self._btn_num = btn_num
	
	def set_neutral(self):
		"""Does nothing; this is just here to be compatible with GamepadAxisChannel."""
		pass
	
	def is_on(self):
		return self._gamepad._btns[self._btn_num]
	
	def value(self):
		if self._gamepad._btns[self._btn_num]:
			return 1.0
		return 0.0
	
	def desc(self):
		btn_name = self._btn_num
		if self._gamepad._btn_names is not None and self._btn_num in self._gamepad._btn_names:
			btn_name = self._gamepad._btn_names[btn_name]
		return "JoyBtn:%s" % btn_name


class GamepadAxisChannel(Channel):
	"""A Channel indicating the state of a gamepad axis."""
	
	def __init__(self, gamepad, axis_num):
		"""Creates a channel for the given axis. Don't call this; use a Gamepad object to get GamepadAxisChannels instead."""
		self._gamepad = gamepad
		self._axis_num = axis_num
		self._neutral = 0.0
	
	def set_neutral(self):
		"""Takes the current axis state as neutral."""
		self._neutral = self._gamepad._axes[self._axis_num]
	
	def is_on(self):
		if abs(self._neutral - self._gamepad._axes[self._axis_num]) > DEAD_ZONE:
			return True
		return False
	
	def value(self):
		# TODO: Return 0 if we're in weird state (once weird state detection is implemented...)
		v = self._gamepad._axes[self._axis_num]
		# We assume here that neutral is either -1 or 0; I can't think of a situation where it would be something else
		# If it's -1, then we are going to treat this axis as though it goes from 0 (at real value -1) to 1 (at real value 1)
		if self._neutral == -1:
			v = (v+1)/2
		return v
	
	def desc(self):
		axis_name = self._axis_num
		if self._gamepad._axis_names is not None and self._axis_num in self._gamepad._axis_names:
			axis_name = self._gamepad._axis_names[axis_name]
		return "JoyAxis:%s" % axis_name


class PseudoAxisChannel(Channel):
	"""A Channel that takes two existing Channel objects and uses their values as either side of a logical axis.
	
	The PseudoAxisChannel's value() is 0 if neither of the two sub-channels are on, or if both are on.
	If only one is on, then the value() is that one's value(), though *-1 if it's the negative channel.
	
	Data attributes:
	negative - The negative side Channel object.
	positive - The positive side Channel object.
	"""
	def __init__(self, negative, positive):
		self.negative = negative
		self.positive = positive
	
	def is_on(self):
		return abs(self.value()) > DEAD_ZONE
	
	def value(self):
		pos_on = self.positive.is_on()
		neg_on = self.negative.is_on()
		if pos_on != neg_on:
			if pos_on:
				return self.positive.value()
			else:
				return -self.negative.value()
		return 0
	
	def desc(self):
		return "%s/%s" % (self.negative.desc(), self.positive.desc())


class MultiOrChannel(Channel):
	"""A Channel that merges multiple Channels into one logical channel.
	
	The MultiOrChannel is on if any of the sub-Channels are on. The MultiOrChannel's value is the sub-Channel value farthest from zero (highest abs).
	
	Data attributes:
	channels - A list of sub-Channels.
	"""
	
	def __init__(self, channels):
		"""Creates a MultiOrChannel with the given list of sub-channels.
		
		The list is assigned, not copied.
		"""
		self.channels = channels
	
	def is_on(self):
		for c in self.channels:
			if c.is_on():
				return True
		return False
	
	def value(self):
		v = 0.0
		for c in self.channels:
			cand_v = c.value()
			if abs(cand_v) > abs(v):
				v = cand_v
		return v
	
	def desc(self):
		return ",".join([c.desc() for c in self.channels])


class MultiAndChannel(Channel):
	"""A Channel that checks the state of several sub-channels, and is only on if every sub-channel is on.
	
	The value of the ReqAllChannel is the sub-channel value closest to zero (smallest abs).
	
	Data attributes:
	channels - A list of sub-Channels.
	"""
	
	def __init__(self, channels):
		"""Creates a MultiAndChannel with the given list of sub-channels.
		
		The list is assigned, not copied.
		"""
		self.channels = channels
	
	def is_on(self):
		for c in self.channels:
			if not c.is_on():
				return False
		return True
	
	def value(self):
		v = None
		for c in self.channels:
			cand_v = c.value()
			if v is None or abs(cand_v) < abs(v):
				v = cand_v
		return v
	
	def desc(self):
		return "+".join([c.desc() for c in self.channels])


class InputManager:
	"""A class that handles input from the player and interprets it as their control intent.
	
	Responsible for gamepad, keyboard, and mouse.
	
	Data attributes:
	all_channels - A sequence of all Channels from known ChannelSources. Do not alter externally.
	intent_channels - A dictionary mapping INTENT_* constants to Channel objects.
	"""
	
	def __init__(self):
		pygame.joystick.init()
		
		self.all_channels = []
		
		self._keyboard = Keyboard()
		
		# TODO : Implement Gamepad and gamepad channels that work even when no gamepad attached, so that we can:
		# - Plug in or unplug gamepad while game is running, or even change gamepads (does pygame support this?)
		# - Not have to mess with InputManager or conf file parser to have it deal with whether or not configured gamepad is plugged in
		self._gamepad = Gamepad(0)
		
		for channel_src in (self._keyboard, self._gamepad):
			for channel in channel_src.channels():
				self.all_channels.append(channel)
		
		# TODO: Make this user-configurable
		self.intent_channels = {
			INTENT_TRANS_X : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_a], self._keyboard.key_channels[K_d]),
				self._gamepad.axis_channels[0],
			]),
			INTENT_TRANS_Y : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_s], self._keyboard.key_channels[K_w]),
				self._gamepad.axis_channels[1],
			]),
			INTENT_TRANS_Z : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_q], self._keyboard.key_channels[K_e]),
				PseudoAxisChannel(self._gamepad.axis_channels[12], self._gamepad.axis_channels[13]),
			]),
			INTENT_ROTATE_X : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_i], self._keyboard.key_channels[K_k]),
				self._gamepad.axis_channels[3],
			]),
			INTENT_ROTATE_Y : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_j], self._keyboard.key_channels[K_l]),
				self._gamepad.axis_channels[2],
			]),
			INTENT_ROTATE_Z : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_u], self._keyboard.key_channels[K_o]),
				PseudoAxisChannel(self._gamepad.axis_channels[14], self._gamepad.axis_channels[15]),
			]),
			INTENT_RUN_STANCE : MultiOrChannel([
				self._keyboard.key_channels[K_SPACE],
				MultiAndChannel([self._gamepad.axis_channels[14], self._gamepad.axis_channels[15]]),
			]),
			INTENT_UI_CONFIRM : MultiOrChannel([
				self._keyboard.key_channels[K_SPACE],
				self._keyboard.key_channels[K_RETURN],
			]),
			INTENT_UI_BACK : MultiOrChannel([
				self._keyboard.key_channels[K_ESCAPE],
			]),
			INTENT_UI_X : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_LEFT], self._keyboard.key_channels[K_RIGHT]),
			]),
			INTENT_UI_Y : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_DOWN], self._keyboard.key_channels[K_UP]),
			]),
			INTENT_DEBUG_OPEN : MultiOrChannel([
				self._keyboard.key_channels[K_BACKQUOTE],
			]),
			INTENT_DEBUG_A_AXIS : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_1], self._keyboard.key_channels[K_2]),
			]),
			INTENT_DEBUG_B_AXIS : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_3], self._keyboard.key_channels[K_4]),
			]),
			INTENT_DEBUG_C_AXIS : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_5], self._keyboard.key_channels[K_6]),
			]),
			INTENT_DEBUG_D_AXIS : MultiOrChannel([
				PseudoAxisChannel(self._keyboard.key_channels[K_7], self._keyboard.key_channels[K_8]),
			]),
		}
		
		# Treat relevant gameplay channels automatically as UI control channels
		self.intent_channels[INTENT_UI_X].channels.append(self.intent_channels[INTENT_TRANS_X])
		self.intent_channels[INTENT_UI_Y].channels.append(self.intent_channels[INTENT_TRANS_Y])
	
	def update(self):
		"""Calls update() on all the ChannelSources being managed.

		You should call this once per frame before checking the Channels.
		"""
		pygame.event.pump()
		self._keyboard.update()
		self._gamepad.update()
