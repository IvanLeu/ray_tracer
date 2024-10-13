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
};

struct InputState {
	KeyboardState kbd;
	MouseState mouse;
};
