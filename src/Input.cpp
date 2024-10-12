#include "Input.h"

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
