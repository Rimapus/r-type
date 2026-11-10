/*
** EPITECH PROJECT, 2026
** j-type/client/src/scenes/MenuScreen
** File description:
** MenuScreen source file
*/

#include "scenes/MenuScreen.hpp"

#include <utility>

#include "render/Sprites.hpp"

namespace jtype {

MenuScreen::MenuScreen(Context &context, std::vector<std::string> lines, float top)
    : _context(context),
      _starfield(this->getRegistry()),
      _texts(this->getRegistry(), context.assets),
      _lines(this->getRegistry(), context.assets),
      _menu(std::move(lines), top)
{
    for (const StarfieldLayer &layer : STARFIELD_LAYERS) {
        this->_starfield.addLayer(context.assets.loadTexture(layer.path), SCROLL_SPEED * layer.parallax);
    }
    this->applyContrast();
    this->drawLines();
}

void MenuScreen::update(float dt)
{
    if (this->_context.paused) {
        return;
    }
    if (!this->interceptInput()) {
        this->readMenu();
    }
    this->_starfield.scroll(dt);
}

void MenuScreen::readMenu(void)
{
    const Controls &controls = this->_context.controls;

    if (controls.isMenuPressed(MenuAction::Up)) {
        this->_menu.move(-1);
        this->drawLines();
    } else if (controls.isMenuPressed(MenuAction::Down)) {
        this->_menu.move(1);
        this->drawLines();
    } else if (controls.isMenuPressed(MenuAction::Left)) {
        this->adjust(this->_menu.getSelected(), -1);
    } else if (controls.isMenuPressed(MenuAction::Right)) {
        this->adjust(this->_menu.getSelected(), 1);
    } else if (controls.isMenuPressed(MenuAction::Confirm)) {
        this->confirm(this->_menu.getSelected());
    } else if (controls.isMenuPressed(MenuAction::Back)) {
        this->back();
    }
}

void MenuScreen::adjust(std::size_t /* line */, int /* delta */)
{
}

void MenuScreen::back(void)
{
}

bool MenuScreen::interceptInput(void)
{
    return false;
}

void MenuScreen::setLine(std::size_t line, std::string text)
{
    this->_menu.setLine(line, std::move(text));
    this->drawLines();
}

void MenuScreen::write(const HudText &text)
{
    this->_texts.write(text);
}

void MenuScreen::applyContrast(void)
{
    this->_starfield.setColor(this->_context.settings.highContrast ? DIMMED_STARFIELD : younity::Color{});
}

Context &MenuScreen::getContext(void)
{
    return this->_context;
}

void MenuScreen::drawLines(void)
{
    this->_lines.clear();
    for (const HudText &text : this->_menu.layout()) {
        this->_lines.write(text);
    }
}

} // namespace jtype
