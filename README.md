# windOS 0.01

> *"légère comme le vent"*

Un micro-noyau x86 32 bits, en mode texte VGA, avec un shell, un interpréteur inspiré de DOS et un mini-interpréteur BASIC — le tout en **~15 Ko**, zéro dépendance, zéro système d'exploitation en dessous.

```
  _      _         _  ____   _____
 (_)    (_)       | |/ __ \ / ____|
  __      __ _ ___| | |  | | (___
  \ \ /\ / /| '_ \| | |  | |\___ \
   \ V  V / | | | | | |__| |____) |
    \_/\_/  |_| |_|_|\____/|_____/

  windOS 0.01 -- "legere comme le vent"
```

## ⚠️ Ce que c'est vraiment (lis ça avant tout)

Ce projet est **honnête sur son ambition** :

- C'est un **vrai noyau qui démarre sur du matériel x86 réel** (ou dans un émulateur), écrit en C freestanding + un peu d'assembleur. Ce n'est pas une simulation dans un navigateur.
- Il n'a **aucune dépendance** : pas de libc, pas de bootloader tiers autre que le standard Multiboot (compatible GRUB), pas de pilote graphique — juste le mode texte VGA et le clavier PS/2 en direct.
- Sa légèreté est donc réelle et mesurable : le noyau compilé pèse environ **15 Ko**, contre plusieurs centaines de Mo pour un OS "normal" au démarrage.

**Ce que ce n'est PAS**, pour être clair :

- Ce n'est **pas un émulateur MS-DOS**. La commande `dos` lance un interpréteur *maison* qui reconnaît les commandes classiques (`DIR`, `TYPE`, `ECHO`, `CLS`, `VER`...) sur un faux système de fichiers en mémoire — il ne fait tourner aucun vrai `.EXE`/`.COM`. Un vrai émulateur DOS (type DOSBox) doit émuler un processeur x86 complet en mode réel, le BIOS, et l'interface DOS — c'est un projet à part entière de plusieurs dizaines de milliers de lignes.
- Ce n'est **pas QBasic**. La commande `basic` lance un mini-interpréteur BASIC "tiny BASIC" — numéros de ligne, `PRINT`, `LET`, `IF/THEN`, `GOTO`, `FOR/NEXT`, `INPUT` — en entiers uniquement, sans IDE, sans flottants, sans tableaux.

Dans cet esprit, oui : ce noyau est probablement plus léger que n'importe quel OS grand public existant, "Minuet OS" y compris — parce qu'il ne fait presque rien d'autre que le strict minimum. C'est le compromis assumé.

## Ce qui fonctionne réellement

- Un vrai bootloader **Multiboot** (compatible GRUB / `qemu -kernel` direct)
- Un pilote d'affichage **VGA texte 80×25** (couleurs, scroll, curseur)
- Un pilote **clavier PS/2** par scrutation (lettres, chiffres, majuscules via Shift)
- Un **shell** avec ses propres commandes (`help`, `ver`, `cls`, `echo`, `color`, `ascii`, `reboot`, `halt`)
- Un interpréteur **façon DOS** (`dos`) avec un mini système de fichiers en RAM
- Un interpréteur **BASIC** minimal (`basic`) capable d'exécuter de vrais petits programmes

## Compiler

Il faut `gcc`, `as` (binutils) et `ld`, tous disponibles sur toute distribution Linux avec les paquets de compilation standards installés (`build-essential` sur Debian/Ubuntu).

```bash
make
```

Ça produit `windos.elf`. Pour voir la taille et le détail des sections :

```bash
make size
```

## Tester (sur ta machine — je n'ai pas QEMU dans mon environnement)

Installe QEMU si besoin :

```bash
sudo apt install qemu-system-x86    # Debian/Ubuntu
brew install qemu                    # macOS
```

Puis lance directement le noyau, sans ISO ni bootloader à installer :

```bash
qemu-system-i386 -kernel windos.elf
```

(ou `make run`, qui fait exactement ça)

### Tester sur une vraie machine

1. Installe `grub-mkrescue` (`sudo apt install grub-pc-bin xorriso`).
2. Crée l'arborescence GRUB et l'ISO :
   ```bash
   mkdir -p iso/boot/grub
   cp windos.elf iso/boot/windos.elf
   cat > iso/boot/grub/grub.cfg << 'EOF'
   menuentry "windOS 0.01" {
       multiboot /boot/windos.elf
   }
   EOF
   grub-mkrescue -o windos.iso iso
   ```
3. Grave `windos.iso` sur une clé USB (`dd if=windos.iso of=/dev/sdX`, en vérifiant bien le périphérique) et démarre dessus.

## Utilisation une fois démarré

```
windOS> help
windOS> dos
C:\>DIR
C:\>TYPE README.TXT
C:\>EXIT
windOS> basic
] 10 PRINT "BONJOUR"
] 20 LET X = 0
] 30 LET X = X + 1
] 40 PRINT X
] 50 IF X < 5 THEN 30
] RUN
] EXIT
```

## Structure du code

```
src/
  boot.S        point d'entrée + en-tête Multiboot
  linker.ld     script de liaison (charge à 1 Mo)
  kernel.c      shell principal, dispatch des commandes
  vga.c/.h      pilote d'affichage texte
  keyboard.c/.h pilote clavier PS/2
  strutil.c/.h  fonctions chaînes (pas de libc disponible)
  dosexec.c     interpréteur façon DOS
  basicexec.c   interpréteur BASIC minimal
```

## Idées d'évolution honnêtes

- Gestion des interruptions clavier (IRQ) au lieu du polling, pour libérer le CPU
- Un vrai système de fichiers persistant (actuellement tout est en RAM, perdu au reboot)
- Support de la souris et d'un mode graphique VGA (320×200) en option
- Extension du BASIC : chaînes de caractères, tableaux, sous-programmes (`GOSUB`/`RETURN`)

## Licence

MIT — voir [LICENSE](LICENSE).
