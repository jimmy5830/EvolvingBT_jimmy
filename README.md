# EvolvingBehavior Example Project - SurvivalGame

### Authors: Epic Survival Game Series is by Tom Looman, modified for EvolvingBehavior by Nathan Partlan (@NPCDev), Jim Howe, Luis Soto, Sarthak Shrivastava, Alex Grundwerg, Isha Srivastava

### Collaboration and Advice: Magy Seif El-Nasr, Stacy Marsella, Erica Kleinman, Sabbir Ahmad, Muhammad Ali, Zheng Fang

-------------------------------
## Summary

This is a simple third-person survival game, modified to provide an example of using the [EvolvingBehavior](https://evolvingbehavior.npc.codes) plugin for evolving AI behavior trees in Unreal Engine.

Running the game in the EvolvingBehavior test map will automatically evolve AI zombies, starting from a non-functional stub of a behavior tree. They will often develop capabilities to patrol the map, chase, and attack the player. The resulting behavior trees are saved (in the Content/EvolvingBehavior folder) and can be examined, adjusted, and used just like any standard Unreal behavior tree! Editing the settings in the EvolutionControlActor allows testing a variety of evolution controls and styles.

NOTE: This example project is a modified version of the open-source [Epic Survival Game Series](https://github.com/tomlooman/EpicSurvivalGameSeries) by Tom Looman. See the bottom of this document for links and details from that project.

## Installation
----
- Install Unreal Engine 4, version 4.27.
- Install a code editor, such as Visual Studio on Windows, XCode on Mac, or VSCodium on Linux.
- Open the "SurvivalGame.uproject" file in Unreal Engine 4.

## Running
----
- To run a test map for Evolving Behavior Trees:
- Open one of the EvolvingBehavior test maps in the "EvolvingBehavior" subfolder of the maps folder.
- Play in the editor, with the "Play" button at the top right, or use Unreal Engine to build a standalone copy and play it.

## License
----
This example project uses Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere. Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc. All rights reserved.

This project is free and open source under the MIT License, which can be used in conjunction with code licensed under the Unreal Engine EULA. Note that, to use this plugin in Unreal Engine 4, you will need to have your own license to use the Unreal Engine 4 software itself. See LICENSE.md for details.

## Epic Survival Game Series
-------------------------
### Author: Tom Looman

Third-person survival game for Unreal Engine 4 made entirely in C++. Originally built as a 6 section tutorial series, now available as open-source C++ sample project.

See [the main documentation page on the unreal engine Wiki](https://wiki.unrealengine.com/Survival_sample_game).

For questions & feedback visit [the official thread on the unreal engine forums](https://forums.unrealengine.com/showthread.php?63678-Upcoming-C-Gameplay-Example-Series-Making-a-Survival-Game)
