/*
** EPITECH PROJECT, 2026
** j-type/tests/client/Captions
** File description:
** Unit tests of the captions of the sounds
*/

#include <criterion/criterion.h>

#include <vector>

#include "jtype/common/Constants.hpp"
#include "scenes/Captions.hpp"

using jtype::Captions;
using jtype::HudText;
using jtype::Sound;

Test(client_captions, shows_nothing_before_a_sound)
{
    Captions captions;

    cr_assert_not(captions.update(0.016F));
    cr_assert(captions.layout().empty());
}

Test(client_captions, shows_a_line_for_a_sound_that_tells_something)
{
    Captions captions;

    captions.hear({Sound::EnemyShot});
    cr_assert(captions.update(0.016F));

    const std::vector<HudText> texts = captions.layout();

    cr_assert_eq(texts.size(), 1U);
    cr_assert_str_eq(texts[0].string.c_str(), "BYDO SHOT");
    // At the bottom of the screen
    cr_assert_gt(texts[0].position.y, jtype::WORLD_HEIGHT / 2.0F);
}

Test(client_captions, shows_nothing_for_what_the_screen_already_shows)
{
    Captions captions;

    captions.hear({Sound::PlayerShot, Sound::GameOver});
    cr_assert_not(captions.update(0.016F));
    cr_assert(captions.layout().empty());
}

Test(client_captions, shows_a_sound_heard_twice_on_one_line)
{
    Captions captions;

    captions.hear({Sound::Explosion, Sound::Explosion, Sound::PlayerHit});
    (void)captions.update(0.016F);

    const std::vector<HudText> texts = captions.layout();

    cr_assert_eq(texts.size(), 2U);
    cr_assert_str_eq(texts[0].string.c_str(), "EXPLOSION");
    cr_assert_str_eq(texts[1].string.c_str(), "PLAYER HIT");
    cr_assert_gt(texts[1].position.y, texts[0].position.y);
}

Test(client_captions, hides_a_line_after_its_duration)
{
    Captions captions;

    captions.hear({Sound::PlayerOut});
    (void)captions.update(0.016F);
    cr_assert_not(captions.update(jtype::CAPTION_DURATION / 2.0F));
    cr_assert(captions.update(jtype::CAPTION_DURATION));
    cr_assert(captions.layout().empty());
}

Test(client_captions, keeps_a_line_heard_again)
{
    Captions captions;

    captions.hear({Sound::EnemyShot});
    (void)captions.update(jtype::CAPTION_DURATION * 0.6F);
    captions.hear({Sound::EnemyShot});
    // Heard again before it fades, it starts its duration again: no change to draw
    cr_assert_not(captions.update(jtype::CAPTION_DURATION * 0.6F));
    cr_assert_eq(captions.layout().size(), 1U);
    cr_assert_eq(captions.layout()[0].tint.a, 255);
}

Test(client_captions, fades_a_line_out_before_hiding_it)
{
    Captions captions;

    captions.hear({Sound::EnemyShot});
    (void)captions.update(0.0F);
    cr_assert_eq(captions.layout()[0].tint.a, 255);
    // Halfway through the fade
    cr_assert(captions.update(jtype::CAPTION_DURATION - jtype::CAPTION_FADE / 2.0F));

    const std::vector<HudText> texts = captions.layout();

    cr_assert_eq(texts.size(), 1U);
    cr_assert_gt(texts[0].tint.a, 0);
    cr_assert_lt(texts[0].tint.a, 255);
    // Drawn again at each step of the fade
    cr_assert(captions.update(0.016F));
    cr_assert_lt(captions.layout()[0].tint.a, texts[0].tint.a);
}

Test(client_captions, a_line_heard_again_while_fading_is_opaque_again)
{
    Captions captions;

    captions.hear({Sound::Explosion});
    (void)captions.update(jtype::CAPTION_DURATION - jtype::CAPTION_FADE / 2.0F);
    captions.hear({Sound::Explosion});
    cr_assert(captions.update(0.016F));
    cr_assert_eq(captions.layout()[0].tint.a, 255);
}
