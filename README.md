# tronbot

_A bot that plays TRON: light cycle, and more._

## What this is

This started out as an assignement for school to learn dynamic programming.
Since I did not find any implementation of the game logic that suited me, I
reimplemented it along with basic dynamic programming algorithms to solve the
game. The limited time span I had to work on this project made it so
that the final state of it was not very satisfying. I am now working on properly
completing this project.

## Goals

My goal is to learn stuff through fun projects like this one so my steps are
based on what will teach me the most rather than on what will progress the project.
The current order of things is :
- Build a functionnal and proper server
  - Flexible game mechanics
  - Standalone from the player code for bot modularity
  - Network play?
  - Human play?
- Build a few bots with different strategies with different levels of refinment
  - Have a base bot 'class' in with only one function to implement to change its
    behavior
  - No current plans about which strategies will be implemented.

### Old project notes
- Maximiser le temps de calcul (faire en sorte de prendre près de une seconde
  à chaque fois)
- Choisir des grilles sur lesquelles faire des tests
- Choisir les paramètres à tester

Faire les enfants des noeuds déjà finis si le joueur les a évalués? Sinon, on
ne remarque pas les positions qui forcent la nulle et on les considère
seulement comme une défaite.
