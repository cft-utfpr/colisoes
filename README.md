# Simulador de colisões
> Trabalho por Caio Furlan Traebert (Ra: 2780046)

<img src="assets/Screenshot_3D.png">

## Opções
- `--no-sound`: Desativa o som das colisões
- `--resolution larguraxaltura`: Especifica o tamanho da janela diretamente

## Dependências
### Raylib

apt:

>sudo apt install raylib

pacman:

>sudo pacman -S raylib

Windows:

>https://www.raylib.com/

## Compilando (linux)

```
git clone https://github.com/cft-utfpr/colisoes.git
cd colisoes
gcc collisions3d.c -lm -lraylib -o out
```
