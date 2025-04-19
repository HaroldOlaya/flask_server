from flask import Flask, request, Response
import cv2
import numpy as np
import requests

app = Flask(__name__)

# IP fija y puerto del servidor
server_ip = "192.168.1.100"  # Cambia esta IP según tu red/upload
server_port = 5000
image_path = 'received_image.jpg'  # Ruta donde se guarda la última imagen recibida

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

# Enviar mensaje HTTP dependiendo del color
def send_color_message(color):
    if color == 'Rojo':
        url = 'http://192.168.161.157/message?message=/7L'
    elif color == 'Verde':
        url = 'http://192.168.161.157/message?message=/8P'
    elif color == 'Azul':
        url = 'http://192.168.161.157/message?message=/9N'
    else:
        return {'status': 'error', 'message': 'Color no reconocido'}
    try:
        response = requests.get(url)
        return {'status': 'success', 'response_code': response.status_code}
    except Exception as e:
        return {'status': 'error', 'message': str(e)}

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

# Generar transmisión de video
def generate_video_stream():
    while True:
        frame = cv2.imread(image_path)
        if frame is not None:
            ret, jpeg = cv2.imencode('.jpg', frame)
            if not ret:
                continue
            frame_bytes = jpeg.tobytes()
            yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')

@app.route('/view', methods=['GET'])
def view_video():
    return Response(generate_video_stream(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
