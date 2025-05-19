﻿#pragma once

enum class MouseButton : uint8_t
{
	Left,
	Right,
	Middle
};

enum CursorState : uint8_t
{
	Normal,
	Hidden,
	Disabled
};

enum class Key : uint16_t
{
	// Letter keys
	A = 65 /* GLFW_KEY_A */,
	B = 66 /* GLFW_KEY_B */,
	C = 67 /* GLFW_KEY_C */,
	D = 68 /* GLFW_KEY_D */,
	E = 69 /* GLFW_KEY_E */,
	F = 70 /* GLFW_KEY_F */,
	G = 71 /* GLFW_KEY_G */,
	H = 72 /* GLFW_KEY_H */,
	I = 73 /* GLFW_KEY_I */,
	J = 74 /* GLFW_KEY_J */,
	K = 75 /* GLFW_KEY_K */,
	L = 76 /* GLFW_KEY_L */,
	M = 77 /* GLFW_KEY_M */,
	N = 78 /* GLFW_KEY_N */,
	O = 79 /* GLFW_KEY_O */,
	P = 80 /* GLFW_KEY_P */,
	Q = 81 /* GLFW_KEY_Q */,
	R = 82 /* GLFW_KEY_R */,
	S = 83 /* GLFW_KEY_S */,
	T = 84 /* GLFW_KEY_T */,
	U = 85 /* GLFW_KEY_U */,
	V = 86 /* GLFW_KEY_V */,
	W = 87 /* GLFW_KEY_W */,
	X = 88 /* GLFW_KEY_X */,
	Y = 89 /* GLFW_KEY_Y */,
	Z = 90 /* GLFW_KEY_Z */,

	// Function keys
	F1 = 290 /* GLFW_KEY_F1 */,
	F2 = 291 /* GLFW_KEY_F2 */,
	F3 = 292 /* GLFW_KEY_F3 */,
	F4 = 293 /* GLFW_KEY_F4 */,
	F5 = 294 /* GLFW_KEY_F5 */,
	F6 = 295 /* GLFW_KEY_F6 */,
	F7 = 296 /* GLFW_KEY_F7 */,
	F8 = 297 /* GLFW_KEY_F8 */,
	F9 = 298 /* GLFW_KEY_F9 */,
	F10 = 299 /* GLFW_KEY_F10 */,
	F11 = 300 /* GLFW_KEY_F11 */,
	F12 = 301 /* GLFW_KEY_F12 */,

	// Arrow keys
	UP = 265 /* GLFW_KEY_UP */,
	DOWN = 264 /* GLFW_KEY_DOWN */,
	RIGHT = 262 /* GLFW_KEY_RIGHT */,
	LEFT = 263 /* GLFW_KEY_LEFT */,

	// Numpad
	NUMLOCK = 282 /* GLFW_KEY_NUM_LOCK */,
	NUM0 = 320 /* GLFW_KEY_KP_0 */,
	NUM1 = 321 /* GLFW_KEY_KP_1 */,
	NUM2 = 322 /* GLFW_KEY_KP_2 */,
	NUM3 = 323 /* GLFW_KEY_KP_3 */,
	NUM4 = 324 /* GLFW_KEY_KP_4 */,
	NUM5 = 325 /* GLFW_KEY_KP_5 */,
	NUM6 = 326 /* GLFW_KEY_KP_6 */,
	NUM7 = 327 /* GLFW_KEY_KP_7 */,
	NUM8 = 328 /* GLFW_KEY_KP_8 */,
	NUM9 = 329 /* GLFW_KEY_KP_9 */,
	DECIMAL = 330 /* GLFW_KEY_KP_DECIMAL */,
	DIVIDE = 331 /* GLFW_KEY_KP_DIVIDE */,
	MULTIPLY = 332 /* GLFW_KEY_KP_MULTIPLY */,
	SUBSTRACT = 333 /* GLFW_KEY_KP_SUBTRACT */,
	ADD = 334 /* GLFW_KEY_KP_ADD */,

	// Modifiers
	LEFT_SHIFT = 340 /* GLFW_KEY_LEFT_SHIFT */,
	RIGHT_SHIFT = 344 /* GLFW_KEY_RIGHT_SHIFT */,
	LEFT_CTRL = 341 /* GLFW_KEY_LEFT_CONTROL */,
	RIGHT_CTRL = 345 /* GLFW_KEY_RIGHT_CONTROL */,
	LEFT_ALT = 342 /* GLFW_KEY_LEFT_ALT */,
	RIGHT_ALT = 346 /* GLFW_KEY_RIGHT_ALT */,

	// Miscellaneous
	HOME = 268 /* GLFW_KEY_HOME */,
	END = 269 /* GLFW_KEY_END */,
	PAGEUP = 266 /* GLFW_KEY_PAGE_UP */,
	PAGEDOWN = 267 /* GLFW_KEY_PAGE_DOWN */,
	CAPSLOCK = 280 /* GLFW_KEY_CAPS_LOCK */,
	SPACE = 32  /* GLFW_KEY_SPACE */,
	BACKSPACE = 259 /* GLFW_KEY_BACKSPACE */,
	INSERT = 260 /* GLFW_KEY_INSERT */,
	ESCAPE = 256 /* GLFW_KEY_ESCAPE */,
	PRINT_SCREEN = 283 /* GLFW_KEY_PRINT_SCREEN */,
	PAUSE = 284 /* GLFW_KEY_PAUSE */
};