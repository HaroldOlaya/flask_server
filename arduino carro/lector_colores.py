from flask import Flask, request, Response, send_file, render_template, redirect, url_for
import requests
import numpy as np
import cv2
import socket
import time
import os

app = Flask(__name__)
ip_cam = None  # Variable global para almacenar la IP de la ESP32 CAM
image_path = './received_image.jpg'
current_mode = 'desactivar'  # Puede ser: 'color', 'desactivar', 'linea'

# Función para obtener la IP pública del servidor
def get_public_ip():
    global ip_cam
    try:
        response = requests.get('https://haroldolaya99.pythonanywhere.com/get_ip')
        if response.status_code == 200:
            data = response.json()
            last_ip = data.get('last_ip')
            ip_cam = last_ip
            print(f"IP pública obtenida: {last_ip}")
        return ip_cam
    except Exception as e:
        print(f"Error obteniendo la IP pública: {str(e)}")
        return None

# Función para enviar la IP al ESP32 CAM
def send_ip_to_esp32(ip):
    global ip_cam
    esp32_url = f"http://{ip_cam}/receive_ip"
    public_ip = get_local_ip()
    if public_ip:
        print(f"Mi IP pública es: {public_ip}")
    payload = {'ip': public_ip}
    try:
        response = requests.post(esp32_url, data=payload)
        if response.status_code == 200:
            print(f"IP enviada correctamente a ESP32 CAM: {ip}")
        else:
            print(f"Error al enviar la IP a ESP32 CAM: {response.status_code}")
    except Exception as e:
        print(f"Error al enviar la IP a la ESP32 CAM: {str(e)}")

# Obtener IP local
def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception as e:
        print(f"Error obteniendo la IP local: {e}")
        return None

# Generador de imágenes para streaming
def generate_frames():
    while True:
        if os.path.exists(image_path):
            frame = cv2.imread(image_path)
            if frame is None:
                continue
            _, buffer = cv2.imencode('.jpg', frame)
            frame_bytes = buffer.tobytes()

            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
        else:
            time.sleep(0.1)

# Ruta para vista del video
@app.route('/view')
def view_video():
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

# Ruta principal - muestra frontend
@app.route('/')
def index():
    return render_template('index.html', mode=current_mode)

# Ruta para cambiar modo desde frontend
@app.route('/set_mode/<mode>', methods=['POST'])
def set_mode(mode):
    global current_mode
    if mode in ['color', 'desactivar', 'linea']:
        current_mode = mode
        print(f"Modo cambiado a: {current_mode}")
    return redirect(url_for('index'))

# Ruta para recibir imagen desde ESP32 CAM
# Ruta para recibir imagen desde ESP32 CAM
@app.route('/upload', methods=['POST'])
def upload_image():
    global current_mode
    try:
        img_data = request.data
        np_arr = np.frombuffer(img_data, np.uint8)
        image = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
        cv2.imwrite(image_path, image)

        if current_mode == 'desactivar':
            print("Modo DESACTIVAR: imagen ignorada.")
            return {'status': 'ok', 'message': 'Modo desactivado, imagen ignorada'}

        if current_mode == 'linea':
            print("Modo LINEA: analizando cuadros con negro...")

            # Convertir la imagen a escala de grises
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

            # Aplicar un umbral para detectar las áreas negras
            _, thresh = cv2.threshold(gray, 50, 255, cv2.THRESH_BINARY_INV)

            # Obtener el alto y ancho de la imagen
            height, width = image.shape[:2]

            # Dividir la imagen en una cuadrícula (por ejemplo, en cuadros de 100x100 píxeles)
            square_size = 100
            num_rows = height // square_size
            num_cols = width // square_size

            # Guardar los cuadros donde se detecta el negro
            black_squares = []

            # Analizar cada cuadro
            for i in range(num_rows):
                for j in range(num_cols):
                    # Obtener las coordenadas del cuadro
                    x_start = j * square_size
                    y_start = i * square_size
                    x_end = (j + 1) * square_size
                    y_end = (i + 1) * square_size

                    # Extraer el cuadro de la imagen umbralizada
                    square = thresh[y_start:y_end, x_start:x_end]

                    # Verificar si hay áreas negras en el cuadro (es decir, si hay píxeles no cero)
                    if cv2.countNonZero(square) > 0:
                        black_squares.append((i, j))  # Guardamos las coordenadas del cuadro

            # Mostrar la información sobre los cuadros donde se encontró el negro
            if black_squares:
                for square in black_squares:
                    print(f"Negro encontrado en el cuadro: Fila {square[0]}, Columna {square[1]}")
                return {'status': 'ok', 'message': f'Negro encontrado en los cuadros: {black_squares}'}
            else:
                return {'status': 'ok', 'message': 'No se detectó negro en los cuadros'}

        if current_mode == 'color':
            hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            dominant_color = get_dominant_color(hsv)
            response = send_color_message(dominant_color)
            return {'status': 'success', 'color': dominant_color, 'response': response}

        return {'status': 'ok', 'message': 'Modo no definido'}
    except Exception as e:
        return {'status': 'error', 'message': str(e)}

# Detección de color dominante
def get_dominant_color(hsv):
    colors = {
        "Rojo": ((0, 50, 50), (10, 255, 255)),
        "Verde": ((35, 50, 50), (85, 255, 255)),
        "Azul": ((100, 50, 50), (130, 255, 255)),
        "Amarillo": ((15, 50, 50), (35, 255, 255)), 
    }

    max_count = 0
    dominant_color = "Desconocido"

    for color_name, (lower, upper) in colors.items():
        mask = cv2.inRange(hsv, np.array(lower), np.array(upper))
        count = cv2.countNonZero(mask)
        if count > max_count:
            max_count = count
            dominant_color = color_name

    return dominant_color

# Envío de comando HTTP según el color
def send_color_message(color):
    print(f"Color detectado: {color}")
    try:
        response = requests.get('https://haroldolaya99.pythonanywhere.com/get_ip_motor')
        if response.status_code == 200:
            data = response.json()
            ip_motor = data.get('last_ip_motor')
            print(f"IP del motor: {ip_motor}")

            message_map = {
                'Rojo': '/2C',
                'Verde': '/2E',
                'Azul': '/2D',
                'Amarillo': '/2F'
            }

            if color in message_map:
                url = f'http://{ip_motor}/message?message={message_map[color]}'
                response = requests.get(url)
                return {'status': 'success', 'response_code': response.status_code}
            else:
                return {'status': 'error', 'message': 'Color no reconocido'}
        else:
            return {'status': 'error', 'message': 'No se pudo obtener la IP del motor'}
    except Exception as e:
        return {'status': 'error', 'message': f"Error al obtener la IP del motor: {str(e)}"}

# Al iniciar la app
def print_and_send_ip_on_start():
    ip = get_public_ip()
    if ip:
        print(f"La IP pública es: {ip}")
        send_ip_to_esp32(ip)
    else:
        print("No se pudo obtener la IP pública.")

print_and_send_ip_on_start()

# Iniciar servidor Flask
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
