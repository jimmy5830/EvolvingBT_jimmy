# Changelog
Describes notable changes to the EvolvingBehavior example project between versions.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)

## [0.2.0] - 2022-08-09
### Added

- Added a Blueprint-only evolution test map that shows how to run an experiment entirely in Blueprint.
- Added Blueprint subclasses of Zombie Character, AI Controller, and Game Mode to use in the Blueprint test map, and to show how to set up the evolution process through blueprints.
- Added two larger test maps (medium and large), reorganized EvolvingBehavior-specific maps into a folder.
- Added static testing maps to test resulting evolved zombies (instead of evolving, the settings for these maps will just copy and run a specific behavior tree and measure its fitness).
- Added a few more manual example behavior trees.
- Added template Zombie evolution control actor, with some sensible default settings.
- Added human AI behavior tree, which randomly runs around and attempts to avoid zombies. This is used in evolution test maps.


### Changed
- Updated to Unreal Engine 4.27 (should also be compatible with 4.26)
- Each game type can now have a list of types of zombie to spawn, and will spawn them at random (or through some other form of selecting between them - for instance, the Evolution Testing game mode spawns randomly only from populations that have remaining behavior trees to test).
- Separated EvolvingBehavior-specific functionality in to subclasses, so that it is clear which parts of the code are specific to behavior tree evolution, rather than being part of the base game functionality.
- Fixed simulation settings to work with large numbers of AI actors.
- Updated EvolvingBehavior plugin to latest version.
- Improved Zombie AI to track last known location after losing track of a player, so it can search nearby.
- Added more complex factors to fitness functions for zombies (detecting chases, losing players, etc.).

## [0.1.0] - 2020-12-03
### Added
- First released version!

