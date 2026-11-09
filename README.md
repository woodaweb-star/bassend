# NARMIA.AI Bassend

Sidechain ducker de uso personal. Entrada principal: bajo o pads. Sidechain: kick.

## Controles
- **Depth**: cuánto baja el volumen con cada kick (0–100 %).
- **Attack**: velocidad con la que baja el volumen (0.1–50 ms).
- **Hold**: tiempo que se mantiene la reducción antes de subir (0–500 ms).
- **Release**: velocidad con la que vuelve el volumen (5–1000 ms).

## Uso en el DAW
1. Pon Bassend en la pista de bajo.
2. Activa la entrada sidechain del plugin en tu DAW y envíale la pista de kick.
3. Ajusta Depth y Release hasta que el bajo respire con cada golpe.

## Compilar en la nube
El workflow `.github/workflows/build.yml` compila el VST3 para Windows y macOS.
Ve a la pestaña **Actions** del repositorio y descarga el artefacto `NARMIA-Bassend-Windows` o `NARMIA-Bassend-macOS`.
