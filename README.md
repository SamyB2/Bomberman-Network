# Projet de Programmation Réseaux

## Groupe :
- Sami BELMELLAT (22116580)
- Rayane LOTENBERG (22111684)
- Alexandre GORCE NEIVA DA COSTA (22104734)

## Lancer le projet

### Cloner le dépôt :
```bash
git clone https://moule.informatique.univ-paris-diderot.fr/belmella/pr6-projet.git
cd pr6-projet
```

Pour lancer le serveur :
- make
- ./server

Pour lancer le client :
- 
```bash
 ./client <url> <port> <gamemode>
```

Le port par défaut du serveur est 8080. Il faut le specifier malgré tout.

Remplacer gamemode par : 
- 1 si la partie se déroule sans équipe
- 2 si la partie se déroule avec équipe

## Gameplay :
Contrôle : 
- Flèche de gauche -> LEFT           
- Flèche de droite -> RIGHT
- Flèche du haut -> UP 
- Flèche du bas -> DOWN
- ~ -> QUIT
- . -> DROP the bomb
- , -> CHANGE the sender chat TO ALL or TEAM
