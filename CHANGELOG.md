# GDIntercept Changelog

## v0.8.11 

- Increase overal performance
- Increase performance when loading binary views
- Delete masking due to too many compat issues

## v0.8.10 Fix the wraith

- Fix the wraith endpoint which relies on the user ID

## v0.8.9 Fix user endpoints

- Fix endpoints which for some reason rely on the user IDs

## v0.8.8 Fix request censoring

- Fix a bug where the sent body had censored fields

## v0.8.7 Masking

- Add request masking to hide telemetry info

## v0.8.6 Yes.

- We do a little trolling

## v0.8.5 Username change

- Refactored my GitHub account name in all references

## v0.8.4 Bug Fixes

- Added sizes to the info block
- Fixed unexpected formats accidentally being parsed as a form
- Fixed variable scrolling being rounded to 0
- Fixed saving files using invalid characters
- Fixed code smells which could potenttially use deleted memory

## v0.8.3 Fixing Android Request Bodies

- Fixed request bodies not being shown on Android (thx fod)

## v0.8.2 Scroll Lock Fix

- Fixed y axis scroll lock on narrow lists

## v0.8.1 General QoL

- Added hover based scroll focus
- Added header list syntax parsing
- Added mod ID to the request info
- Fixed horizontal scroll
- Fixed 0 size track thumbs on large requests

## v0.8.0 - GD/Geode Version Migration

- Migrated to Geode V5
- Migrated to GD 2.208x
- Extensively optimized the mod
- Created a v2 RobTop response to JSON parser
- Made the minimized menu setting default false
- Made Geode proxies less intrusive in the client
- Added auto decoding for RobTop responses
- Added a setting for max line size
- Added better memory management for proxies
- Added univeral keybind support
- Added repeatable keybinds
- Added iOS support
- Added early loading
- Added curl and info copy shortcuts
- Moved theme buttons to settings
- Moved keybind settings to the GDIntercept settings
- Fixed incompatibilities with other Geode web hooking mods
- Fixed keyless headers not showing
- Fixed list headers not showing all values
- Fixed to raw conversions causing crashes in certain edge cases
- Fixed excesive number sizes causing crashes
- Fixed excesive line sizes causing lag spikes when culled
- Disabled pausing/cancelling for Geode proxies

## v0.7.2 (2025-02-15) - Fixed Raw Delimiter Usage

- Fixed an issue where the entry delimiter was used rather than the main delimiter between values
- Added a HTTP message log option to the settings

## v0.7.1 (2025-02-15) - Fixed Port Handling

- Ensured that ports don't get duplicated in the URL

## v0.7.0 (2025-01-25) - API Deprecation, Bugfixes, GDPS Support & URL Rework

- Deprecated the public API as it was limiting the workflow
- Fixed a lot of small bugs
- Changed the RobTop to JSON converter to be more GDPS friendly
- Reworked URLs so that they are compatible with every standard
- Added more info to requests

## v0.6.7-alpha.6 (2024-10-08) - Release Migration

- Migrated the releases to the Binary Locker service to track downloads and speed up the development cycle
- Moved pages to the new deployment platform

## v0.6.6-alpha.6 (2024-10-02) - Hook Compatibility Patch

- Added compatibility for hooks that modify the request data

## v0.6.5-alpha.6 (2024-09-30) - Race Condition Patch

- Patched all remaining race conditions (closed source for security reasons)

## v0.6.4-alpha.6 (2024-09-27) - Theme Docs

- Added a theme documentation page

## v0.6.3-alpha.6 (2024-09-26) - Pause Persisting

- Added a confirmation setting to persist the pause state between sessions

## v0.6.2-alpha.6 (2024-09-25) - Minor Semantic Fixes

- Fixed the release date of the previous version
- Fixed the one-dark-pro theme targeting an old schema ID

## v0.6.1-alpha.6 (2024-09-24) - Themes Update

- Added custom theme support
- Added additional default themes
- Adjusted several nodes to allow for more customization
- Added a reloadable theme setting
- Added key and string quote tokens
- Added a persistent paused request warning
- Added a client flag to show the client used for the request
- Patched a race condition which would occur when a request was finished receiving but still being processed while reaching the request cache limit
- Patched a memory leak when a cocos request is canceled
- Fixed a bug where the bottom scrollbar would slightly scroll down the content of the code block
- Fixed a bug where opening and closing settings followed by receiving a request while the menu is still open would cause a crash
- Removed the pause requests setting in favor of an instance based pause
- Increased performance by reducing the amount of updates per frame

## v0.6.0-alpha.6 (2024-09-14) - UI v3 Update

- Replaced the small menus with a main code block menu
- Added horizontal scrolling to the code block menu
- Added a side bar with quick actions and view switching
- Added a request info view
- Added a response headers view
- Made line numbers stick to the left side of the code block
- Added a save request button
- Added more themes
- Added support for more theme customizability for the future
- Added snapping aggression to snap the scroll to a specific axis for a few update cycles
- Implemented the new settings system
- Major keybind overhaul
- Added a cancel feature to the proxy
- Added more stability for proxying geode requests

