# Griljor Web Rewrite — TODO

## Client / Server

- [x] Keep-alive pings — client sends a ping every 60 seconds while idle so a player can remain in-game with browser open; server uses this to avoid disconnecting motionless players
- [ ] Consumables — items with health/power effects beyond the door mechanism
- [ ] Teams — lobby team selection + server-side enforcement (friendly fire, spawn zones)
- [ ] 8-direction movement — currently 4-directional only
- [x] Fix `nearbyFreeTile` in `session.ts` — currently uses `permeable` instead of `movement` for free-tile checks
- [x] When everyone leaves a map, it should respawn and reset to original state
- [ ] Research how the original screen was laid out so the UI matches better
- [ ] Make the frontend much nicer to look at (consider a frontend design pass / design tool)
- [ ] Figure out how to use ammo — numbered weapon items should consume charges on fire
- [ ] Audit pickup and drop mechanisms — verify they are working correctly end-to-end
- [ ] Decide whether to show the border arrow indicators that reveal adjacent rooms

## Networking / Deployment

- [ ] Figure out how to set up a free VM on Oracle Cloud
- [ ] Review the deployment plan in `deployment-plan.md` and execute it
- [ ] Figure out what can be demoed next Sunday night (use Chrome + screen share in Google Meet); ideally have it deployed so others can join from their browser

## Maps

- [ ] Test unlocking a door or repairing a broken one (they appear normal on the castle map — investigate)
- [ ] Pick the best maps for showcasing in the demo

## Later / Polish

- [ ] Title screen and game listing refinements before public release
- [ ] Make it possible for others to deploy a production release (if they have VM access)
- [ ] Consider whether a code refactor is worthwhile to improve clarity and maintainability
- [ ] add Burt back into the plan somehow, maybe to respond to chats if you address him directly?