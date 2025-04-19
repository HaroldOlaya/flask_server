from flask import Flask, request, Response,send_file
import requests
import numpy as np
import cv2
import socket
import time
import os

app = Flask(__name__)

ip_cam = None  # Variable global para almacenar la IP de la ESP32 CAM
image_path = './received_image.jpg'

# Función para obtener la IP pública del servidor
def get_public_ip():
    global ip_cam
    try:
        response = requests.get('https://haroldolaya99.pythonanywhere.com/get_ip')
        ip_cam = response.text.strip()

        if response.status_code == 200:
            data = response.json()
            last_ip = data.get('last_ip')
            ip_cam = last_ip
            print(last_ip)
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
            # Esperar un poco si no hay imagen aún
            time.sleep(0.1)

def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # No es necesario conectarse realmente
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception as e:
        print(f"Error obteniendo la IP local: {e}")
        return None
    
@app.route('/view')
def view_video():
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')
# Ruta para recibir imagen desde ESP32 CAM
@app.route('/upload', methods=['POST'])
def upload_image():
    try:
        img_data = request.data
        np_arr = np.frombuffer(img_data, np.uint8)
        image = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
        cv2.imwrite(image_path, image)

        hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        dominant_color = get_dominant_color(hsv)
        response = send_color_message(dominant_color)

        return {'status': 'success', 'color': dominant_color, 'response': response}
    except Exception as e:
        return {'status': 'error', 'message': str(e)}

# Función para obtener el color predominante
def get_dominant_color(hsv):
    colors = {
        "Rojo": ((0, 50, 50), (10, 255, 255)),
        "Verde": ((35, 50, 50), (85, 255, 255)),
        "Azul": ((100, 50, 50), (130, 255, 255)),
        "Amarillo": ((20, 50, 50), (30, 255, 255))
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

# Función para enviar mensaje HTTP según el color detectado
# Función para enviar mensaje HTTP según el color detectado
def send_color_message(color):
    print(f"Color detectado: {color}")

    # Obtener la IP del motor desde la URL proporcionada
    try:
        response = requests.get('https://haroldolaya99.pythonanywhere.com/get_ip_motor')
        if response.status_code == 200:
            data = response.json()
            ip_motor = data.get('last_ip_motor')
            print(f"IP del motor: {ip_motor}")
            
            if color == 'Rojo':
                url = f'http://{ip_motor}/message?message=/7L'
            elif color == 'Verde':
                url = f'http://{ip_motor}/message?message=/8P'
            elif color == 'Azul':
                url = f'http://{ip_motor}/message?message=/9N'
            else:
                return {'status': 'error', 'message': 'Color no reconocido'}

            try:
                response = requests.get(url)
                return {'status': 'success', 'response_code': response.status_code}
            except Exception as e:
                return {'status': 'error', 'message': str(e)}
        else:
            return {'status': 'error', 'message': 'No se pudo obtener la IP del motor'}
    except Exception as e:
        return {'status': 'error', 'message': f"Error al obtener la IP del motor: {str(e)}"}

# Función que se ejecuta al iniciar la aplicación
def print_and_send_ip_on_start():
    ip = get_public_ip()
    if ip:
        print(f"La IP pública es: {ip}")
        send_ip_to_esp32(ip)
    else:
        print("No se pudo obtener la IP pública.")

# Ejecutar función de arranque
print_and_send_ip_on_start()

# Ruta raíz
@app.route('/')
def index():
    return "La IP pública ya se ha mostrado en la terminal y se ha enviado a la ESP32 CAM."

# Iniciar servidor Flask
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
