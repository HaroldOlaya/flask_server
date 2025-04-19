import cv2
import numpy as np
import matplotlib.pyplot as plt

# Crear la rueda de colores HSV
hue_range = np.arange(0, 180, 1)
saturation = 255
value = 255
hsv_wheel = np.zeros((180, 180, 3), dtype=np.uint8)

for i, hue in enumerate(hue_range):
    hsv_wheel[:, i] = (hue, saturation, value)

# Convertir a BGR para visualización
bgr_wheel = cv2.cvtColor(hsv_wheel, cv2.COLOR_HSV2BGR)

# Definir los rangos de colores
colors = {
    "Rojo": ((0, 50, 50), (10, 255, 255)),
    "Verde": ((35, 50, 50), (85, 255, 255)),
    "Azul": ((100, 50, 50), (130, 255, 255)),
    "Amarillo": ((20, 50, 50), (30, 255, 255))
}

# Crear una máscara para cada color
highlighted_wheel = bgr_wheel.copy()
for color, (lower, upper) in colors.items():
    lower_np = np.array(lower, dtype=np.uint8)
    upper_np = np.array(upper, dtype=np.uint8)
    mask = cv2.inRange(hsv_wheel, lower_np, upper_np)
    highlighted_wheel[mask > 0] = [255, 255, 255]  # Resaltar en blanco

# Mostrar la imagen
plt.imshow(cv2.cvtColor(highlighted_wheel, cv2.COLOR_BGR2RGB))
plt.title("Rangos HSV Destacados")
plt.axis("off")
plt.show()
