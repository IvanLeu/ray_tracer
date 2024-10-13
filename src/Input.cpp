#include "Input.h"
#include <Window.h>

bool KeyboardState::KeyIsPressed(unsigned char code) const noexcept
{
    return keyStates[code];
}

std::optional<KeyboardState::Event> KeyboardState::ReadKey() noexcept
{
    if (!keyBuffer.empty())
    {
        auto e = keyBuffer.front();
        keyBuffer.pop();
        return e;
    }
    return {};
}

bool KeyboardState::KeyIsEmpty() const noexcept
{
    return keyBuffer.empty();
}

void KeyboardState::ClearKey() noexcept
{
    keyBuffer = std::queue<KeyboardState::Event>();
}

std::optional<unsigned char> KeyboardState::ReadChar() noexcept
{
    if (!charBuffer.empty())
    {
        auto c = charBuffer.front();
        charBuffer.pop();
        return c;
    }
    return {};
}

bool KeyboardState::CharIsEmpty() const noexcept
{
    return charBuffer.empty();
}

void KeyboardState::ClearChar() noexcept
{
    charBuffer = std::queue<unsigned char>();
}

void KeyboardState::Clear() noexcept
{
    ClearKey();
    ClearChar();
}

bool KeyboardState::AutorepeatEnabled() const noexcept
{
    return autorepeat;
}

void KeyboardState::EnableAutorepeat() noexcept
{
    autorepeat = true;
}

void KeyboardState::DisableAutorepeat() noexcept
{
    autorepeat = false;
}

void KeyboardState::OnKeyPressed(unsigned char code) noexcept
{
    keyStates[code] = true;
    keyBuffer.push(KeyboardState::Event{ KeyboardState::Event::Type::Press, code });
    TrimBuffer(keyBuffer);
}

void KeyboardState::OnKeyReleased(unsigned char code) noexcept
{
    keyStates[code] = false;
    keyBuffer.push(KeyboardState::Event{ KeyboardState::Event::Type::Release, code });
    TrimBuffer(keyBuffer);
}

void KeyboardState::OnChar(unsigned char code) noexcept
{
    charBuffer.push(code);
    TrimBuffer(charBuffer);
}

void KeyboardState::ClearState() noexcept
{
    keyStates.reset();
}

template<class T>
void KeyboardState::TrimBuffer(std::queue<T>& buffer) noexcept
{
    while (buffer.size() > bufferSize) {
        buffer.pop();
    }
}

bool MouseState::LeftIsPressed() const noexcept
{
    return leftIsPressed;
}

bool MouseState::MiddleIsPressed() const noexcept
{
    return middleIsPressed;
}

bool MouseState::RightIsPressed() const noexcept
{
    return rightIsPressed;
}

bool MouseState::IsInWindow() const noexcept
{
    return isInWindow;
}

std::pair<int, int> MouseState::GetPos() const noexcept
{
    return { x,y };
}

int MouseState::GetPosX() const noexcept
{
    return x;
}

int MouseState::GetPosY() const noexcept
{
    return y;
}

std::optional<MouseState::Event> MouseState::Read() noexcept
{
    if (!IsEmpty()) {
        auto e = buffer.front();
        buffer.pop();
        return e;
    }
    return {};
}

bool MouseState::IsEmpty() const noexcept
{
    return buffer.empty();
}

void MouseState::Clear() noexcept
{
    buffer = std::queue<Event>();
}

void MouseState::OnMouseMove(int x_in, int y_in) noexcept
{
    this->x = x_in;
    this->y = y_in;
    buffer.push(MouseState::Event{ MouseState::Event::Type::Move, *this });
    TrimBuffer();
}

void MouseState::OnMouseLeave() noexcept
{
    isInWindow = false;
    buffer.push(MouseState::Event{ MouseState::Event::Type::Leave, *this });
    TrimBuffer();
}

void MouseState::OnMouseEnter() noexcept
{
    isInWindow = true;
    buffer.push(MouseState::Event{ MouseState::Event::Type::Enter, *this });
    TrimBuffer();
}

void MouseState::OnLeftPressed([[maybe_unused]] int x_in, [[maybe_unused]] int y_in) noexcept
{
    leftIsPressed = true;
    buffer.push(MouseState::Event{ MouseState::Event::Type::LeftPress, *this });
    TrimBuffer();
}

void MouseState::OnMiddlePressed([[maybe_unused]] int x_in, [[maybe_unused]] int y_in) noexcept
{
    middleIsPressed = true;
    buffer.push(MouseState::Event{ MouseState::Event::Type::MiddlePress, *this });
    TrimBuffer();
}

void MouseState::OnRightPressed([[maybe_unused]] int x_in, [[maybe_unused]] int y_in) noexcept
{
    rightIsPressed = true;
    buffer.push(MouseState::Event{ MouseState::Event::Type::RightPress, *this });
    TrimBuffer();
}

void MouseState::OnLeftReleased([[maybe_unused]] int x_in, [[maybe_unused]] int y_in) noexcept
{
    leftIsPressed = false;
    buffer.push(MouseState::Event{ MouseState::Event::Type::LeftRelease, *this });
    TrimBuffer();
}

void MouseState::OnMiddleReleased([[maybe_unused]] int x_in, [[maybe_unused]] int y_in) noexcept
{
    middleIsPressed = false;
    buffer.push(MouseState::Event{ MouseState::Event::Type::MiddleRelease, *this });
    TrimBuffer();
}

void MouseState::OnRightReleased([[maybe_unused]] int x_in, [[maybe_unused]] int y_in) noexcept
{
    rightIsPressed = false;
    buffer.push(MouseState::Event{ MouseState::Event::Type::RightRelease, *this });
    TrimBuffer();
}

void MouseState::OnWheelUp([[maybe_unused]] int x_in, [[maybe_unused]] int y_in) noexcept
{
    buffer.push(MouseState::Event{ MouseState::Event::Type::WheelUp, *this });
    TrimBuffer();
}

void MouseState::OnWheelDown([[maybe_unused]] int x_in, [[maybe_unused]] int y_in) noexcept
{
    buffer.push(MouseState::Event{ MouseState::Event::Type::WheelDown, *this });
    TrimBuffer();
}

void MouseState::OnWheelDelta([[maybe_unused]] int x_in, [[maybe_unused]] int y_in, int delta) noexcept
{
    wheelDeltaCarry += delta;
    while (wheelDeltaCarry >= WHEEL_DELTA) {
        wheelDeltaCarry -= WHEEL_DELTA;
        OnWheelUp(x, y);
    }
    while (wheelDeltaCarry <= -WHEEL_DELTA) {
        OnWheelDown(x, y);
        wheelDeltaCarry += WHEEL_DELTA;
    }
}

void MouseState::TrimBuffer() noexcept
{
    while (buffer.size() > bufferSize) {
        buffer.pop();
    }
}
