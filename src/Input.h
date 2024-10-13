#pragma once

#include <bitset>
#include <queue>
#include <optional>

class KeyboardState {
	friend class Window;
public:
	class Event {
	public:
		enum class Type {
			Press,
			Release,
			Invalid
		};
	public:
		Event(Type type, unsigned char code) noexcept
			:
			type(type),
			code(code)
		{}
		bool IsPress() const noexcept {
			return type == Type::Press;
		}
		bool IsRelease() const noexcept {
			return type == Type::Release;
		}
		bool IsValid() const noexcept {
			return type != Type::Invalid;
		}
		unsigned char GetCode() const noexcept {
			return code;
		}
	private:
		Type type;
		unsigned char code;
	};
public:
	KeyboardState() = default;
	KeyboardState(const KeyboardState&) = delete;
	KeyboardState& operator=(const KeyboardState&) = delete;
	// key stuff
	bool KeyIsPressed(unsigned char code) const noexcept;
	std::optional<Event> ReadKey() noexcept;
	bool KeyIsEmpty() const noexcept;
	void ClearKey() noexcept;
	// char stuff
	std::optional<unsigned char> ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void ClearChar() noexcept;
	void Clear() noexcept;
	// autorepeat stuff
	bool AutorepeatEnabled() const noexcept;
	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
private:
	void OnKeyPressed(unsigned char code) noexcept;
	void OnKeyReleased(unsigned char code) noexcept;
	void OnChar(unsigned char code) noexcept;
	void ClearState() noexcept;
	template<class T>
	void TrimBuffer(std::queue<T>& buffer) noexcept;
private:
	static constexpr unsigned int nKeys = 256u;
	static constexpr unsigned int bufferSize = 16u;
	std::bitset<nKeys> keyStates;
	std::queue<Event> keyBuffer;
	std::queue<unsigned char> charBuffer;
	bool autorepeat = false;
};

class MouseState {
	friend class Window;
public:
	class Event {
	public:
		enum class Type {
			Move,
			Enter,
			Leave,
			LeftPress,
			MiddlePress,
			RightPress,
			LeftRelease,
			MiddleRelease,
			RightRelease,
			WheelUp,
			WheelDown,
			Invalid
		};
	public:
		Event() = default;
		Event(Type type_in, int x_in, int y_in) noexcept
			:
			type(type_in),
			x(x_in),
			y(y_in)
		{}
		Event(Type type, const MouseState& parent) noexcept
			:
			type(type),
			x(parent.x),
			y(parent.y),
			leftIsPressed(parent.leftIsPressed),
			middleIsPressed(parent.middleIsPressed),
			rightIsPressed(parent.rightIsPressed)
		{}
		bool IsValid() const noexcept {
			return type != Type::Invalid;
		}
		Type GetType() const noexcept {
			return type;
		}
		std::pair<int, int> GetPos() const noexcept {
			return { x, y };
		}
		int GetPosX() const noexcept {
			return x;
		}
		int GetPosY() const noexcept {
			return y;
		}
		bool LeftIsPressed() const noexcept {
			return leftIsPressed;
		}
		bool MiddleIsPressed() const noexcept {
			return middleIsPressed;
		}
		bool RightIsPressed() const noexcept {
			return rightIsPressed;
		}
	private:
		int x;
		int y;
		bool leftIsPressed = false;
		bool middleIsPressed = false;
		bool rightIsPressed = false;
		Type type;
	};
public:
	MouseState() = default;
	MouseState(const MouseState&) = delete;
	MouseState& operator=(const MouseState&) = delete;
	bool LeftIsPressed() const noexcept;
	bool MiddleIsPressed() const noexcept;
	bool RightIsPressed() const noexcept;
	bool IsInWindow() const noexcept;
	std::pair<int, int> GetPos() const noexcept;
	int GetPosX() const noexcept;
	int GetPosY() const noexcept;
	std::optional<Event> Read() noexcept;
	bool IsEmpty() const noexcept;
	void Clear() noexcept;
private:
	void OnMouseMove(int x, int y) noexcept;
	void OnMouseLeave() noexcept;
	void OnMouseEnter() noexcept;
	void OnLeftPressed(int x, int y) noexcept;
	void OnMiddlePressed(int x, int y) noexcept;
	void OnRightPressed(int x, int y) noexcept;
	void OnLeftReleased(int x, int y) noexcept;
	void OnMiddleReleased(int x, int y) noexcept;
	void OnRightReleased(int x, int y) noexcept;
	void OnWheelUp(int x, int y) noexcept;
	void OnWheelDown(int x, int y) noexcept;
	void OnWheelDelta(int x, int y, int delta) noexcept;
	void TrimBuffer() noexcept;
private:
	static constexpr unsigned int bufferSize = 16u;
	int x = 0;
	int y = 0;
	int wheelDeltaCarry = 0;
	bool leftIsPressed = false;
	bool middleIsPressed = false;
	bool rightIsPressed = false;
	bool isInWindow = false;
	std::queue<Event> buffer;
};

struct InputState {
	KeyboardState kbd;
	MouseState mouse;
};
