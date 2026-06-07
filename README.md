# ⚡ Pikachu 3D animé – OpenGL

Projet académique dans le cadre du cours **synthèse d’image** (L3 informatique) consistant à modéliser et animer un **Pikachu 3D interactif**
en utilisant **OpenGL** et **GLUT / FreeGLUT** en **C++**.

📚 Réalisé dans le cadre du module **Synthèse d’Image**  
🎓 Licence 3 Informatique – Université de Bourgogne  
📅 Année universitaire 2025–2026

---

## 🧩 Présentation du projet

L’objectif de ce projet est de créer une **scène 3D complète et interactive** à partir de
**primitives géométriques simples**, intégrant :

- Une modélisation hiérarchique du personnage
- Des animations fluides
- Des textures
- Un éclairage dynamique
- Une caméra orbitale contrôlable
- Des interactions clavier et souris

Le personnage (Pikachu) est entièrement construit à partir de **sphères, cylindres, cônes et cubes**.

---

## 🎮 Fonctionnalités principales

### 🧱 Modélisation 3D hiérarchique
- Corps, tête, bras, jambes, queue, oreilles
- Gestion propre des transformations avec `glPushMatrix()` / `glPopMatrix()`

### 🎞️ Animations
- **Bras animés** (touche `A` / `a`)
- **Nez pivotant automatiquement** toutes les 2 secondes
- Animations fluides par interpolation progressive

### 💡 Éclairage dynamique
- ☀️ Lumière directionnelle (soleil)
- 🔴 Lumière positionnelle tournante autour de Pikachu
- Composantes ambiante, diffuse et spéculaire configurées

### 🎥 Caméra orbitale
- Rotation autour du personnage
- Contrôle au clavier et à la souris
- Zoom et reset de la vue

### 🖼️ Textures
- Textures JPEG appliquées au corps et au sol
- Sol texturé de grande taille (pelouse)

---

## 🧭 Contrôles

| Action | Touche |
|------|------|
| Lever / baisser les bras | `A` / `a` |
| Rotation caméra | Flèches directionnelles |
| Zoom | `Z` / `z` |
| Reset caméra | `R` |
| Quitter | `Esc` |

---

## 🗂️ Structure du projet

```text
.
├── cube.cpp           # Code principal OpenGL
├── textures/          # Images JPEG (textures)
├── README.md

⚙️ Compilation & Exécution
🔧 Prérequis

OpenGL

GLUT ou FreeGLUT

Compilateur C++ (g++)

🛠️ Compilation (Linux)
g++ cube.cpp -o pikachu \
-lGL -lGLU -lglut

▶️ Exécution
./pikachu
