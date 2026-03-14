# Griljor Web Rewrite — ToDo list

## Maps
- [ ] fix initial spawn in locked room without a key in castle map

## Bugs in Play
- [x] ask the AI to compare the old code and check systematically for differences in implementation and point them out
- [x] there's a bug in burden, it doesn't go back to zero, and not sure it is counting what is in the hands properly
- [x] eating food does not work and/or health bar does not work
- [x] make hit markers show up momentarily, consider removing the message on every hit (or summarizing when someone dies who did the dmg to them)
- [x] grenades don't get depleted properly from your inventory as you fire them
- [x] consider including health bars on player list display so people can hunt weak players
- [ ] get punching to work
- [ ] figure out how to use ammo — numbered weapon items should consume charges until empty and reload from ammo in other hand
- [ ] need to test grenades, and other things that you throw and which blow up (at a distance or on hitting something?)
- [ ] the bow disappears when empty and probably should not
- [ ] add something to the title screen that mentions it is an old school game from 1989 and has been modernized, include a link to the repository
- [ ] player should only be able to take things that are near it and not across the room
- [ ] time people out of AFK games so that they don't stay on the server forever if they leave their browser tab open
- [ ] wait a little longer to reset the map after no one is left in it, and consider a timer in the lobby that tells you how long until the map resets (30 seconds?)
- [ ] let the chat history be taller, contain more text, perhaps use up space that is not needed by the player list
- [ ] make the mouse graphic not so tall, figure out a better way to display what the mouse buttons are for, it kind of needs a redesign for the modern era
- [ ] need the border boxes around other player (or to make them a different color than everything else? team colors?)
- [ ] there's still a display glitch in the client sometimes when you have joined a 2nd or 3rd map sometimes, is everything getting cleared?
- [ ] when you leave the browser open too long and there are hot reloads, chrome has to be restarted (probably only a dev experience problem)
- [ ] when you try to use the repair kit and there is nothing to repair you should get a message
- [ ] keep the chat history for up to 10 or 20 minutes so you can return and see it
- [ ] help users in the lobby know when server is going to get reset (when no one is left in it)
- [ ] there probably isn't anything keeping a 17th player from joining a max 16 player game
- [ ] need a tutorial to help teach how to pickup and manipulate inventory and learn shortcut keys (or at least a help page)
- [ ] do something different with the mouse controls (the big mouse looks funny)
- [ ] add hot key setting and mouse changes settings (but how will they save these, do we need profiles, or maybe just 3 good hotkey setting choices)
- [ ] consider placing player list at the top of the display to make them more prominent
- [ ] consider making it possible to know which room other players are in
- [ ] there might be a bug when player reappears after death, they spawn in another room, appear then jump to another location in the same room quickly
- [ ] when a player spawns in a locked room they cannot escape, maybe give them a give-up key that will give them a 5-second countdown to go to a new spawn location, and maybe that doesn't count as a death
- [ ] consider making the limited lighting feature work properly (where you can only see things near you and you have a flashlight)

Make teams work:
- [x] players start in their team room
- [x] add join team buttons (2 of them, and double the height of that map row for the avatars?)
- [ ] display which team people are in the player list, make your team more obvious on the map (in colors?)
- [ ] consider friendly fire options for teammates (i.e. cannot hurt members of your own team)


## Remaining Differences with Legacy
- [x] **fire rate limiting** — legacy has an ~850ms cooldown between shots (adjustable per weapon via a `refire` field); currently you can fire infinitely fast
- [x] **health regeneration** — in the original, players slowly heal between fights (1 HP/sec baseline, faster at higher levels); without it, consumables are the only way to recover HP
- [ ] **periodic item placement (`.pla` files)** — weapons and items respawn on a timer per map script; the game gets stale without this
- [x] **team UI selection** — lobby now shows per-team join buttons with avatar strips and player counts for multi-team maps; server uses the chosen team for spawn placement
- [ ] **friendly fire** — team membership is tracked server-side but teammates can still deal damage to each other
- [ ] **win conditions** — no game ever ends; the original had capture-the-flag mechanics (`flag`/`flagteams` on objects) and a game timer with rewards
- [x] **speed penalty when hurt** — in the original, players moved proportionally slower as they took damage (25% HP = 25% speed); creates tactical depth
- [ ] **dark rooms + flashlight** — several maps (castle, hack, outdoor) have rooms that are dark by default; flashlight items illuminate a radius around the bearer
- [ ] **day/night cycle** — some rooms are lit during the day and dark at night based on in-game time
- [ ] **wearable items** — armor, rings, helmets with a slot system exist in item definitions but cannot be equipped
- [ ] **spread/arc weapons** — shotguns, wide-arc blasters, and 360° explosions fire multiple projectiles; a whole weapon category is missing
- [ ] **explosion system** — grenades and certain weapons are supposed to spawn explosion objects that deal area damage; flammable map objects can chain-react; currently everything is point-damage only
- [ ] **ammo reload from quiver** — bows should reload from arrows carried in inventory; currently they just run out
- [ ] **object look/examine messages** — each item in the original had flavor text shown when looking at or examining it; currently only a name tooltip is shown

## Later / Polish / Maintainability
- [x] add unit and integration testing
- [ ] make it possible for other collaborators to deploy a production release (give them access to the VPS)
- [ ] add Burt back into the plan somehow, maybe to respond to chats if you address him directly?