## v0.5.6-alpha.5 (2024-09-10) - ID System Implementation

- To prevent being incompatible with other geode web API mods, the ID system has been implemented

## v0.5.5-alpha.5 (2024-08-31) - Mod Size Reduction

- Removed a build setting that was causing the mod to be larger than it should be

## v0.5.4-alpha.5 (2024-08-06) - Alpha.5 Hotfix 4

- Moved some logic around to prevent accidentally deleting data before providing it to the events

## v0.5.3-alpha.5 (2024-08-06) - Alpha.5 Hotfix 3

- Added race condition protection due to some edge cases where progress may be sent to a canceled request

## v0.5.2-alpha.5 (2024-08-05) - Alpha.5 Hotfix 2

- Replaced nulls with empty strings in the form and Robtop conversions to prevent incorrect to raw conversions

## v0.5.1-alpha.5 (2024-08-05) - Alpha.5 Hotfix

- Fixed a bug where certain query strings may cause crashes

## v0.5.0-alpha.5 (2024-08-05) - Badges Update

- Added badges to the request list
- Added several API changes for better conversion support and repeat detection
- Added soft reloading support for minor state changes
- Added tooltips
- Added hovering support
- Added a new setting to disable the new badge system
- Added a copy button to the code block and info block
- Added a new setting to disable the new copy button
- Fixed thread safety for API events
- Fixed a bug where the raw data would not censor sensitive data
- Fixed crashes occurring when a non ASCII character was anywhere in the request

## v0.4.2-alpha.4 (2024-07-23) - UI Scaling Update

- Added compatibility for different aspect ratios and resolution changes

## v0.4.1-alpha.4 (2024-07-23) - Pause/Resume Safety Update

- Added checks to prevent pause/resume from accidentally triggering a recursive loop
- Fixed settings overlapping the send button once closed

## v0.4.0-alpha.4 (2024-07-22) - Optimization Update

- Added node culling to only show the visible nodes
- Added request limiting to prevent excessive RAM usage
- Removed conversion caching as they significantly increase RAM usage
- Optimized the binary parser
- Fixed scroll resets when pausing/resuming requests

## v0.3.7-alpha.3 (2024-07-13) - Stability Update V5

- Changed the way Android NDK specific objects are initialized

## v0.3.6-alpha.3 (2024-07-13) - Stability Update V4

- Removed the use of CCHttpClient threading because either Android breaks from it or the Android fix breaks windows

## v0.3.5-alpha.3 (2024-07-13) - Stability Update V3

- Fixed cocos getting so trigger happy that it cleaned up a request before it could even be read

## v0.3.4-alpha.3 (2024-07-13) - Stability Update V2

- Fixed an issue where cocos requests would be released before the thread was done with them

## v0.3.3-alpha.3 (2024-07-13) - Stability Update

- Changed the way per frame updates are handled to prevent crashes

## v0.3.2-alpha.3 (2024-07-12) - Cancel Support

- Added support for cancelling requests

## v0.3.1-alpha.3 (2024-07-12) - Bugfix Update

- Fixed a bug where certain requests would cause an unallocated reference exception
- Optimized the content loading

## v0.3.0-alpha.3 (2024-07-12) - API, Geode and Request Flow Support

- Added support for the Geode web API
- Added support for request pausing, resuming and resending
- Added API support for other mods
- Added a lot more keybinds
- Added a filter setting
- Fixed several bugs

## v0.2.3-alpha.2 (2024-06-25) - More Android Fixes

- Fixed Android having a lot of touch priority issues

## v0.2.2-alpha.2 (2024-06-23) - Binary Update

- Added binary support
- Optimized text processing
- Fixed binary being attempted to be rendered as forms
- Added cache support
- Made the setting descriptions clearer

## v0.2.1-alpha.2 (2024-06-22) - Android Fix

- Added a failsafe for Android devices

## v0.2.0-alpha.2 (2024-06-22) - Optimizations, QOL and Redesign

- Added a direct link to the Geode settings menu
- Fixed a ton of issues
- Optimized the JSON parser
- Made theme selecting easier
- Added an option to log requests

## v0.1.2-alpha.1 (2024-06-20) - Bugfix Update

- Fixed some bugs with the parsing of the response data
- Fixed some UI bugs
- Added better MacOS support

## v0.1.1-alpha.1 (2024-06-20) - Compatibility Update

- Made the mod compatible with other mods on the menu layer
- Added MacOS support

## v0.1.0-alpha.1 (2024-06-19) - Initial Release

- Added all the basic interface features
- Added some useful keybinds
- Introduced Android/Windows support
- Added data obscuring
- Added JSON conversions
- Added a readonly monaco editor
- Added request caching
