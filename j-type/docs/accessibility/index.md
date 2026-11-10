# Accessibility

This page lists, for each kind of disability, what J-Type does to let more people play, and what it does not do yet. A solution is **done** when the game on `main` has it.

## Where the solutions come from

We follow guidelines written by people who test games with disabled players, and the choices of games known for their accessibility:

- [Xbox Accessibility Guidelines](https://learn.microsoft.com/en-us/gaming/accessibility/guidelines): a checklist by topic, like input, text, audio and difficulty.
- [Game Accessibility Guidelines](https://gameaccessibilityguidelines.com/): the same topics sorted from basic to advanced, with examples.
- [The Last of Us Part II](https://www.playstation.com/en-us/games/the-last-of-us-part-ii/accessibility/): more than 60 options, among them full rebinding, a high-contrast mode and audio cues shown on screen.
- Celeste and its Assist Mode: the player slows the game down or makes it more forgiving, without leaving the game nor being judged for it.
- [AbleGamers](https://www.ablegamers.org/): a charity that helps disabled players and studios.

## Physical and motor

| Solution | Why | Status |
|----------|-----|--------|
| Few controls: four directions, one shoot key and a mute key | Fewer keys to reach and to hold | Done |
| Holding the shoot key keeps firing, at the rate of the cooldown | No need to press the key again and again | Done |
| AUTO FIRE in the settings: the ship fires without the shoot key | Holding a key for long is tiring or impossible for some players | Done |
| CONTROLS in the settings binds each action to another key | A player reaches the keys they can use, e.g. on one side of the keyboard | Done |
| A gamepad plays and moves through the menus, its inputs bound in CONTROLS too | A gamepad, or an adapted controller, instead of the keyboard | Done |

The engine reads the keyboard and the gamepad through actions that can be bound again ([Younity input](https://github.com/Rimapus/r-type/blob/main/younity-engine/docs/architecture/input.md)): the game binds them from its settings file.

## Visual

| Solution | Why | Status |
|----------|-----|--------|
| Each player has a ship and shots of its own color, the tag `P1` to `P4` over its ship, and `YOU` under the ship of this client | Each player finds their ship without telling the colors apart | Done |
| COLORBLIND in the settings corrects the colors of the window for protanopia, deuteranopia or tritanopia | Red and green, or blue and yellow, look the same to some players | Done |
| HIGH CONTRAST in the settings darkens the starfield | The ships, the Bydos and the shots stand out from the background | Done |
| The selected line of a menu is amber and between dashes | It is told apart without its color | Done |
| A text size option for the menus and the HUD | Small pixel-art text is hard to read | Not done yet |
| REDUCE FLASHES in the settings draws the explosions smaller and half see-through | Bright flashes can hurt, or trigger seizures | Done |

## Hearing

| Solution | Why | Status |
|----------|-----|--------|
| CAPTIONS in the settings writes a line at the bottom of the screen for each important sound, like a Bydo shot or a ship hit | Deaf and hard-of-hearing players get the same warning | Done |
| A notice under the HUD when a player leaves, `P2 DISCONNECTED` | The players know the game changed without a sound | Done |
| Separate volumes for the music and the effects, and a mute key on every screen | The effects stay audible over the music, or the game plays in silence | Done |

## Mental and cognitive

| Solution | Why | Status |
|----------|-----|--------|
| One goal and one rule: shoot the Bydos, avoid being hit | Few things to keep in mind | Done |
| A destroyed ship stays where it was and cannot be hurt for 2 seconds | Time to understand what happened before the next hit, with no jump across the screen | Done |
| HELP on the main menu recalls the goal of the game and each action with the keys and gamepad inputs bound | Nothing to remember from a previous game | Done |
| A pause | Time to rest or to look away; online, the server must agree to stop the game for everyone | Not done yet |
| A difficulty on the server: `easy` gives 5 lives and slower Bydo shots, less often; `hard` the opposite | Players play at their own pace | Done |

## The documentation itself

| Solution | Why | Status |
|----------|-----|--------|
| Colors checked for a contrast of 4.5:1 or more, in the light and the dark theme | The level asked by [WCAG 2.2](https://www.w3.org/TR/WCAG22/) for text | Done |
| A text alternative on each image | Screen readers read it | Done |
| Headings in order, one idea per section | Screen readers move from heading to heading | Done |
| Search, and a light or a dark theme | The reader finds a page fast, in the theme they read best | Done |
